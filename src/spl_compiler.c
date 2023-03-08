#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spl_common.h"
#include "spl_compiler.h"
#include "spl_lexer.h"
#include "spl_utils.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"

typedef struct {
    spl_token current;
    spl_token previous;
    bool hadError;
    bool panicMode;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR, // or
    PREC_AND, // and
    PREC_EQUALITY, // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM, // + -
    PREC_FACTOR, // * /
    PREC_UNARY, // ! -
    PREC_CALL, // . ()
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool canAssign);


typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

typedef struct {
    spl_token name;
    int depth;
    bool final;
} Local;

typedef struct {
    Local locals[UINT8_COUNT];
    int localCount;
    int scopeDepth;
} Compiler;


Parser parser;

Compiler* current = NULL;

Chunk* compilingChunk;

static Chunk* currentChunk() {
    return compilingChunk;
}

// TODO needs refactoring
static void errorAt(spl_token* token, const char* message) {

    if (parser.panicMode) return;

    int lineNr = token->line;
    int length = 0;
    
    parser.panicMode = true;
    fprintf(stderr, "%s[line %d] Error", ANSI_COLOR_RED, lineNr);
    if (token->type == TK_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TK_ERROR) {
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }
    fprintf(stderr, ": %s\n%s", message, ANSI_COLOR_RESET);
    // print line before 
    
    fprintf(stderr,"^\n%s", ANSI_COLOR_RESET);
    parser.hadError = true;
}

static void error(const char* message) {
    errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

static void advance() {
    parser.previous = parser.current;
    for(;;) {
        parser.current = next_token();
        if (parser.current.type != TK_ERROR) break;
        errorAtCurrent(parser.current.start);
    }

}

static void consume(spl_token_type type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }
    errorAtCurrent(message);
}

static bool check(spl_token_type type) {
    return parser.current.type == type;
}

static bool match(spl_token_type type) {
    if (!check(type)) return false;
    advance();
    return true;
}

static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
	emitByte(byte1);
	emitByte(byte2);
}

