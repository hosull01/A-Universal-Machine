#ifndef emulator_h
#define emulator_h

#include <stdlib.h>
#include <stdio.h>
#include "segment.h"

//#define E Emulator_T

//typedef struct E Emulator_T;

/* constructor, initialises cpu, realMem */
//E new_emulator(Segment UMprogram);

/* 
 * function for running UM program and loops though words in $m[0] 
 * calls a function to execute the word
 */
extern void run_emulator(Segment UMprogram);

#endif