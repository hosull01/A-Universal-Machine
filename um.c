#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "load_program.h"
//#include "seq.h"
#include "segment.h"
#include "emulator.h"

int main(int argc, char *argv[])
{

	(void)argc;
    
    /* open UM program */
    Segment program;
    program = open_prog(argv[1]);

    /* run um program */
    run_emulator(program);

	return 0;
}