#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <list>
#include "keyboardlayoutoptimizer.h"


char qwerty_layout[NUMKEYS+1]  = { "`1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./" };
char dvorak_layout[NUMKEYS+1]  = { "`1234567890[]',.pyfgcrl/=\\aoeuidhtns-;qjkxbmwvz" };
char colemak_layout[NUMKEYS+1] = { "`1234567890-=qwfpgjluy;[]\\arstdhneio'zxcvbkm,./" }; 
char workman_layout[NUMKEYS+1] = { "`1234567890-=qdrwbjfup;[]\\ashtgyneoi'zxmcvkl,./" };
char bulpkm_layout[NUMKEYS+1]  = { "`1234567890-='bulpkmyf;[]\\riaohdtensjzxcvqgw,./" };
char xfyl_layout[NUMKEYS+1]    = { "`1234567890-=xfyljkpuw;[]\\asinhdtero'zb.mqgc,v/" };
char test_layout[NUMKEYS+1]    = { "`1234567890-=tkpb'oqc,.[]\\r/;sxfzvgwluyemdnihja" };


// table containing information on which hand, row, finger
// a given key index corresponds to.
struct KeyInfo {
    HandType hand;
    RowType row;
    FingerType finger;

} keyInfoTable[NUMKEYS] = {
    { LeftHand,  NumberRow,  FingerPinky  },   // `
    { LeftHand,  NumberRow,  FingerRing   },   // 1
    { LeftHand,  NumberRow,  FingerRing   },   // 2
    { LeftHand,  NumberRow,  FingerMiddle },   // 3
    { LeftHand,  NumberRow,  FingerIndex  },   // 4
    { LeftHand,  NumberRow,  FingerIndex  },   // 5
    { LeftHand,  NumberRow,  FingerIndex  },   // 6
    { RightHand, NumberRow,  FingerIndex  },   // 7
    { RightHand, NumberRow,  FingerMiddle },   // 8
    { RightHand, NumberRow,  FingerMiddle },   // 9
    { RightHand, NumberRow,  FingerRing   },   // 0
    { RightHand, NumberRow,  FingerPinky  },   // [
    { RightHand, NumberRow,  FingerPinky  },   // ]

    { LeftHand,  TopRow,     FingerPinky  },   // '
    { LeftHand,  TopRow,     FingerRing   },   // ,
    { LeftHand,  TopRow,     FingerMiddle },   // .
    { LeftHand,  TopRow,     FingerIndex  },   // p
    { LeftHand,  TopRow,     FingerIndex  },   // y
    { RightHand, TopRow,     FingerIndex  },   // f
    { RightHand, TopRow,     FingerIndex  },   // g
    { RightHand, TopRow,     FingerMiddle },   // c
    { RightHand, TopRow,     FingerRing   },   // r
    { RightHand, TopRow,     FingerPinky  },   // l
    { RightHand, TopRow,     FingerPinky  },   // /
    { RightHand, TopRow,     FingerPinky  },   // =
    { RightHand, TopRow,     FingerPinky  },   // backslash 

    { LeftHand,  HomeRow,    FingerPinky  },   // a
    { LeftHand,  HomeRow,    FingerRing   },   // o
    { LeftHand,  HomeRow,    FingerMiddle },   // e
    { LeftHand,  HomeRow,    FingerIndex  },   // u
    { LeftHand,  HomeRow,    FingerIndex  },   // i
    { RightHand, HomeRow,    FingerIndex  },   // d
    { RightHand, HomeRow,    FingerIndex  },   // h
    { RightHand, HomeRow,    FingerMiddle },   // t
    { RightHand, HomeRow,    FingerRing   },   // n
    { RightHand, HomeRow,    FingerPinky  },   // s
    { RightHand, HomeRow,    FingerPinky  },   // -

    { LeftHand,  BottomRow,  FingerPinky  },   // ;
    { LeftHand,  BottomRow,  FingerRing   },   // q
    { LeftHand,  BottomRow,  FingerMiddle },   // j
    { LeftHand,  BottomRow,  FingerIndex  },   // k
    { LeftHand,  BottomRow,  FingerIndex  },   // x
    { RightHand, BottomRow,  FingerIndex  },   // b
    { RightHand, BottomRow,  FingerIndex  },   // m
    { RightHand, BottomRow,  FingerMiddle },   // w
    { RightHand, BottomRow,  FingerRing   },   // v
    { RightHand, BottomRow,  FingerPinky  },   // z
};
  

