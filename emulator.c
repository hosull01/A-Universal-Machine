#include "emulator.h"
#include "memory.h"
#include "unistd.h"
#include "assert.h"

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


enum regs { r0 = 0, r1, r2, r3, r4, r5, r6, r7 };
static uint32_t regs[8] = { 0 };
Memory_T realMem;


/* 
 * function for running UM program and loops though words in $m[0]
 * calls a function to execute the word
 */
 void run_emulator(Segment UMprogram)
{
	realMem = new_Memory(UMprogram);

	//uint32_t prog_ID = 0;
	uint32_t prog_size = *(UMprogram);
	uint32_t prog_counter = 0;
	uint32_t um_inst;

	while (prog_counter < prog_size) {
		//fprintf(stderr, "(%u)", prog_counter);

		//um_inst = seg_get(UMprogram, prog_counter);
		um_inst = *(UMprogram + (prog_counter + 1));


		decode_inst(um_inst, &prog_counter, &prog_size, &UMprogram);
		prog_counter++;
	}

}


int decode_inst(uint32_t instruction, uint32_t* pc, uint32_t* ps, Segment* prog)
{

	uint32_t val;
	char c;
	int success = 1;
	int failed = 0;

	/* extract opcode and indicies to regs[] */ 
	uint32_t opcode = instruction >> OPCODE_LSB;
	uint32_t ra = (instruction << RA_SHIFTLEFT) >> R_SHIFTRIGHT;
	uint32_t rb = (instruction << RB_SHIFTLEFT) >> R_SHIFTRIGHT;
	uint32_t rc = (instruction << RC_SHIFTLEFT) >> R_SHIFTRIGHT;


	switch (opcode) {
		case 0: /* conditional move */
			if (regs[rc] != 0)
				regs[ra] = regs[rb];

			return success;

		case 1: /* segmented load */
			//fprintf(stderr, "mem_read called\n");
			val = mem_read(realMem, regs[rb], regs[rc]);

			regs[ra] = val;
			return success;

		case 2: /* segmented store */
		//fprintf(stderr, "mem_write called");	
			mem_write(realMem, regs[rc], regs[ra], regs[rb]);
		//	fprintf(stderr, "mem_write returned: rc %u ra %u rb %u\n", regs[rc], regs[ra], regs[rb]);
			return success;

		case 3: /* addition */ 
			regs[ra] = regs[rb] + regs[rc];
			return success;

		case 4: /* multiplcation */
			regs[ra] = (regs[rb] * regs[rc]) % REG_SIZE;
			return success;

		case 5: /* divison */
			regs[ra] = (regs[rb] / regs[rc]) % REG_SIZE;
			return success;

		case 6: /* NAND */
			regs[ra] = ~(regs[rb] & regs[rc]);
			return success;

		case 7: /* halt */	
            mem_free(realMem);
			exit(0);

		case 8: /* map segment */
			regs[rb] = map(realMem, regs[rc]);
			if (regs[rb] == 0) {
				fprintf(stderr, "Emulator out of memory");
				mem_free(realMem);
				exit(1);
			}
			return success;

		case 9: /* unmap segment */
			unmap(realMem, regs[rc]);
			return success;

		case 10: /* output */
			c = regs[rc];
			write(1, &c, 1);
			return success;

		case 11: /* input */ 
			val = read(0, &c, 1);
			if (val == 0)
				regs[rc] = ~0;
			else
				regs[rc] = (uint32_t) c;

			return success;

		case 12: /* load program */ 
            if (regs[rb] != 0) {
            	//fprintf(stderr, "changing program by duplicating --- ");
                val = duplicate(realMem, regs[rb], 0, prog);
                *ps = val;
                //fprintf(stderr, "ps is %u  pc is %u\n", val, regs[rc] - 1);
            }
            assert(prog);

			*pc = regs[rc] - 1;
			return success;

		case 13: /* load value */
			ra = (instruction << OPCODE_WIDTH) >> R_SHIFTRIGHT;
			val = (instruction << VAL_SHIFTLEFT) >> VAL_SHIFTRIGHT;

			regs[ra] = val;
			return success;
	}

	return failed;	/* invalid opcode */
}
