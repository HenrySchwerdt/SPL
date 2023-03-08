#include <stdlib.h>
#include "spl_chunk.h"
#include "spl_memory.h"

void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    initLineArray(&chunk->lines);
    initValueArray(&chunk->constants);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    // if chunk full free memory
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    writeLineArray(&chunk->lines, line);

    chunk->count++;
}

void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    freeLineArray(&chunk->lines);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}


bool writeConstant(Chunk* chunk, Value value, int line) {
    uint32_t index = (uint32_t) addConstant(chunk, value);
    if (index > UINT32_MAX) {
        return false;
    }
    if (index > UINT8_MAX) {
        uint8_t largeConstant[CONSTANT_LONG_BYTE_SIZE];
        CONVERT_TO_BYTE_ARRAY(largeConstant, CONSTANT_LONG_BYTE_SIZE, index);
        writeChunk(chunk, OP_CONSTANT_LONG, line);

        for (int i = 0; i < CONSTANT_LONG_BYTE_SIZE; i++) {
            writeChunk(chunk, largeConstant[i], line);
        }
    } else {
        writeChunk(chunk, OP_CONSTANT, line);
        writeChunk(chunk, index, line);
    }
    return true;
}