// Indexed by the rows indices of 3 keys (a triad)
// The value provided is a row cost multiplier, calculate
// by how the keys are arranged in rows (ie Top, Bottom, Top)
int rowFlagTable[NUMROWS][NUMROWS][NUMROWS] = {
    {{0, 1, 1, 1},     // [0][0][0], [0][0][1], [0][0][2], [0][0][3]
     {3, 1, 4, 4},     // [0][1][0], [0][1][1], [0][1][2], [0][1][3]
     {5, 5, 1, 4},     // [0][2][0], [0][2][1], [0][2][2], [0][2][3]
     {5, 5, 5, 1}},    // [0][3][0], [0][3][1], [0][3][2], [0][3][3]
  
    {{2, 3, 5, 5},     // [1][0][0], [1][0][1], [1][0][2], [1][0][3]
     {2, 0, 1, 1},     // [1][1][0], [1][1][1], [1][1][2], [1][1][3]
     {5, 3, 1, 4},     // [1][2][0], [1][2][1], [1][2][2], [1][2][3]
     {5, 5, 5, 1}},    // [1][3][0], [1][3][1], [1][3][2], [1][3][3]

    {{2, 5, 5, 5},     // [2][0][0], [2][0][1], [2][0][2], [2][0][3]
     {6, 2, 3, 5},     // [2][1][0], [2][1][1], [2][1][2], [2][1][3]
     {2, 2, 0, 1},     // [2][2][0], [2][2][1], [2][2][2], [2][2][3]
     {5, 5, 3, 1}},    // [2][3][0], [2][3][1], [2][3][2], [2][3][3]

    {{2, 5, 5, 5},     // [3][0][0], [3][0][1], [3][0][2], [3][0][3]
     {6, 2, 5, 5},     // [3][1][0], [3][1][1], [3][1][2], [3][1][3]
     {6, 6, 2, 3},     // [3][2][0], [3][2][1], [3][2][2], [3][2][3]
     {2, 2, 2, 0}},    // [3][3][0], [3][3][1], [3][3][2], [3][3][3]
};


inline int random_range(int min, int max)
{
    int range = max-min+1;
    return min+int(range*(double)rand()/(RAND_MAX+1.0));
}


// Specifies which keys are allowed to be swapped when optimizing
uint8_t layoutMask[NUMKEYS] = {
  // ` 1 2 3 4 5 6 7 8 9 0 - = q w e r t y u i o p [ ] \ a s d f g h j k l ; ' z x c v b n m , . /
     0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,0,0
};


KeyboardLayoutOptimizer::KeyboardLayoutOptimizer()
{
    srand(time(0));

    for (int i=0; i<NUMKEYS; i++) {
        for (int j=0; j<NUMKEYS; j++) {
            for (int k=0; k<NUMKEYS; k++) {
                _triadeffort[i][j][k] = 0.0;
            }
        }
    }
}


KeyboardLayoutOptimizer::~KeyboardLayoutOptimizer()
{
}


// rebuild character to index mapping specific to _layout
void KeyboardLayoutOptimizer::buildCharToIndexMap(char *layout)
{
    for (int i=0; i<NUMKEYS; i++)
        _chartoindex[(uint8_t)layout[i]] = i;
}




