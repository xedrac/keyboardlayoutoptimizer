#ifndef KEYBOARDLAYOUTOPTIMIZER_H
#define KEYBOARDLAYOUTOPTIMIZER_H

#include <stdint.h>
#include <string>
#include <map>
#include "configuration.h"

using namespace std;

/* 2*47 keys, to incorporate caps */
#define SHIFTMOD    (1<<16)

enum corpusmode {
    LETTERS     = 0x01,
    NUMBERS     = 0x02,
    PUNCTUATION = 0x04,
    WHITESPACE  = 0x08,
    SYMBOLS     = 0x10,
};

/* per key category */
//enum PenaltyType { HAND, ROW, FINGER, BASE };

enum HandType {
    LeftHand,
    RightHand
};

enum RowType {
    NumberRow,
    TopRow,
    HomeRow,
    BottomRow,
    NUMROWS
};

enum FingerType {
    FingerPinky,
    FingerRing,
    FingerMiddle,
    FingerIndex,
    FingerThumb
};


/* Weigh different parameters differently:
kb:  Base Weight     (Finger travel distance to type something)
kp:  Pentalty Weight (Key sequence to type something)
ks:  Stroke Weight   (Key sequence of triad) */
const double kb = 0.3555;
const double kp = 0.6423;
const double ks = 0.4268;

/* key sequence weights.  
   k1       = effort to type key1
   k1*k2    = effort to type key1+key2
   k1*k2*k3 = effort to type key1+key2+key3 */
const double k1 = 1;
const double k2 = 0.367;
const double k3 = 0.235;

//const double weight[3]          = { 1, 1.3088, 2.5948 };   // { hand, row, finger }
//const double handpenalty[NHAND] = { 0.0, 0.0 };            // { left_hand, right_hand }
//const double rowpenalty[NROW]   = { 1.5, 0.5, 0.0, 1.0 };  // { number_row, top_row, home_row, bottom_row }

//const double fingerpenalty[NFINGER] = { 1.0, 0.5, 0.0, 0.0, 0.0,    // { Lpinky, Lring, Lmid, Lindex, Lthumb
//                                        0.0, 0.0, 0.0, 0.5, 1.0 };  //   Rthumb, Rindex, Rmid, Rring, Rpinky }


class KeyboardLayoutOptimizer
{
public:
    KeyboardLayoutOptimizer();
    ~KeyboardLayoutOptimizer();

    double optimizeLayout(char *layout, int iterations, double t0, double p0, double k);
    void printLayoutTransition(int iteration, char *oldlayout, char *newlayout, double oldeffort, double neweffort, double p, double t, bool accept);
    void printLayout(char *layout);
    void printLayoutsSideBySide(char *layout1, char *layout2);
    void showLayouts();
    void showTriads(int sortbyfreq);
    void showDigraphs(int sortbyfreq);
    void buildCharToIndexMap(char *layout);
    bool parseTriads(const string &file, uint8_t mode);

    string getBestLayout()      { return _bestlayout; }
    double getBestLayoutScore() { return _bestlayoutscore; }

private:
    double getTriadEffort(int ikey1, int ikey2, int ikey3);
    double getTriadEffort(const string &triad);
    double computeTriadEffort(int ikey1, int ikey2, int ikey3);
    double computeLayoutEffort(char *layout);
    void swapLayoutKeys(char *layout, int minswaps, int maxswaps, uint8_t *mask);
    void printTriads();

private:
    char _layout[NUMKEYS+1];
    
    // stores the cost of typing any 3 keys in succession for a given layout
    double  _triadeffort[NUMKEYS][NUMKEYS][NUMKEYS];

    // tells the optimizer which keys it's allowed to move when optimizing
    uint8_t _layoutmask[NUMKEYS];

    // maps ascii characters to indices within the current layout
    uint8_t _chartoindex[0xFF];

    // map of all triads to their frequency as found in the corpus
    map<string, int> _triadmap;

    // total number of triads found in the corpus (not unique)
    int _triadcount;

    // frequency of all digraphs found in the corpus
    int _digraphs[0x7F][0x7F];

    string _bestlayout;
    double _bestlayoutscore;

    Configuration _config;
};


#endif // KEYBOARDLAYOUTOPTIMIZER_H
