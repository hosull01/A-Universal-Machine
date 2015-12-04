#include "emulator.h"
#include "unistd.h"
#include "assert.h"

#define OPCODE_LSB 28
#define OPCODE_WIDTH 4
#define RA_SL 23
#define RB_SL 26
#define RC_SL 29
#define R_SR 29
#define VAL_SL 7
#define VAL_SR 7
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

/* memory variables */
#define CAPACITY 250000000
Segment* realMem;
uint32_t memUsed;
uint32_t memCap;
uint32_t segCount;

/* cache variables */
uint32_t recent_id1, recent_id2;
Segment recent_seg1, recent_seg2;
Segment program_seg;

/* umStack variables */
uint32_t* umStack;
uint32_t stkCount;
uint32_t stkCap;


/* 
 * function for running UM program and loops though words in $m[0]
 * calls a function to execute the word
 */
 void run_emulator(Segment UMprogram)
{
	/* setup um memory */
	memCap = 31250000;
	realMem = calloc(memCap, sizeof(Segment));
	*(realMem) = UMprogram;
	memUsed = *(UMprogram);
	segCount = 1;
    
    /* setup um segID stack */
    stkCap = 1000000;
    umStack = calloc(stkCap, sizeof(uint32_t));
    stkCount = 0;
	
    /* setup um cache */
	recent_id1 = 0;
	recent_seg1 = UMprogram;
	recent_id2 = -1;
	program_seg = UMprogram;

    /* setup um control loop */
	uint32_t prog_size = *(UMprogram);
	uint32_t prog_counter = 0;
	uint32_t um_inst;

	/* run um */
	while (prog_counter < prog_size) {
		um_inst = *(*(realMem) + (prog_counter + 1));

		/* decode instruction */

		decode_inst(um_inst, &prog_counter, &prog_size, &UMprogram);
		prog_counter++;
	}

}


