#ifndef SPL_CHUNK_H
#define SPL_CHUNK_H

#include "spl_common.h"
#include "spl_utils.h"
#include "spl_value.h"
#include "spl_line_tracer.h"

#define CONSTANT_LONG_BYTE_SIZE 4

typedef enum {
    OP_CONSTANT_LONG,
    OP_CONSTANT,
	OP_NIL,
	OP_TRUE,
	OP_FALSE,
    OP_POP,
    OP_GET_GLOBAL,
    OP_GET_GLOBAL_LONG,
    OP_GET_LOCAL,
    OP_GET_LOCAL_LONG,
    OP_DEFINE_GLOBAL,
    OP_DEFINE_GLOBAL_LONG,
    OP_SET_GLOBAL,
    OP_SET_GLOBAL_LONG,
    OP_SET_LOCAL,
    OP_SET_LOCAL_LONG,
	OP_EQUAL,
	OP_GREATER,
	OP_LESS,
	OP_NOT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_PRINT,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_LOOP,
    OP_RETURN,
} OpCode;

typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    LineArray lines;
    ValueArray constants;
} Chunk;


void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
bool writeConstant(Chunk* chunk, Value value, int line);
int addConstant(Chunk* chunk, Value value);


#endif