// Compute the (naive) effort for a given layout
double KeyboardLayoutOptimizer::computeLayoutEffort(char *layout)
{
    buildCharToIndexMap(layout);
    double effort = 0.0;

    map<string, int>::iterator it;
    for (it = _triadmap.begin(); it != _triadmap.end(); it++) {
        effort += getTriadEffort(it->first) * it->second;
    }

    return effort / (double)_triadcount;
}



// Generate a new layout by randomly swapping some of the keys
void KeyboardLayoutOptimizer::swapLayoutKeys(char *layout, int minswaps, int maxswaps, uint8_t *mask)
{
    int key1 = 0;
    int key2 = 0;

    int nswaps = random_range(minswaps, maxswaps);

    for (int i=0; i<nswaps; i++) {
        while (key1=random_range(0, NUMKEYS), !mask[key1])
            ;

        //while (key2=random_range(0, NUMKEYS), !mask[key2] && key2 != key1)
        while (key2=random_range(0, NUMKEYS), !mask[key2] && key2 != key1)
            ;
    
        char hold = layout[key1];
        layout[key1] = layout[key2];
        layout[key2] = hold;    
    }
}


void KeyboardLayoutOptimizer::printLayout(char *layout)
{
    int i = 0;

    printf("\t");
    for (i=0; i<13; i++)
        printf("%c ", layout[i]);
    //printf("    ");
    //for (i=0; i<13; i++)
    //    printf("%c ", shiftModifiedChar[layout[i]]);


    printf("\n\t");
    for (i=13; i<26; i++)
        printf("%c ", layout[i]);
    //printf("    ");
    //for (i=13; i<26; i++)
    //    printf("%c ", shiftModifiedChar[layout[i]]);


    printf("\n\t");
    for (i=26; i<37; i++)
        printf("%c ", layout[i]);
    //printf("        ");
    //for (i=26; i<37; i++)
    //    printf("%c ", shiftModifiedChar[layout[i]]);


    printf("\n\t");
    for (i=37; i<47; i++)
        printf("%c ", layout[i]);
    //printf("          ");
    //for (i=37; i<47; i++)
    //    printf("%c ", shiftModifiedChar[layout[i]]);

    printf("\n");
}


void KeyboardLayoutOptimizer::printLayoutsSideBySide(char *layout1, char *layout2)
{
    int i = 0;

    printf("\t");
    for (i=0; i<13; i++)
        printf("%c ", layout1[i]);
    printf("\t");
    for (i=0; i<13; i++)
        printf("%c ", layout2[i]);

    printf("\n\t");
    for (i=13; i<26; i++)
        printf("%c ", layout1[i]);
    printf("\t");
    for (i=13; i<26; i++)
        printf("%c ", layout2[i]);

    printf("\n\t  ");
    for (i=26; i<37; i++)
        printf("%c ", layout1[i]);
    printf("\t  ");
    for (i=26; i<37; i++)
        printf("%c ", layout2[i]);

    printf("\n\t   ");
    for (i=37; i<47; i++)
        printf("%c ", layout1[i]);
    printf(" \t   ");
    for (i=37; i<47; i++)
        printf("%c ", layout2[i]);

    printf("\n");
}



void KeyboardLayoutOptimizer::printLayoutTransition(int iteration,
                                                    char *oldlayout,
                                                    char *newlayout,
                                                    double oldeffort,
                                                    double neweffort,
                                                    double p,
                                                    double t,
                                                    bool accept)
{
    printf("--------------------------------------------------------------------------------\n");
    double effortdelta = neweffort-oldeffort;
    printf("iter: %d   effort: %.4f -> %.4f  d=%.2f  p=%.2f  t=%.2f  %s/%s\n",
            iteration,
            oldeffort,
            neweffort,
            effortdelta,
            p,
            t,
            (effortdelta<0? "better": "worse"),
            (accept? "accept": "reject"));
    printf("--------------------------------------------------------------------------------\n");
    printLayoutsSideBySide(oldlayout, newlayout);
    printf("\n");
}


