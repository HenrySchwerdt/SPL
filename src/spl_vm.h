#ifndef SPL_VM_H
#define SPL_VM_H

#include "spl_chunk.h"
#include "spl_table.h"
#include "spl_value.h"

#define STACK_MAX 65535 // 4 * Bytes

typedef struct {
    Chunk* chunk;
    uint8_t * ip;
    Value stack[STACK_MAX];
    Value* stackTop;
    Table globals;
    Table strings;
	Obj* objects;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(const char* chunk);
void push(Value value);
Value pop();

#endif