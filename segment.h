//
//  segment.h
//  
//
//  Created 17/11/2015.
//
//

#ifndef segment_h
#define segment_h

#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <stdint.h>
#include <stdbool.h>
#include <except.h>

#define S Segment
/* a segment pointer to a uint32_t dynamic array */
typedef uint32_t* Segment;

extern Except_T Segment_callocError;
extern Except_T Offset_Error;

/* returns a new segment of specified size */
extern S seg_create (uint32_t numElem);

/* add element to specified location */
extern void seg_put (S segment, uint32_t val, uint32_t offset);

/* get element from specified location */
extern uint32_t seg_get (S segment, uint32_t offset);

extern S seg_copy(S segment);

/* returns the length of a segment */
extern  uint32_t seg_length(S segment);

/* deletes a segment */
extern void seg_delete (S segment);

#endif
