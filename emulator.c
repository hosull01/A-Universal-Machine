#include "emulator.h"
#include "memory.h"
#include "bitpack.h"
#include "unistd.h"

#define OPCODE_LSB 28
#define OPCODE_WIDTH 4 
#define RA_LSB 6
#define RA_LSB_onereg 25
#define RB_LSB 3
#define RC_LSB 0
#define REG_WIDTH 3
#define VAL_LSB 0
#define VAL_WIDTH 25
#define REG_SIZE 4294967296 /* 2 ^ 32 */

int decode_inst(uint32_t instruction, uint32_t* pc, uint32_t* ps);
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

	uint32_t prog_ID = 0;
	uint32_t prog_size = memory_seglength(realMem, prog_ID);
	uint32_t prog_counter = 0;
	uint32_t um_inst;

	while (prog_counter < prog_size) {
        
		um_inst = mem_read(realMem, prog_ID, prog_counter);
		decode_inst(um_inst, &prog_counter, &prog_size);
		prog_counter++;
	}

}


int decode_inst(uint32_t instruction, uint32_t* pc, uint32_t* ps)
{

	uint32_t opcode = Bitpack_getu(instruction, OPCODE_WIDTH, OPCODE_LSB);
	uint32_t ra, rb, rc;	/* indices to regs[] */
	uint32_t val;
	int success = 1;
	int failed = 0;

	switch (opcode) {
		case 0: /* conditional move */
			ra = Bitpack_getu(instruction, REG_WIDTH, RA_LSB);
			rb = Bitpack_getu(instruction, REG_WIDTH, RB_LSB);
			rc = Bitpack_getu(instruction, REG_WIDTH, RC_LSB);

			if (regs[rc] != 0)
				regs[ra] = regs[rb];

			return success;

		case 1: /* segmented load */
			ra = Bitpack_getu(instruction, REG_WIDTH, RA_LSB);
			rb = Bitpack_getu(instruction, REG_WIDTH, RB_LSB);
			rc = Bitpack_getu(instruction, REG_WIDTH, RC_LSB);
			val = mem_read(realMem, regs[rb], regs[rc]);

			regs[ra] = val;
			return success;

		case 2: /* segmented store */
			ra = Bitpack_getu(instruction, REG_WIDTH, RA_LSB);
			rb = Bitpack_getu(instruction, REG_WIDTH, RB_LSB);
			rc = Bitpack_getu(instruction, REG_WIDTH, RC_LSB);
			
			mem_write(realMem, regs[rc], regs[ra], regs[rb]);
			return success;

		case 3: /* addition */ 
			ra = Bitpack_getu(instruction, REG_WIDTH, RA_LSB);
			rb = Bitpack_getu(instruction, REG_WIDTH, RB_LSB);
			rc = Bitpack_getu(instruction, REG_WIDTH, RC_LSB);

			regs[ra] = regs[rb] + regs[rc];
			return success;

		case 4: /* multiplcation */
			ra = Bitpack_getu(instruction, REG_WIDTH, RA_LSB);
			rb = Bitpack_getu(instruction, REG_WIDTH, RB_LSB);
			rc = Bitpack_getu(instruction, REG_WIDTH, RC_LSB);

			regs[ra] = (regs[rb] * regs[rc]) % REG_SIZE;
			return success;

		case 5: /* divison */
			ra = Bitpack_getu(instruction, REG_WIDTH, RA_LSB);
			rb = Bitpack_getu(instruction, REG_WIDTH, RB_LSB);
			rc = Bitpack_getu(instruction, REG_WIDTH, RC_LSB);

			regs[ra] = (regs[rb] / regs[rc]) % REG_SIZE;
			return success;

		case 6: /* NAND */
			ra = Bitpack_getu(instruction, REG_WIDTH, RA_LSB);
			rb = Bitpack_getu(instruction, REG_WIDTH, RB_LSB);
			rc = Bitpack_getu(instruction, REG_WIDTH, RC_LSB);

			regs[ra] = ~(regs[rb] & regs[rc]);
			return success;

		case 7: /* halt */	
            mem_free(realMem);
			exit(0);

		case 8: /* map segment */
			rb = Bitpack_getu(instruction, REG_WIDTH, RB_LSB);
			rc = Bitpack_getu(instruction, REG_WIDTH, RC_LSB);

			regs[rb] = map(realMem, regs[rc]);
			if (regs[rb] == 0) {
				fprintf(stderr, "Emulator out of memory");
				mem_free(realMem);
				exit(1);
			}
			return success;

		case 9: /* unmap segment */
			rc = Bitpack_getu(instruction, REG_WIDTH, RC_LSB);

			unmap(realMem, regs[rc]);
			return success;

		case 10: /* output */
			rc = Bitpack_getu(instruction, REG_WIDTH, RC_LSB);

			char c = regs[rc];
			write(1, &c, 1);
			return success;

		case 11: /* input */ 
			rc = Bitpack_getu(instruction, REG_WIDTH, RC_LSB);
            
			val = read(0, &c, 1);
			if (val == 0)
				regs[rc] = ~0;
			else
				regs[rc] = (uint32_t) c;

			return success;

		case 12: /* load program */ 
			rb = Bitpack_getu(instruction, REG_WIDTH, RB_LSB);
			rc = Bitpack_getu(instruction, REG_WIDTH, RC_LSB);
            
            if (regs[rb] != 0) {
                val = duplicate(realMem, regs[rb], 0);
                *ps = val;
            }
            
			*pc = regs[rc] - 1;
			return success;

		case 13: /* load value */
			ra = Bitpack_getu(instruction, REG_WIDTH,
				RA_LSB_onereg);
			val = Bitpack_getu(instruction, VAL_WIDTH, VAL_LSB);

			regs[ra] = val;
			return success;
	}

	return failed;	/* invalid opcode */
}
