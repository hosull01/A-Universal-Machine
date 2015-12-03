
//  segment.c
//  
//
//  Created 17/11/2015.
//
//

#include "segment.h"
#include "assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <stdint.h>
#include <stdbool.h>


Except_T Segment_callocError  = {"Could not create segment, out of memory"};
Except_T Offset_Error = {"Offset out of segment bounds\n"};

/* returns a new segment of specified size
 * returns NULL if out of memory
 */
inline S seg_create (uint32_t numElem)
{
    //assert (numElem > 0);
    numElem += 1;
    
    S new_seg = calloc(numElem, sizeof(uint32_t));
    if (new_seg == NULL) {
        RAISE(Segment_callocError);
        exit(2);
    }
    
    /* new_seg[0] contains the length of the segment */
    *(new_seg) = numElem;
    
    /* segment initialised with 0s */
    // for (uint32_t i = 1; i < numElem; i++) {
    //     *(new_seg + i) = 0;
    // }
    return new_seg;
}

/* add element to specified location
 * returns true if element was added to segment,
 * returns false if offset out of segment bounds
 */
inline void seg_put (S segment, uint32_t val, uint32_t offset)
{
    //assert(segment);
    
    offset++;
    if (offset >= *(segment)) {
        RAISE(Offset_Error);
        exit(3);
    }
    *(segment + offset) = val;
}

/* get element from specified location */
inline uint32_t seg_get (S segment, uint32_t offset)
{
    //assert(segment);
    
    offset++;
    if (offset >= *(segment)) {
        RAISE(Offset_Error);
        exit(3);
    }
    return *(segment + offset);
}

/* returns a new segment which is an exact copy of passed in segment */
inline S seg_copy(S segment)
{
    //assert(segment);
    uint32_t iter = 0;
    uint32_t len = *(segment);
    Segment new_seg = seg_create(len - 1);
    
    for (iter = 1; iter < len; iter++) {
        *(new_seg + iter) =  *(segment + iter);
    }
    
    return new_seg;
}

/* deletes a segment */
inline void seg_delete (S segment)
{
    //assert(segment);
    free(segment);
}

/* returns the length of a segment */
inline uint32_t seg_length(S segment)
{
    //assert(segment);
    return *(segment) - 1;
}
