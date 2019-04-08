#ifndef __ACTION_CODE__
#define __ACTION_CODE__

// moves

#define ACT_NONE            0
#define ACT_FORWARD			1
#define ACT_BACK			2
#define ACT_STRIF_LEFT		3
#define ACT_STRIF_RIGHT		4
#define ACT_ASCEND          5
#define ACT_DESCEND         6

// looks
#define ACT_TRUN_LEFT       7
#define ACT_TURN_RIGHT      8
#define ACT_TURN_DOWN       9
#define ACT_TURN_UP        10


// mise joysticks or touch pad

#define ACT_X              11
#define ACT_Y              12


// special buttons

#define ACT_BUTTON1        13
#define ACT_BUTTON2	       14
#define ACT_BUTTON3        15

// jumps

#define ACT_JUMP           16

// interective

#define ACT_USE            17

// special moves
#define ACT_ROLL_LEFT      18
#define ACT_ROLL_RIGHT     19


// keyboard keys
/*
    we don't need these
*/

// max action
#define ACT_MAX            128


// actions

typedef struct Action {
    int Code;
    int Actived;
    int Started;
    int Stoped;
    float Range;   // for joystick and mice
}Action;

#endif