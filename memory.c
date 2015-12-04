#include "memory.h"
#include "seq.h"
#include "umstack.h"
#include "assert.h"

#define CAPACITY 250000000	/* 1000MB of memory */

// struct Memory_T {
// 	Segment* realMem;
//     uint32_t recent_id1, recent_id2;    /* cached segments id */
//     Segment recent_seg1, recent_seg2; /* cached segments */
// 	umStack unmappedIDs;
// 	uint32_t memUsed;
//     uint32_t memCap;
//     uint32_t segCount; 
// };

/*
 * purp: constructor for new memory 
 * args: a segment of program instructions 
 * does: initializes real memory and addes program instructions to first slot
 * rets: a struct Memory_T
 */
Memory_T new_Memory(Segment program)
{
    //assert(program);
	Memory_T memory = malloc(sizeof(*memory));

    memory->memCap = 31250000;

	/* create new memory & add new instructions */ 
	//memory->realMem = malloc(sizeof(memory->realMem));
    memory->realMem = calloc(memory->memCap, sizeof(Segment));
    *(memory->realMem) = program;
    memory->segCount = 1;

	//Seq_addhi(memory->realMem, (void *)program);

	memory->memUsed = *(program);

	/* create stack to account unmappedIDs */ 
	memory->unmappedIDs = Stack_new();

    /* initialising id of cached segments */
    memory->recent_id1 = 0;
    memory->recent_seg1 = program;
    memory->recent_id2 = -1;

	return memory;
}

/*
 * purp: updates existing word in a segment 
 * args: struct M, new word, which segment, where in segment 
 * does: replaces an existing word with a new word 
 * rets: N/A 
 */
inline void mem_write (Memory_T memory, uint32_t word, uint32_t segID, 
 	                   uint32_t offset)
 {
     //assert(memory);

     if (!(memory->recent_id1 ^ segID)) { 
        /* offset incremented bc segments are indexed from 1 */
         *(memory->recent_seg1 + (offset + 1)) = word;
         return;
     }

     if (!(memory->recent_id2 ^ segID)) {
        *(memory->recent_seg2 + (offset + 1)) = word;
        return;
     }

     if (!(segID & 1)) {
        memory->recent_id1 = segID;
        memory->recent_seg1 = *(memory->realMem + segID);
        *(memory->recent_seg1 + (offset + 1)) = word;
     } else {
        memory->recent_id2 = segID;
        memory->recent_seg2 = *(memory->realMem + segID);
        *(memory->recent_seg2 + (offset + 1)) = word;
     
    }
 }


 /*
 * purp: retrieves a word from a segment 
 * args: struct M, which segment, where in segment 
 * does: pulls a word from a particular location
 * rets: the word 
 */
inline uint32_t mem_read (Memory_T memory, uint32_t segID, uint32_t offset)
{
    //assert(memory);

    if (memory->recent_id1 == segID) { 
        /* offset incremented bc segments are indexed from 1 */
        return *(memory->recent_seg1 + (offset + 1));
    }

    if (memory->recent_id2 == segID) {
        return *(memory->recent_seg2 + (offset + 1));
    }

    if (!(segID & 1)) {
        memory->recent_id1 = segID;
        memory->recent_seg1 = *(memory->realMem + segID);
        return *(memory->recent_seg1 + (offset + 1));
     } else {
        memory->recent_id2 = segID;
        memory->recent_seg2 = *(memory->realMem + segID);
        return *(memory->recent_seg2 + (offset + 1));
     
    }

}

inline Segment mem_getProg (Memory_T memory)
{
    //assert(memory);

    return *(memory->realMem);
}


/*
 * purp: creating a new segment 
 * args: struct M, the size of new segment 
 * does: creates a segment of particular size and inserts to approrpiate place
 * rets: ID of the new segment 
 */
