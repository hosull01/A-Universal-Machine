#ifndef memory_h
#define memory_h

#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include "segment.h"
#include "umstack.h"

//typedef struct Memory_T *Memory_T;

typedef struct Memory_T {
	Segment* realMem;
    uint32_t recent_id1, recent_id2;    /* cached segments id */
    Segment recent_seg1, recent_seg2; /* cached segments */
	umStack unmappedIDs;
	uint32_t memUsed;
    uint32_t memCap;
    uint32_t segCount; 
} *Memory_T;


Memory_T new_Memory(Segment program);

/* puts a word into a particular location */
extern void mem_write (Memory_T memory, uint32_t word, uint32_t segID, 
					   uint32_t offset);

/* pulls a sequence from a particular location */
extern uint32_t mem_read (Memory_T memory, uint32_t segID, uint32_t offset);

/* returns the program segment */
extern Segment mem_getProg (Memory_T memory);

/* creates a sequence of a particular size */
extern uint32_t map(Memory_T memory, uint32_t size);

/* to clear a sequence at a particular location */
extern void unmap(Memory_T memory, uint32_t segID);

/* returns lengh of segment in memory */ 
extern uint32_t memory_seglength(Memory_T memory, uint32_t segID);

/* returns the length of the duplicate segment */
extern uint32_t duplicate(Memory_T memory, uint32_t orig_ID, uint32_t dup_ID,
	Segment* UMprogram);

extern void mem_free(Memory_T memory);

#endif