double KeyboardLayoutOptimizer::optimizeLayout(char *layout, int iterations, double t0, double p0, double k)
{
    char prev_layout[NUMKEYS+1];
    char curr_layout[NUMKEYS+1];
    double prev_effort = 0.0;
    double curr_effort = 0.0;
    double effortdelta = 0.0;
    double t;
    double p;
    int i = 0;
    int iwindow = 0;

    memcpy(prev_layout, layout, NUMKEYS);
    memcpy(curr_layout, layout, NUMKEYS);
    prev_layout[NUMKEYS]=0;
    curr_layout[NUMKEYS]=0;
    prev_effort = computeLayoutEffort(prev_layout);    

    struct timespec ts0, ts1;
    clock_gettime(CLOCK_MONOTONIC, &ts0);

    do {
        curr_effort = computeLayoutEffort(curr_layout);        
        effortdelta = curr_effort - prev_effort;

        t = t0 * exp((-1*((double)i)*k/(double)iterations));
        p = p0 * exp(-1*fabs(effortdelta)/t);
        if (p > 1.0) {
            p = 1.0;
        }

        // Always accept new layout if better than previous layout
        if (effortdelta < 0) {
            printLayoutTransition(i, prev_layout, curr_layout, prev_effort, curr_effort, p, t, true);
            prev_effort = curr_effort;
            memcpy(prev_layout, curr_layout, NUMKEYS);

            // Sometimes accept new layout if worse than previous
        } else {
            if (p*10000 > random_range(0, 10000)) {
                printLayoutTransition(i, prev_layout, curr_layout, prev_effort, curr_effort, p, t, true);
                prev_effort = curr_effort;
                memcpy(prev_layout, curr_layout, NUMKEYS);
            } else {
                //printLayoutTransition(i, prev_layout, curr_layout, prev_effort, curr_effort, p, t, false);
            }
        }

        if (iwindow++ == 32768) {  // print average layouts per/sec calculated
            clock_gettime(CLOCK_MONOTONIC, &ts1);
            double elapsed = (ts1.tv_sec - ts0.tv_sec) + (ts1.tv_nsec - ts0.tv_nsec)/1000000000.0;
            printf("avg_layouts_per_sec: %.2f\n", iwindow/elapsed);
            clock_gettime(CLOCK_MONOTONIC, &ts0);
            iwindow = 0;
        }

        swapLayoutKeys(curr_layout, 1, 3, layoutMask);
    } while (++i < iterations);

    printf("%3.6f = \"%s\"\n", prev_effort, prev_layout);
    printLayout(prev_layout);

    return prev_effort;
}


