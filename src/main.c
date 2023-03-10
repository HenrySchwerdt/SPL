#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spl_common.h"
#include "spl_vm.h"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"

static void repl() {
    char line[1024];
    for (;;) {
        printf("%s> %s", CYN, ANSI_COLOR_RESET);

        if(!fgets(line, sizeof(line), stdin)) {
            printf("Exit Repl");
            printf("\n");
            break;
        }
        interpret(line);
    }
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);
    char* buffer = (char *)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\"\n", path);
        exit(74);
    }
    
    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}

static void runFile(const char* path) {
    char* source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);
    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}



int main(int argc, const char * argv[]) {
    initVM();
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: clox [path]\n");
        exit(64);
    }
    freeVM();
    return 0;
}