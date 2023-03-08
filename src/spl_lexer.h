#ifndef SPL_LEXER_H
#define SPL_LEXER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


typedef enum {
    // Single-character tokens
    TK_LEFT_PAREN, TK_RIGHT_PAREN,                      
    TK_LEFT_BRACE, TK_RIGHT_BRACE,                      
    TK_LEFT_BRACKET, TK_RIGHT_BRACKET,                  
    TK_DOT, TK_BANG,              
    TK_SEMICOLON,         

    // One or two character tokens
    TK_MINUS,        
    TK_PLUS,             
    TK_SLASH,                        
    TK_STAR,                                         
    TK_EQUAL_EQUAL,                
    TK_GREATER, 
    TK_GREATER_EQUAL,   
    TK_LESS, 
    TK_LESS_EQUAL,  
    TK_AND, 
    TK_OR,
    TK_EQUAL,      

    // Literals
    TK_IDENTIFIER,                                      
    TK_NUMBER_VAL,                                    
    TK_STRING_VAL,                                     
                            

    // Keywords                                        
    TK_IF,                                              
    TK_ELSE,                                            
    TK_TRUE,                                                  
    TK_FALSE,                                           
    TK_FOR,                                             
    TK_WHILE,
    TK_NULL, 
    TK_VAR,                                 
    TK_PRINT,

    TK_ERROR,
    TK_EOF, 
} spl_token_type;

typedef struct {
    spl_token_type type;
    const char* start;
    int length;
    int line;
} spl_token;


void spl_lex_init(const char* source);
void spl_lex_free();
spl_token next_token(void);

#endif
