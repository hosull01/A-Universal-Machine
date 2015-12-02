#ifndef load_program_h
#define load_program_h

#include <stdlib.h>
#include <stdio.h>
#include "segment.h"

/* takes a program and returns a sequence with all instructions */
extern Segment open_prog(char *filename);

#endif