// parse a text file into 3-letter triads and calculate effort for each triad
bool KeyboardLayoutOptimizer::parseTriads(const string &file, uint8_t mode)
{
    FILE *fp = fopen(file.c_str(), "r");
    if (!fp)
        return false;

    char buf[1024*32];
    char *c1 = &buf[0];
    char *c2 = &buf[1];
    char *c3 = &buf[2];

    string triad(4, 0);
    int batchsize;
    char c;

    while (!feof(fp) && (batchsize=fread(buf, 1, sizeof(buf), fp))) {
        if (batchsize < 3)
            break;

        triad[0] = tolower(*c1);
        triad[1] = tolower(*c2);
        triad[2] = tolower(*c3);
        //indexTriadEffort(triad);    
        _triadmap[triad]++;
        _triadcount++;
        _digraphs[(int)triad[0]][(int)triad[1]]++;

        for (int i=3; i<batchsize; i++) {
            c1 = c2;
            c2 = c3;
            c3 = &buf[i];
            c = *c3;

            // Skip any characters that are not part of our mode
            // TODO:  Should build a mode map, indexed by ascii to simply
            //        lookup the mode(s) a given character falls under
            while ((c<0x20 || c>=0x7F)         ||
                   // 'c' is a letter but the LETTERS flag is not set
                   (!(mode & LETTERS) &&
                    ((c>=0x41 && c<=0x5A) ||       // uppercase letters
                     (c>=0x61 && c<=0x7A)))    ||  // lowercase letters
                     
                   // 'c' is a number but the NUMBERS flag is not set
                   (!(mode & NUMBERS) &&
                    (c>=0x30 && c<=0x39))      ||  // 0-9

                   // 'c' is whitespace but the WHITESPACE flag is not set
                   (!(mode & WHITESPACE) &&
                    (c==0x20))                 ||  // space

                   // 'c' is a punctuation but the PUNCTUATION flag is not set
                   (!(mode & PUNCTUATION) &&
                    ((c>=0x21 && c<=0x22) ||       // ! "
                     (c>=0x27 && c<=0x29) ||       // ' ( )
                     (c>=0x2C && c<=0x2F) ||       // , - . /
                     (c>=0x3A && c<=0x3B) ||       // : ;
                     (c>=0x5B && c<=0x5D) ||       // [ \ ]
                     (c==0x3F ||                   // ? 
                      c==0x5F ||                   // _
                      c==0x7B ||                   // {
                      c==0x7D)))               ||  // }

                   // 'c' is a symbol but the SYMBOL flag is not set
                   (!(mode & SYMBOLS) &&            
                    ((c>=0x23 && c<=0x26) ||       // # $ % &
                     (c>=0x2A && c<=0x2B) ||       // * +
                     (c>=0x3C && c<=0x3E) ||       // < = >
                     (c==0x5E ||                   // ^
                      c==0x60 ||                   // `
                      c==0x7C ||                   // |
                      c==0x7E))))                  // ~
            {
                if (i >= batchsize-1)
                    return true;
                c3 = &buf[++i];
                c = *c3;
            }
                            
            triad[0] = tolower(*c1);
            triad[1] = tolower(*c2);
            triad[2] = tolower(*c3);
            //indexTriadEffort(triad);
            _triadmap[triad]++;
            _triadcount++;
            _digraphs[(int)triad[0]][(int)triad[1]]++;
        }    
    }

    fclose(fp);
    return true;
}


void KeyboardLayoutOptimizer::printTriads()
{
    map<string, int>::iterator it;
    for (it=_triadmap.begin(); it != _triadmap.end(); it++) {
        printf("%d: %s\n", it->second, it->first.c_str());    
    }
}


void KeyboardLayoutOptimizer::showLayouts()
{ 
    double qwerty_effort  = computeLayoutEffort(qwerty_layout);
    double dvorak_effort  = computeLayoutEffort(dvorak_layout);
    double colemak_effort = computeLayoutEffort(colemak_layout);
    double workman_effort = computeLayoutEffort(workman_layout);
    double bulpkm_effort  = computeLayoutEffort(bulpkm_layout);
    double xfyl_effort    = computeLayoutEffort(xfyl_layout);
    double test_effort    = computeLayoutEffort(test_layout);

    printf("Comparison: \n\n");
    printf("%20s: %10.8f\n", "Qwerty",  qwerty_effort);
    printf("%20s: %10.8f\n", "Dvorak",  dvorak_effort);
    printf("%20s: %10.8f\n", "Colemak", colemak_effort);
    printf("%20s: %10.8f\n", "Workman", workman_effort);
    printf("%20s: %10.8f\n", "Bulpkm",  bulpkm_effort);
    printf("%20s: %10.8f\n", "Xfyl",    xfyl_effort);
    printf("%20s: %10.8f\n", "Test",    test_effort);
    printf("\n\n");
}


struct entry { 
    string str;
    int count;
};

bool comp_entry(const entry &a, const entry &b) {
    return (a.count > b.count); 
}