static void emitLoop(int loopStart) {
    emitByte(OP_LOOP);

    int offset = currentChunk()->count - loopStart + 2;
    if (offset > UINT16_MAX) error("Loop body too large.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

static int emitJump(uint8_t instruction) {
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk()->count - 2;
}

static void emitReturn() {
    emitByte(OP_RETURN);
}

static int emitConstant(Value value) {
    int index = writeConstant(currentChunk(), value, parser.previous.line);
    if(index == -1){
        error("Too many constants in one chunk");
    }
    return index;
}

static void patchJump(int offset) {
    int jump = currentChunk()->count - offset - 2;
    if (jump > UINT16_MAX) {
        error("Too much code to jump over.");
    }
    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;
}

static void initCompiler(Compiler* compiler) {
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    current = compiler;
}

static void endCompiler() {
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if(!parser.hadError) {
        disassembleChunk(currentChunk(), "code");
    }
#endif
}

static void beginScope() {
    current->scopeDepth++;
}

static void endScope() {
    current->scopeDepth--;
    while(current->localCount > 0 &&
            current->locals[current->localCount -1].depth >
                current->scopeDepth) {
        emitByte(OP_POP);
        current->localCount--;
    } 
}


static void expression();
static void statement();
static void declaration();
static ParseRule* getRule(spl_token_type type);
static void parsePrecedence(Precedence precedence);

static uint32_t identifierConstant(spl_token* name) {
    int constant = addConstant(currentChunk(), OBJ_VAL(copyString(name->start, name->length)));
    return (u_int32_t) constant;
}

static bool identifierEqual(spl_token* a, spl_token* b) {
    if (a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Compiler* compiler, spl_token* name) {
    for (int i = compiler->localCount -1; i >= 0; i--) {
        Local* local = &compiler->locals[i];
        if (identifierEqual(name, &local->name)) {
            if (local->depth == -1) {
                error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }
    return -1;
}

static void addLocal(spl_token name, bool isFinal) {
    if (current->localCount == UINT8_COUNT) {
        error("Too many local variables in function.");
    }
    Local* local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
    local->final = isFinal;
    local->depth = current->scopeDepth;
}

static void declareVariable(bool isFinal) {

    if (current->scopeDepth == 0) return;
    spl_token* name = &parser.previous;
    for (int i = current->localCount -1; i >= 0; i--) {
        Local* local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scopeDepth) {
            break;
        }
        if (identifierEqual(name, &local->name)) {
            error("Already variable with this name in this scope");
        }
    }
    addLocal(*name, isFinal);
}

static uint32_t parseVariable(const char* errorMessage) {
    bool isFinal = false;
    consume(TK_IDENTIFIER, errorMessage);
    declareVariable(isFinal);
    if (current->scopeDepth > 0) return 0;
    return identifierConstant(&parser.previous);
}

static void markInitialized() {
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void defineVariable(uint32_t global) {
    if (current->scopeDepth > 0) {
        markInitialized();
        return;
    }
    if (global <= UINT8_MAX) {
        emitBytes(OP_DEFINE_GLOBAL, (uint8_t) global);
    } else if (global <= UINT32_MAX) {
        uint8_t largeConstant[CONSTANT_LONG_BYTE_SIZE];
        CONVERT_TO_BYTE_ARRAY(largeConstant, CONSTANT_LONG_BYTE_SIZE, global);
        emitByte(OP_DEFINE_GLOBAL_LONG);
        for (int i = 0; i < CONSTANT_LONG_BYTE_SIZE; i++) {
            emitByte(largeConstant[i]);
        }
    } else {
        error("Too many constants in one chunk");
    }
}

static void and_(bool canAssign) {
    int endJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    parsePrecedence(PREC_AND);
    patchJump(endJump);
}

static void number(bool canAssign) {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

static void or_(bool canAssign) {
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);
    patchJump(elseJump);
    emitByte(OP_POP);

    parsePrecedence(PREC_OR);
    patchJump(endJump);
}

static void string(bool canAssign) {
	emitConstant(OBJ_VAL(copyString(parser.previous.start + 1,
						parser.previous.length - 2)));
}

static void namedVariable(spl_token name, bool canAssign) {
    uint8_t getOp, setOp;
    int arg = resolveLocal(current, &name);
    if (arg != -1) {
        getOp = arg <= UINT8_MAX ? OP_GET_LOCAL : OP_GET_LOCAL_LONG;
        setOp = arg <= UINT8_MAX ? OP_SET_LOCAL : OP_SET_LOCAL_LONG;
    } else {
        arg = identifierConstant(&name);
        getOp = arg <= UINT8_MAX ? OP_GET_GLOBAL : OP_GET_GLOBAL_LONG;
        setOp = arg <= UINT8_MAX ? OP_SET_GLOBAL : OP_SET_GLOBAL_LONG;
    }

    if (getOp == OP_GET_LOCAL_LONG || getOp == OP_GET_GLOBAL_LONG) {
        uint8_t largeConstant[CONSTANT_LONG_BYTE_SIZE];
        CONVERT_TO_BYTE_ARRAY(largeConstant, CONSTANT_LONG_BYTE_SIZE, arg);
        if (match(TK_EQUAL) && canAssign) {
            Local* local = &current->locals[arg];
            if (local->final) {
                error("Can't reassign final variable");
            }
            expression();
            emitByte(setOp);
        } else {
            emitByte(getOp);
        }
        // Adds 4 bytes of memory to the chunk
        for (int i = 0; i < CONSTANT_LONG_BYTE_SIZE; i++) {
                emitByte(largeConstant[i]);
        }
    } else {
        if (match(TK_EQUAL) && canAssign) {
            Local* local = &current->locals[arg];
            if (local->final) {
                error("Can't reassign final variable");
            }
            expression();
            emitBytes(setOp, arg);
        } else {
             emitBytes(getOp, (uint8_t) arg);
        }
    }
}

static void variable(bool canAssign) {
    namedVariable(parser.previous, canAssign);
}

static void unary(bool canAssign) {
    spl_token_type operatorType = parser.previous.type;
    // compile the operand
    parsePrecedence(PREC_UNARY);
    // Emit the operator instruction
    switch (operatorType)
    {
		case TK_BANG: emitByte(OP_NOT); break;
		case TK_MINUS: emitByte(OP_NEGATE); break;
        default:
            return; // Unreachable
    }

}



static void binary(bool canAssign) {
    // Remember the operator
    spl_token_type operatorType = parser.previous.type;
    // previous: +, current: 1

    // Compile the rig`
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    // Emit the operator instruction
    switch (operatorType)
    {
		case TK_EQUAL_EQUAL: emitByte(OP_EQUAL); break;
		case TK_GREATER: emitByte(OP_GREATER); break;
		case TK_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
		case TK_LESS: emitByte(OP_LESS); break;
		case TK_LESS_EQUAL: emitBytes(OP_GREATER, OP_NOT); break;
        case TK_PLUS: emitByte(OP_ADD); break;
        case TK_MINUS: emitByte(OP_SUBTRACT); break;
        case TK_STAR: emitByte(OP_MULTIPLY); break;
        case TK_SLASH: emitByte(OP_DIVIDE); break;
        default:
            return; // Unreachable
    }
}

static void literal(bool canAssign) {
	switch(parser.previous.type) {
		case TK_FALSE: emitByte(OP_FALSE); break;
		case TK_NULL: emitByte(OP_NIL); break;
		case TK_TRUE: emitByte(OP_TRUE); break;
		default:
			break; // Unreachable
	}

}

static void grouping(bool canAssign) {
    expression();
    consume(TK_RIGHT_PAREN, "Expect ')' after expression");
}

ParseRule rules[] = {
    [TK_LEFT_PAREN] = {grouping, NULL,PREC_NONE},
    [TK_RIGHT_PAREN]= {NULL,NULL,PREC_NONE},
    [TK_LEFT_BRACE]= {NULL,NULL,PREC_NONE},
    [TK_RIGHT_BRACE]= {NULL,NULL,PREC_NONE},
    [TK_DOT]= {NULL,NULL,PREC_NONE},
    [TK_MINUS]= {unary,binary, PREC_TERM},
    [TK_PLUS]= {NULL,binary, PREC_TERM},
    [TK_SEMICOLON]= {NULL,NULL, PREC_NONE},
    [TK_SLASH]= {NULL,binary, PREC_FACTOR},
    [TK_STAR]= {NULL,binary, PREC_FACTOR},
    [TK_BANG]= {unary,NULL,PREC_NONE},
    [TK_EQUAL]= {NULL,NULL,PREC_NONE},
    [TK_EQUAL_EQUAL]= {NULL,binary,PREC_EQUALITY},
    [TK_GREATER]= {NULL,binary,PREC_COMPARISON},
    [TK_GREATER_EQUAL] = {NULL,binary,PREC_COMPARISON},
    [TK_LESS]= {NULL,binary,PREC_COMPARISON},
    [TK_LESS_EQUAL]= {NULL,binary,PREC_COMPARISON},
    [TK_IDENTIFIER]= {variable,NULL,PREC_NONE},
    [TK_STRING_VAL]= {string,NULL,PREC_NONE},
    [TK_NUMBER_VAL]= {number,NULL,PREC_NONE},
    [TK_AND]= {NULL,and_,PREC_AND},
    [TK_ELSE]= {NULL,NULL,PREC_NONE},
    [TK_FALSE]= {literal,NULL,PREC_NONE},
    [TK_IF]= {NULL,NULL,PREC_NONE},
    [TK_NULL]= {literal,NULL,PREC_NONE},
    [TK_OR]= {NULL,or_,PREC_OR},
    [TK_PRINT]= {NULL,NULL,PREC_NONE},
    [TK_TRUE]= {literal,NULL,PREC_NONE},
    [TK_VAR]= {NULL,NULL,PREC_NONE},
    [TK_WHILE]= {NULL,NULL,PREC_NONE},
    [TK_ERROR]= {NULL,NULL,PREC_NONE},
    [TK_EOF]= {NULL,NULL,PREC_NONE},
};



static void parsePrecedence(Precedence precedence) {
    advance(); 
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL) {
        error("Expect expression.");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);
    while(precedence <= getRule(parser.current.type)->precedence) {
        advance(); 
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }
    if (canAssign && match(TK_EQUAL)) {
        error("Invalid assignment target.");
    }
}

static ParseRule* getRule(spl_token_type type) {
    return &rules[type];
}



static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

static void block() {
    while(!check(TK_RIGHT_BRACE) && !check(TK_EOF)) {
        declaration();
    }
    consume(TK_RIGHT_BRACE, "Expect '}' after block.");
}

static void varDeclaration() {
    uint32_t global = parseVariable("Expect variable name.");

    if (match(TK_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NIL);
    }
    consume(TK_SEMICOLON, "Expect ';' after variable declaration.");
    defineVariable(global);
}

static void expressionStatement() {
    expression();
    consume(TK_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

static void printStatement() {
    expression();
    consume(TK_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

static void whileStatement() {
    int loopStart = currentChunk()->count;
    consume(TK_LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TK_RIGHT_PAREN, "Expect ')' after condition.");

    int exitJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    statement();

    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OP_POP);
}

static void ifStatement() {
    consume(TK_LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TK_RIGHT_PAREN, "Expect ')' after condition.");
    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();
    int elseJump = emitJump(OP_JUMP);
    patchJump(thenJump);
    emitByte(OP_POP);
    if (match(TK_ELSE)) statement();
    patchJump(elseJump);
}

static void synchronize() {
    parser.panicMode = false;
    while(parser.current.type != TK_EOF) {
        if (parser.previous.type == TK_SEMICOLON) return;

        switch (parser.current.type) {

            case TK_VAR:
            case TK_IF:
            case TK_WHILE:
            case TK_PRINT:
                return;
            default:
                // do nothing
                ;
        }
        advance();
      
    }
}

static void declaration() {
    if (match(TK_VAR)) {
        varDeclaration();
    } else {
        statement();
    }
    if (parser.panicMode) synchronize();
}

static void statement() {
    if (match(TK_PRINT)) {
        printStatement();
    } else if (match(TK_IF)) {
        ifStatement();
    } else if (match(TK_WHILE)) {
        whileStatement();
    } else if (match(TK_LEFT_BRACE)) {
        beginScope();
        block();
        endScope();
    } else {
        expressionStatement();
    }
}

bool compile(const char* source, Chunk* chunk) {
    spl_lex_init(source);
    Compiler compiler;
    initCompiler(&compiler);
    compilingChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;
    advance();
    while(!match(TK_EOF)) {
        declaration();
    }
    endCompiler();
    return !parser.hadError;
}