#ifndef SPL_LINE_TRACER_H
#define SPL_LINE_TRACER_H

#include "spl_common.h"

#define STORAGE_LENGTH 2 

typedef struct {
    int capacity;
    int count;
    int* values;
} LineArray;

void initLineArray(LineArray* array);
void writeLineArray(LineArray* array, int value);
void freeLineArray(LineArray* array);
int getLine(LineArray* array, int index);

#endif