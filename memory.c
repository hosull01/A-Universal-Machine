#include "memory.h"
#include "seq.h"
#include "stack.h"
#include "assert.h"

#define CAPACITY 250000000	/* 1000MB of memory */

struct Memory_T {
	Seq_T realMem;
	Stack_T unmappedIDs;
	int memUsed;
};

/*
 * purp: constructor for new memory 
 * args: a segment of program instructions 
 * does: initializes real memory and addes program instructions to first slot
 * rets: a struct Memory_T
 */
Memory_T new_Memory(Segment program)
{
    assert(program);
	Memory_T memory = malloc(sizeof(*memory));

	/* create new memory & add new instructions */ 
	memory->realMem = Seq_new(0);
	Seq_addhi(memory->realMem, (void *)program);

	memory->memUsed = seg_length(program) + 1;

	/* create stack to account unmappedIDs */ 
	memory->unmappedIDs = Stack_new();

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
     assert(memory);
     
     Segment s = Seq_get(memory->realMem, segID);
     
     // if s is null send error message
     seg_put(s, word, offset);
 }


 /*
 * purp: retrieves a word from a segment 
 * args: struct M, which segment, where in segment 
 * does: pulls a word from a particular location
 * rets: the word 
 */
inline uint32_t mem_read (Memory_T memory, uint32_t segID, uint32_t offset)
{
    assert(memory);

    Segment segment = Seq_get(memory->realMem, segID);

	return seg_get(segment, offset);
}

inline Segment mem_getProg (Memory_T memory)
{
    assert(memory);

    return Seq_get(memory->realMem, 0);
}


/*
 * purp: creating a new segment 
 * args: struct M, the size of new segment 
 * does: creates a segment of particular size and inserts to approrpiate place
 * rets: ID of the new segment 
 */
inline uint32_t map(Memory_T memory, uint32_t size)
{
    assert(memory);
    
	if((memory->memUsed + size + 1) <= CAPACITY) {
		Segment new_seg = seg_create(size);
		uint32_t new_segID;
        uint32_t* IDptr;

		if(Stack_empty(memory->unmappedIDs) == 1) {
			new_segID = Seq_length(memory->realMem);
			Seq_addhi(memory->realMem, new_seg);
		} else {
            IDptr = (uint32_t *)Stack_pop(memory->unmappedIDs);
            new_segID = *IDptr;
			Seq_put(memory->realMem, new_segID, new_seg);
            free(IDptr);
		}
        
		memory->memUsed += size + 1;
		return new_segID;
	}

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
    assert(memory && segID);
    
	uint32_t seg_size = seg_length(Seq_get(memory->realMem, segID));
    Segment umapped_seg;
    
	uint32_t *uID = malloc(sizeof(uint32_t));
	*uID = segID;
	
	/* Delete element and place NULL has empty placeholder */ 
	umapped_seg = Seq_put(memory->realMem, segID, NULL);
    seg_delete(umapped_seg);
    
    /* free memory */
	memory->memUsed -= seg_size + 1;
    
	Stack_push(memory->unmappedIDs, (void *) uID);
}

inline uint32_t memory_seglength(Memory_T memory, uint32_t segID)
{
    assert(memory);
    
	Segment segment = Seq_get(memory->realMem, segID);

	return seg_length(segment);
}

inline uint32_t duplicate(Memory_T memory, uint32_t toDup_ID, uint32_t toRep_ID,
    Segment* UMprogram)
{
    assert(memory);
    
    uint32_t size_dup;
    uint32_t size_rep;
    
    /* duplicate segment */
	Segment seg_to_duplicate = seg_copy(Seq_get(memory->realMem, toDup_ID));
    *UMprogram = seg_to_duplicate;

    
    /* place duplicate segment in new memory location */
    Segment seg_to_replace = Seq_put(memory->realMem, toRep_ID,
				 seg_to_duplicate);
     
    
    size_dup = seg_length(seg_to_duplicate);
    size_rep = seg_length(seg_to_replace);
    
    /* update memory */
    memory->memUsed = memory->memUsed + size_dup - size_rep + 2;
    
    /* delete item that was in the duplicate's current location */
    seg_delete(seg_to_replace);
  

	return seg_length(seg_to_duplicate);
}

inline void mem_free(Memory_T memory)
{
    assert(memory);
    
    /* clear memory sequence */
    while (Seq_length(memory->realMem)) {
        seg_delete(Seq_remhi(memory->realMem));
    }
    Seq_free(&memory->realMem);
    
    /* clear unmapped_ids stack */
    while (!Stack_empty(memory->unmappedIDs)) {
        free(Stack_pop(memory->unmappedIDs));
    }
    Stack_free(&memory->unmappedIDs);
    
    free(memory);
}











