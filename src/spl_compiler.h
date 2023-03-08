#ifndef SPL_COMPILER_H
#define SPL_COMPILER_H

#include "spl_object.h"
#include "spl_vm.h"


bool compile(const char* source, Chunk* chunk);


#endif