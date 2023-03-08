#ifndef SPL_DEBUG_H
#define SPL_DEBUG_H

#include "spl_chunk.h"

void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);

#endif