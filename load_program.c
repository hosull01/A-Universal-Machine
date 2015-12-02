#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "except.h"
#include "mem.h"
#include "segment.h"
#include "load_program.h"

/* we estimate that an average program consists of roughly 10 new_insts */ 
#define HINT 10



Except_T Open_Failed = {"Failed to open file"};
Except_T Empty_File = {"Error: UM file is empty"};

static inline uint32_t program_size (char *filename);
static inline uint32_t reverse_endian (uint32_t word);


/*
 * input: an open file to read from
 */
inline Segment open_prog(char *filename)
{
    FILE *file;
    uint32_t numElem = program_size(filename);
    Segment program = seg_create(numElem);
    
    file = fopen(filename, "rb");
    if (file == NULL) {
        RAISE(Open_Failed);
    }

    uint32_t prog_iter = 0;
    uint32_t inst;
    uint32_t* new_instptr = malloc(sizeof(inst));

    /* read one elements that is at most 4 bytes */
    while (fread(new_instptr, 4, 1, file) > 0) {
             inst = reverse_endian(* new_instptr);
             seg_put(program, inst, prog_iter);
             prog_iter++;
    }
    free(new_instptr);
    fclose(file);
    
    return program;
}

/* determines the number of instructions in the program file to be read */
static inline uint32_t program_size (char *filename)
{
    struct stat buffer;
    int status;
    uint32_t numElem;
    
    status = stat(filename, &buffer);
    
    if (status != 0) {
        RAISE(Open_Failed);
    }
    
    numElem = buffer.st_size;
    if (numElem == 0) {
        RAISE(Empty_File);
    }
    
    if (numElem%4 == 0){
        numElem = numElem/4;
    } else {
        numElem = (numElem/4) + 1;
    }
    return numElem;
}



/* reverses endianess of word, big to little or little to bit */
static inline uint32_t reverse_endian (uint32_t word)
{
    uint32_t b0, b1, b2, b3;

    b0 = (word & 0x000000ff) << 24u;
    b1 = (word & 0x0000ff00) << 8u;
    b2 = (word & 0x00ff0000) >> 8u;
    b3 = (word & 0xff000000) >> 24u;

    return b0 | b1 | b2 | b3;
}