inline uint32_t map(Memory_T memory, uint32_t size)
{
    //assert(memory);
    
	if((memory->memUsed + size + 1) <= CAPACITY) {
       // fprintf(stderr, "I am mapping memUsed is %u\n", memory->memUsed);
		Segment new_seg = seg_create(size);
		uint32_t new_segID;
        uint32_t cap = memory->memCap;

		if(Stack_empty(memory->unmappedIDs)) {
            if (memory->segCount == cap) {
                cap = cap * 2;
                memory->realMem = realloc(memory->realMem, (sizeof(Segment) * cap));
                memory->memCap = cap;
                // fprintf(stderr, "i pass the test\n");
                // Segment test = *(memory->realMem + memory->segCount-1);
                // fprintf(stderr, "i pass the test AGAIN\n");
            }
			new_segID = memory->segCount;

		} else {
            new_segID = Stack_pop(memory->unmappedIDs);
            //new_segID = *IDptr;
			//Seq_put(memory->realMem, new_segID, new_seg);
            //free(IDptr);
		}
        
        *(memory->realMem + new_segID) = new_seg;
        memory->segCount++;
		memory->memUsed += size + 1;
		return new_segID;
	}
    //fprintf(stderr, "I am mapping but returning 0 memused is %u\n", memory->memUsed);
	return 0;
}


/*
 * purp: frees memory  
 * args: struct M, which segment 
 * does: deletes a segment 
 * rets: N/A 
 */
inline void unmap(Memory_T memory, uint32_t segID)
{
    //assert(memory && segID);
    Segment unmapped_seg = *(memory->realMem + segID);
    uint32_t seg_size = *(unmapped_seg);

	
	/* Delete element and place NULL has empty placeholder */ 
	//umapped_seg = Seq_put(memory->realMem, segID, NULL);
    seg_delete(unmapped_seg);
    memory->segCount--;
    
    /* free memory */
	memory->memUsed -= seg_size;
    //fprintf(stderr, "I am unmapping but  memused is %u\n", memory->memUsed);

    /* update cache ids */
    if (memory->recent_id1 == segID) {
        memory->recent_id1 = -1;
    }
    if (memory->recent_id2 == segID) {
        memory->recent_id2 = -1;
    }
    
	Stack_push(memory->unmappedIDs, segID);
}

inline uint32_t memory_seglength(Memory_T memory, uint32_t segID)
{
    //assert(memory);
    
	Segment segment = *(memory->realMem + segID);

	return *(segment);
}

inline uint32_t duplicate(Memory_T memory, uint32_t toDup_ID, uint32_t toRep_ID,
    Segment* UMprogram)
{
    //assert(memory);
    
    uint32_t size_dup;
    uint32_t size_rep;
    
    /* duplicate segment */
	Segment seg_to_duplicate = seg_copy(*(memory->realMem + toDup_ID));
    *UMprogram = seg_to_duplicate;

    
    /* place duplicate segment in new memory location */
    Segment seg_to_replace = *(memory->realMem + toRep_ID);

    *(memory->realMem + toRep_ID) =  seg_to_duplicate;
    
    size_dup = *(seg_to_duplicate);
    size_rep = *(seg_to_replace);
    
    /* update memory */
    memory->memUsed = memory->memUsed + size_dup - size_rep + 2;
    
    /* delete item that was in the duplicate's current location */
    seg_delete(seg_to_replace);
  

	return *(seg_to_duplicate);
}

inline void mem_free(Memory_T memory)
{
    //assert(memory);
    (void)memory;
    
    /* clear memory sequence */
//     while (memory->segCount*) {
//         seg_delete(memory->*(realMem));
//     }
//     Seq_free(&memory->realMem);
    
//     /* clear unmapped_ids stack */
// //    while (!Stack_empty(memory->unmappedIDs)) {
// //        free(Stack_pop(memory->unmappedIDs));
// //    }
//     Stack_free(&memory->unmappedIDs);
    
//     free(memory);
}