int decode_inst(uint32_t instruction, uint32_t* pc, uint32_t* ps, Segment* prog)
{
	uint32_t val;
	char c;

	/* extract opcode and indicies to regs[] */ 
	uint32_t opcode = instruction >> OPCODE_LSB;
	uint32_t ra;
	uint32_t rb;
	uint32_t rc;


	switch (opcode) {
		case 0: /* conditional move */
			ra = (instruction << RA_SL) >> R_SR;
			rb = (instruction << RB_SL) >> R_SR;
	 		rc = (instruction << RC_SL) >> R_SR;

			if (regs[rc] != 0)
				regs[ra] = regs[rb];

			break;

		case 1: /* segmented load */
			ra = (instruction << RA_SL) >> R_SR;
			rb = (instruction << RB_SL) >> R_SR;
	 		rc = (instruction << RC_SL) >> R_SR;

			if (recent_id1 == regs[rb]) {
				regs[ra] = *(recent_seg1 + (regs[rc] + 1));
				break;
			} 

			if (recent_id2 == regs[rb]) {
				regs[ra] = *(recent_seg2 + (regs[rc] + 1));
				break;
			}

			if (!(regs[rb] & 1)) {
        		recent_id1 = regs[rb];
        		recent_seg1 = *(realMem + regs[rb]);
        		regs[ra] = *(recent_seg1 + (regs[rc] + 1));
     		} else { 
        		recent_id2 = regs[rb];
        		recent_seg2 = *(realMem + regs[rb]);
        		regs[ra] = *(recent_seg2 + (regs[rc] + 1));
     		}

			break;

		case 2: /* segmented store */
			ra = (instruction << RA_SL) >> R_SR;
			rb = (instruction << RB_SL) >> R_SR;
	 		rc = (instruction << RC_SL) >> R_SR;

			if (recent_id1 == regs[ra]) {
				*(recent_seg1 + (regs[rb] + 1)) = regs[rc];
				break;
			} 

			if (recent_id2 == regs[ra]) {
				*(recent_seg2 + (regs[rb] + 1)) = regs[rc];
				break;
			}

			if (!(regs[ra] & 1)) {
        		recent_id1 = regs[ra];
        		recent_seg1 = *(realMem + regs[ra]);
        		*(recent_seg1 + (regs[rb] + 1)) = regs[rc];
     		} else { 
        		recent_id2 = regs[ra];
        		recent_seg2 = *(realMem + regs[ra]);
        		*(recent_seg2 + (regs[rb] + 1)) = regs[rc];
     		}
			// curr_segment = *(realMem + regs[ra]);
			// *(curr_segment + (regs[rb] + 1)) = regs[rc];
			break;

		case 3: /* addition */ 
			ra = (instruction << RA_SL) >> R_SR;
			rb = (instruction << RB_SL) >> R_SR;
	 		rc = (instruction << RC_SL) >> R_SR;

			regs[ra] = regs[rb] + regs[rc];
			break;

		case 4: /* multiplcation */
			ra = (instruction << RA_SL) >> R_SR;
			rb = (instruction << RB_SL) >> R_SR;
	 		rc = (instruction << RC_SL) >> R_SR;

			regs[ra] = (regs[rb] * regs[rc]) % REG_SIZE;
			break;

		case 5: /* divison */
			ra = (instruction << RA_SL) >> R_SR;
			rb = (instruction << RB_SL) >> R_SR;
	 		rc = (instruction << RC_SL) >> R_SR;

			regs[ra] = (regs[rb] / regs[rc]) % REG_SIZE;
			break;

		case 6: /* NAND */
			ra = (instruction << RA_SL) >> R_SR;
			rb = (instruction << RB_SL) >> R_SR;
	 		rc = (instruction << RC_SL) >> R_SR;

			regs[ra] = ~(regs[rb] & regs[rc]);
			break;

		case 7: /* halt */	
            mem_free();
			exit(0);

		case 8: /* map segment */
			rb = (instruction << RB_SL) >> R_SR;
	 		rc = (instruction << RC_SL) >> R_SR;

			regs[rb] = map(regs[rc]);
			if (regs[rb] == 0) {
				fprintf(stderr, "Emulator out of memory");
				mem_free();
				exit(1);
			}
			break;

		case 9: /* unmap segment */
			rc = (instruction << RC_SL) >> R_SR;
			unmap(regs[rc]);
			break;

		case 10: /* output */
			rc = (instruction << RC_SL) >> R_SR;
			c = regs[rc];
			write(1, &c, 1);
			break;

		case 11: /* input */
			rc = (instruction << RC_SL) >> R_SR;

			val = read(0, &c, 1);
			if (val == 0)
				regs[rc] = ~0;
			else
				regs[rc] = (uint32_t) c;

			break;

		case 12: /* load program */
			rb = (instruction << RB_SL) >> R_SR;
			rc = (instruction << RC_SL) >> R_SR; 
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
			ra = (instruction << OPCODE_WIDTH) >> R_SR;
			regs[ra] = (instruction << VAL_SL) >> VAL_SR;

			break;
	}

	return 0;	/* invalid opcode */
}

static inline uint32_t map(uint32_t size)
{
	uint32_t new_segID = 0;
	size++;
	if ((memUsed + size) <= CAPACITY) {\
		/* create new segment */
		Segment new_seg = calloc(size, sizeof(uint32_t));
		*(new_seg) = size;
        
        if (!stkCount) {
            if (segCount == memCap) {
                memCap = memCap * 2;
                realMem = realloc(realMem, sizeof(Segment) * memCap);
            }
            new_segID = segCount;
        } else {
            stkCount--;
            new_segID = *(umStack + stkCount);  /* pop from stack */
        }

		*(realMem + new_segID) = new_seg;
		segCount++;
		memUsed += size;

		if (!(new_segID & 1)) {
        	recent_id1 = new_segID;
        	recent_seg1 = *(realMem + new_segID);
        	
     	} else { 
        	recent_id2 = new_segID;
        	recent_seg2 = *(realMem + new_segID);
        	
     	}
	}
	
	return new_segID;
}

static inline void unmap(uint32_t segID)
{
	Segment unmapped_seg = *(realMem + segID);
	memUsed -= *(unmapped_seg);
	free(unmapped_seg);
	segCount--;

	/* update cache ids */
    if (recent_id1 == segID) {
        recent_id1 = -1;
    }
    if (recent_id2 == segID) {
        recent_id2 = -1;
    }
    
    if (stkCount == stkCap) {
        stkCap += 1000000;
        umStack = realloc(umStack, sizeof(uint32_t) * stkCap);
    }
    
    *(umStack + stkCount) = segID;
    stkCount++;
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