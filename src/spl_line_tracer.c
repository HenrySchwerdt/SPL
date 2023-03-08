#include <stdio.h>
#include "spl_memory.h"
#include "spl_line_tracer.h"

void initLineArray(LineArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeLineArray(LineArray* array, int value) {
    // storage length = 2
    // [count, line, count, line, count, line,....]
    if(array->capacity < array->count + STORAGE_LENGTH) {
        int oldCapacity = array->capacity;
        array->capacity = oldCapacity + STORAGE_LENGTH;
        array->values = GROW_ARRAY(int, array->values,
                             oldCapacity, array->capacity);
    }
    if (array->count >= STORAGE_LENGTH && 
            array->values[array->count-1] == value) {
        array->values[array->count-2] += 1;
    } else {
        array->values[array->count] = 1;
        array->values[array->count+1] = value;
        array->count += STORAGE_LENGTH;
    }
   
}

void freeLineArray(LineArray* array) {
    FREE_ARRAY(int, array->values, array->capacity);
    initLineArray(array);
}

int getLine(LineArray * array, int index) {
    int c = 0;
    for (int i = 0; i < array->count; i++) {
        if (i % STORAGE_LENGTH == 0) {
            c += array->values[i];
            if (c - 1 >= index) {
                return array->values[i+1];
            }
            
        }
    }
    return -1;
}