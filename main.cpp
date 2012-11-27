// Keyboard Layout Optimizer
// Written by Shawn Badger 2012

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <float.h>
#include <mpi.h>
#include "keyboardlayoutoptimizer.h"

using namespace std;

#define MCW  MPI_COMM_WORLD
int size, rank;


// Number of jobs to be run before quitting
int num_jobs_remaining = 5;
int num_jobs_completed = 0;

// Number of layouts a job should look at before returning
int layouts_per_job = 100000;

// Keep track of the best layout
string best_layout;
double best_layout_score = DBL_MAX;


// New MPI data type to describe a Message struct
MPI_Datatype KLO_MESSAGE;

// Worker processes respond with one of these
struct Message {
    int sender;
    double score; 
    char layout[NUMKEYS+1];
};



char initial_layout[NUMKEYS+1]  = { "`1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./" };
//char dvorak_layout[NUMKEYS+1]  = { "`1234567890[]',.pyfgcrl/=\\aoeuidhtns-;qjkxbmwvz" };
//char colemak_layout[NUMKEYS+1] = { "`1234567890-=qwfpgjluy;[]\\arstdhneio'zxcvbkm,./" }; 
//char workman_layout[NUMKEYS+1] = { "`1234567890-=qdrwbjfup;[]\\ashtgyneoi'zxmcvkl,./" };
//char bulpkm_layout[NUMKEYS+1]  = { "`1234567890-='bulpkmyf;[]\\riaohdtensjzxcvqgw,./" };
//char xfyl_layout[NUMKEYS+1]    = { "`1234567890-=xfyljkpuw;[]\\asinhdtero'zb.mqgc,v/" };
//char test_layout[NUMKEYS+1]    = { "`1234567890-=tkpb'oqc,.[]\\r/;sxfzvgwluyemdnihja" };


// Generates a random number between [min, max]
int random_range(int min, int max)
{
    int range = max-min+1;
    return min+int(range*(double)rand()/(RAND_MAX+1.0));
}


// The master process call this function to manager all other
// processes by tasking them and maintaining a best layout
void manage_jobs()
{
    MPI_Status status;
    int num_jobs_total = num_jobs_remaining;

    // Send a random seed to every other process
    // and task it with an initial job
    for (int i=1; i<size; i++) {
        int seed = random_range(0, INT_MAX-1);
        MPI_Send(&seed, 1, MPI_INT, i, 0, MCW);

        num_jobs_remaining--;
        MPI_Send(&layouts_per_job, 1, MPI_INT, i, 0, MCW);
    }

    // Wait for processes to finish their tasks
    while (num_jobs_remaining > 0) {
        Message msg;
        MPI_Recv(&msg, 1, KLO_MESSAGE, MPI_ANY_SOURCE, 0, MCW, &status);
        num_jobs_completed++;

        printf("Rank%d completed job - %.4f '%s'\n", msg.sender, msg.score, msg.layout);
        if (msg.score < best_layout_score) {
            best_layout_score = msg.score;
            best_layout = msg.layout;
        }

        MPI_Send(&layouts_per_job, 1, MPI_INT, msg.sender, 0, MCW);
        num_jobs_remaining--;
    }

    // At this point, we've sent out all of the jobs that we want to process
    // now we just have to wait for them to come back
    while (num_jobs_completed < num_jobs_total) {
        Message msg;
        MPI_Recv(&msg, 1, KLO_MESSAGE, MPI_ANY_SOURCE, 0, MCW, &status);
        num_jobs_completed++;

        printf("Rank%d completed job - %.4f '%s'\n", msg.sender, msg.score, msg.layout);
        if (msg.score < best_layout_score) {
            best_layout_score = msg.score;
            best_layout = msg.layout;
        }
    }

    // Now tell all other processes to shutdown
    for (int i=1; i<size; i++) {
        int shutdown = -1;
        MPI_Send(&shutdown, 1, MPI_INT, i, 0, MCW);
    }

    printf("\nBest Layout Score: %.4f\n", best_layout_score);
    printf("Best Layout: %s\n\n", best_layout.c_str());
}


// Worker processes call this to perform the actual work and
// report their results back to the master process
void process_jobs()
{
    MPI_Status status;
    KeyboardLayoutOptimizer klo;
    int niter = 0;
    Message msg;
    msg.sender = rank;

    // This is needed for parseTriads()
    klo.buildCharToIndexMap(initial_layout);
    if (!klo.parseTriads("corpus/corpus.txt", LETTERS /*| NUMBERS | PUNCTUATION | SYMBOLS*/)) {
        fprintf(stderr, "Error parsing triads from '%s' (rank %d)\n", "corpus/corpus.txt", rank);
        return;
    }

    while (1) {
        // Receive a job indication how many times we should iterate
        MPI_Recv(&niter, 1, MPI_INT, 0, 0, MCW, &status);
        if (niter < 0) return;  // shutdown signal

        printf("Processing Job (rank %d)\n", rank);
        int iterations = layouts_per_job;
        double t0=0.5;
        double p0=0.3;   /* Set to zero to refuse transitions to worse layouts */
        double k =500.0; /* set higher to cooldown faster */
        char *layout = initial_layout;

        // perform the optimization
        klo.optimizeLayout(layout, iterations, t0, p0, k);

        string bestlayout      = klo.getBestLayout();
        double bestlayoutscore = klo.getBestLayoutScore();

        strncpy(msg.layout, bestlayout.c_str(), sizeof(msg.layout));
        msg.layout[sizeof(msg.layout)-1] = 0;

        msg.score = bestlayoutscore;

        // send the results back to the master process
        MPI_Send(&msg, 1, KLO_MESSAGE, 0, 0, MCW);
    }

}


int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    Message msg;

    // Create a new MPI datatype representing the Message structure,
    // so we can easily send/recv them using MPI_Send/MPI_Recv
    MPI_Datatype type[3] = {MPI_INT, MPI_DOUBLE, MPI_CHAR};
    int blocklen[3] = {1, 1, NUMKEYS+1};
    MPI_Aint disp[3];
    disp[0] = (char *)&msg.sender - (char *)&msg;
    disp[1] = (char *)&msg.score  - (char *)&msg;
    disp[2] = (char *)msg.layout  - (char *)&msg;
    MPI_Type_create_struct(3, blocklen, disp, type, &KLO_MESSAGE);
    MPI_Type_commit(&KLO_MESSAGE);


    MPI_Comm_size(MCW, &size);
    MPI_Comm_rank(MCW, &rank);


    // rank 0 is the master process that does the following:
    //  - provide all other processes with a unique seed for their random number generator
    //  - task each process with some configuration 
    //  - receive results and save only the best ones
    if (rank == 0) {
        srand(time(NULL));
        manage_jobs();
    } else {
        MPI_Status status;
        int seed = 0;
        MPI_Recv(&seed, 1, MPI_INT, 0, 0, MCW, &status);
        printf("Rank%d received seed: %d\n", rank, seed);
        srand(seed);
        process_jobs();
    }

    MPI_Finalize();
    return 0;
}