void KeyboardLayoutOptimizer::showTriads(int sortbyfreq)
{
    printf("\nTRIADS\n---------\n\n");

    // Don't sort anything
    if (!sortbyfreq) {
        map<string, int>::iterator it;
        for (it = _triadmap.begin(); it != _triadmap.end(); it++) {
            if (it->second < 10)
                continue;
            printf("%6d: %s\n", it->second, it->first.c_str());
        }

    // Sort by most common
    } else {
        list<entry> triadlist;
        map<string, int>::iterator it;
        for (it = _triadmap.begin(); it != _triadmap.end(); it++) {
            if (it->second < 10)
                continue;
            
            entry tx = {it->first, it->second};
            triadlist.push_back(tx);
        }    

        triadlist.sort(comp_entry);    

        list<entry>::iterator itr;
        for (itr=triadlist.begin(); itr != triadlist.end(); itr++) {
            printf("%6d: %s\n", itr->count, itr->str.c_str());
        }
    }
}


void KeyboardLayoutOptimizer::showDigraphs(int sortbyfreq)
{
    printf("\nDIGRAPHS\n---------\n\n");    
    
    list<entry> digraphlist;
    for (int i=0; i<0x7F; i++) {
        for (int j=0; j<0x7F; j++) {
            if (!_digraphs[i][j])
                continue;
            string s;
            s += (char)i;
            s += (char)j;
            entry dx = {s, _digraphs[i][j]};
            digraphlist.push_back(dx);
        }
    }

    digraphlist.sort(comp_entry);
    list<entry>::iterator itr;
    for (itr=digraphlist.begin(); itr != digraphlist.end(); itr++) {
        printf("%4d: %s\n", itr->count, itr->str.c_str());
    }
}


int main(int argc, char **argv)
{
    KeyboardLayoutOptimizer klo;

    //if (!klo.initPathCost("conf/pathcost.conf")) {
    //    printf("Unable to load conf/pathcost.conf\n");
    //    return 0;
    //}

    // This is needed for parseTriads()
    klo.buildCharToIndexMap(qwerty_layout);

    if (!klo.parseTriads("corpus/corpus.txt", LETTERS /*| NUMBERS | PUNCTUATION | SYMBOLS*/)) {
        fprintf(stderr, "Error parsing triads from '%s'\n", "corpus/corpus.txt");
        //exit(1);
    }

    //klo.showTriads(1);
#if 1
    klo.showLayouts();
    //show_triads(1);
    //show_digraphs(1);
        
    printf("Optimizing Layout\n");
    int rounds = 1;
    int iterations = 1000000;
    struct timeval start, end;
    float best=100.0, curr;
    char *layout = qwerty_layout;
    double t0=0.5;
    double p0=0.3;   /* Set to zero to refuse transitions to worse layouts */
    double k =500.0; /* set higher to cooldown faster */

    gettimeofday(&start, NULL);

    for (int i=0; i<rounds; i++) {
        curr = klo.optimizeLayout(layout, iterations, t0, p0, k);
        if (curr < best)
            best = curr;
    }

    gettimeofday(&end, NULL);
    int elapsed = end.tv_sec - start.tv_sec;
    printf("\n\nRounds: %d of %d iterations\n", rounds, iterations);
    printf("Elapsed time: %d seconds (%d layouts per second)\n", elapsed, iterations/elapsed); 
    printf("Best Layout Found: %f\n\n", best);
#endif
    return 0;    
}


