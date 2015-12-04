#include "emulator.h"
//#include "memory.h"
#include "unistd.h"
#include "assert.h"
#include "umstack.h"

#define OPCODE_LSB 28
#define OPCODE_WIDTH 4
#define RA_SHIFTLEFT 23
#define RB_SHIFTLEFT 26
#define RC_SHIFTLEFT 29
#define R_SHIFTRIGHT 29
#define VAL_SHIFTLEFT 7
#define VAL_SHIFTRIGHT 7
#define REG_SIZE 4294967296 /* 2 ^ 32 */


int decode_inst(uint32_t instruction, uint32_t* pc, uint32_t* ps, 
		Segment* prog);
void execute_prog(uint32_t start, uint32_t end);
static inline uint32_t map(uint32_t size);
static inline void unmap(uint32_t segID);
static inline uint32_t duplicate(uint32_t toDup_ID, uint32_t toRep_ID);
static inline void mem_free();


enum regs { r0 = 0, r1, r2, r3, r4, r5, r6, r7 };
static uint32_t regs[8] = { 0 };
//Memory_T realMem;

/* memory variables */
#define CAPACITY 250000000
Segment* realMem;
umStack unmappedIDs;
uint32_t memUsed;
uint32_t memCap;
uint32_t segCount;

/* cache variables */
 // uint32_t recent_id1, recent_id2;
 // Segment recent_seg1, recent_seg2;


/* 
 * function for running UM program and loops though words in $m[0]
 * calls a function to execute the word
 */
 void run_emulator(Segment UMprogram)
{
	/* setup um */
	memCap = 31250000;
	realMem = calloc(memCap, sizeof(Segment));
	*(realMem) = UMprogram;
	memUsed = *(UMprogram);
	segCount = 1;
	unmappedIDs = Stack_new();
 
	// recent_id1 = 0;
	// recent_seg1 = UMprogram;
	// recent_id2 = -1;

	uint32_t prog_size = *(UMprogram);
	uint32_t prog_counter = 0;
	uint32_t um_inst;

	/* run um */
	while (prog_counter < prog_size) {
		um_inst = *(*(realMem) + (prog_counter + 1));
		decode_inst(um_inst, &prog_counter, &prog_size, &UMprogram);
		prog_counter++;
	}

}