double KeyboardLayoutOptimizer::computeTriadEffort(int ikey1, int ikey2, int ikey3)
{
    const KeyInfo &key1 = keyInfoTable[ikey1];
    const KeyInfo &key2 = keyInfoTable[ikey2];
    const KeyInfo &key3 = keyInfoTable[ikey3];

    double k1beffort = _config.baseEffort(ikey1);
    double k2beffort = _config.baseEffort(ikey1);
    double k3beffort = _config.baseEffort(ikey1);

    //int handflag   = 0;
    int fingerflag = 0;
    int rowflag    = 0;

    // Add penalty for how the triad is distributed among the hands
    // 0 for LRR/LLR/RLL/RRL
    // 1 for LRL/RLR
    // 2 for LLL/RRR
    //if (key1.hand == key3.hand)
    //    handflag = (key2.hand == key3.hand? 2: 1);

    // all keys on same hand
    if (key1.hand == key2.hand && key2.hand == key3.hand) {
        if (key1.finger < key2.finger) {
            if      (key2.finger <  key3.finger) { fingerflag = 0; }
            else if (key2.finger == key3.finger) { fingerflag = (ikey2 != ikey3? 5: 0); }
            else if (key1.finger == key3.finger) { fingerflag = 4; }
            else if (key1.finger <  key3.finger) { fingerflag = 2; }
            else /* key3.finger < key1.finger */ { fingerflag = 3; }

        } else if (key1.finger == key2.finger) {
            if      (key2.finger <  key3.finger) { fingerflag = (ikey1 != ikey2)? 4: 1; }
            else if (key2.finger == key3.finger) { fingerflag = (ikey1 != ikey2 && ikey2 != ikey3 && ikey1 != ikey3)? 7: 5; }
            else if (key2.finger >  key3.finger) { fingerflag = (ikey1 != ikey2)? 5: 1; }

        } else {  /* key1.finger > key2.finger */
            if      (key2.finger > key3.finger)  { fingerflag = 3; }
            else if (key2.finger == key3.finger) { fingerflag = (ikey2 != ikey3)? 4: 1; }
            else if (key1.finger == key3.finger) { fingerflag = 4; }
            else if (key2.finger < key3.finger)  { fingerflag = 5; }
            else /* key1.finger < key3.finger */ { fingerflag = 3; }
        }

    // first two keys on same hand
    } else if (key1.hand == key2.hand) {
        if (key1.finger == key2.finger) {
            fingerflag = (ikey1 != ikey2)? 3: 1;
        } else if (key1.finger > key2.finger) {
            fingerflag = 2;
        }

    // last two keys on same hand 
    } else if (key2.hand == key3.hand) {
        if (key2.finger == key3.finger) {
            fingerflag = (ikey2 != ikey3)? 3: 1;
        } else if (key2.finger > key3.finger) {
            fingerflag = 2;
        }

    // no sequential keys on same hand
    } else {  /* key1.hand == key3.hand */
        fingerflag = 0;
    }

    rowflag = rowFlagTable[key1.row][key2.row][key3.row];

    //double stroke_effort = kb*(k1*k1beffort + (1 + k2*k2beffort * (1 + k3*k3beffort)));
    //double path_effort   = 1.0*handflag + 0.3*fingerflag + 0.3*rowflag;
    double stroke_effort = 2.0*(k1*k1beffort + (1 + k2*k2beffort * (1 + k3*k3beffort)));
    double path_effort   = 0.3*fingerflag + 0.4*rowflag;

    return stroke_effort + path_effort;
}


// Adds 1 to the count of given triad
double KeyboardLayoutOptimizer::getTriadEffort(int ikey1, int ikey2, int ikey3)
{
    double *effort = &_triadeffort[ikey1][ikey2][ikey3];
    // Don't recalculate if we've already calculated for this triad key sequence
    if (*effort > 0.0)
        return *effort;

    *effort = computeTriadEffort(ikey1, ikey2, ikey3);
    return *effort;
}

// Overloadad function - adds 1 to the count of given triad
double KeyboardLayoutOptimizer::getTriadEffort(const string &triad)
{
    int ikey1 = _chartoindex[(int)triad[0]];
    int ikey2 = _chartoindex[(int)triad[1]];
    int ikey3 = _chartoindex[(int)triad[2]];


    return getTriadEffort(ikey1, ikey2, ikey3);
}