int decode_inst(uint32_t instruction, uint32_t* pc, uint32_t* ps, Segment* prog)
{

	uint32_t val;
	char c;
	//int success = 1;
	int failed = 0;

	Segment curr_segment;

	/* extract opcode and indicies to regs[] */ 
	uint32_t opcode = instruction >> OPCODE_LSB;
	uint32_t ra = (instruction << RA_SHIFTLEFT) >> R_SHIFTRIGHT;
	uint32_t rb = (instruction << RB_SHIFTLEFT) >> R_SHIFTRIGHT;
	uint32_t rc = (instruction << RC_SHIFTLEFT) >> R_SHIFTRIGHT;


	switch (opcode) {
		case 0: /* conditional move */
			if (regs[rc] != 0)
				regs[ra] = regs[rb];

			break;

		case 1: /* segmented load */

			// if (recent_id1 == regs[rb]) {
			// 	regs[ra] = *(recent_seg1 + (regs[rc] + 1));
			// 	break;
			// } 

			// if (recent_id2 == regs[rb]) {
			// 	regs[ra] = *(recent_seg2 + (regs[rc] + 1));
			// 	break;
			// }

			// if (!(regs[rb] & 1)) {
   //      		recent_id1 = regs[rb];
   //      		recent_seg1 = *(realMem + regs[rb]);
   //      		regs[ra] = *(recent_seg1 + (regs[rc] + 1));
   //   		} else { 
   //      		recent_id2 = regs[rb];
   //      		recent_seg2 = *(realMem + regs[rb]);
   //      		regs[ra] = *(recent_seg2 + (regs[rc] + 1));
   //   		}
			curr_segment = *(realMem + regs[rb]);
			regs[ra] = *(curr_segment + (regs[rc] + 1));
			break;

		case 2: /* segmented store */

			// if (recent_id1 == regs[ra]) {
			// 	*(recent_seg1 + (regs[rb] + 1)) = regs[rc];
			// 	break;
			// } 

			// if (recent_id2 == regs[ra]) {
			// 	*(recent_seg2 + (regs[rb] + 1)) = regs[rc];
			// 	break;
			// }

			// if (!(regs[ra] & 1)) {
   //      		recent_id1 = regs[ra];
   //      		recent_seg1 = *(realMem + regs[ra]);
   //      		*(recent_seg1 + (regs[rb] + 1)) = regs[rc];
   //   		} else { 
   //      		recent_id2 = regs[ra];
   //      		recent_seg2 = *(realMem + regs[ra]);
   //      		*(recent_seg2 + (regs[rb] + 1)) = regs[rc];
   //   		}
			curr_segment = *(realMem + regs[ra]);
			*(curr_segment + (regs[rb] + 1)) = regs[rc];
			break;

		case 3: /* addition */ 
			regs[ra] = regs[rb] + regs[rc];
			break;

		case 4: /* multiplcation */
			regs[ra] = (regs[rb] * regs[rc]) % REG_SIZE;
			break;

		case 5: /* divison */
			regs[ra] = (regs[rb] / regs[rc]) % REG_SIZE;
			break;

		case 6: /* NAND */
			regs[ra] = ~(regs[rb] & regs[rc]);
			break;

		case 7: /* halt */	
            mem_free();
			exit(0);

		case 8: /* map segment */
			regs[rb] = map(regs[rc]);
			if (regs[rb] == 0) {
				fprintf(stderr, "Emulator out of memory");
				mem_free();
				exit(1);
			}
			break;

		case 9: /* unmap segment */
			unmap(regs[rc]);
			break;

		case 10: /* output */
			c = regs[rc];
			write(1, &c, 1);
			break;

		case 11: /* input */ 
			val = read(0, &c, 1);
			if (val == 0)
				regs[rc] = ~0;
			else
				regs[rc] = (uint32_t) c;

			break;

		case 12: /* load program */ 
            if (regs[rb] != 0) {
            	//fprintf(stderr, "changing program by duplicating --- ");
                val = duplicate(regs[rb], 0);
                *ps = val;
                //fprintf(stderr, "ps is %u  pc is %u\n", val, regs[rc] - 1);
            }
            assert(prog);

			*pc = regs[rc] - 1;
			break;

		case 13: /* load value */
			ra = (instruction << OPCODE_WIDTH) >> R_SHIFTRIGHT;
			regs[ra] = (instruction << VAL_SHIFTLEFT) >> VAL_SHIFTRIGHT;

			//regs[ra] = val;
			break;
	}

	return failed;	/* invalid opcode */
}

static inline uint32_t map(uint32_t size)
{
	uint32_t new_segID = 0;
	size++;
	if ((memUsed + size) <= CAPACITY) {\
		/* create new segment */
		Segment new_seg = calloc(size, sizeof(uint32_t));
		*(new_seg) = size;

		if (Stack_empty(unmappedIDs)) {
			if (segCount == memCap) {
				memCap = memCap * 2;
				realMem = realloc(realMem, sizeof(Segment) * memCap);
			}
			new_segID = segCount;
		} else {
			//fprintf(stderr, "popping\n");
			new_segID = Stack_pop(unmappedIDs);
		}

		*(realMem + new_segID) = new_seg;
		segCount++;
		memUsed += size;
	}
	//fprintf(stderr, "this is the segID map rets %u\n", new_segID);
	return new_segID;
}

static inline void unmap(uint32_t segID)
{
	Segment unmapped_seg = *(realMem + segID);
	memUsed -= *(unmapped_seg);
	free(unmapped_seg);
	segCount--;

	/* update cache ids */
    // if (recent_id1 == segID) {
    //     recent_id1 = -1;
    // }
    // if (recent_id2 == segID) {
    //     recent_id2 = -1;
    // }

	Stack_push(unmappedIDs, segID);
}

static inline uint32_t duplicate(uint32_t toDup_ID, uint32_t toRep_ID)
{
	Segment seg_dup = seg_copy(*(realMem + toDup_ID));

	Segment seg_rep = *(realMem + toRep_ID);
	*(realMem + toRep_ID) = seg_dup;

	memUsed += *(seg_dup) - *(seg_rep);

	seg_delete(seg_rep);

	return *(seg_dup);

}

static inline void mem_free()
{

}