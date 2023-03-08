#include "spl_lexer.h"

typedef struct
{
    const char *file_start;
    const char *start;
    const char *current;
    int line;
} spl_lexer;

spl_lexer lexer;

//--------------------------------------
// Private Functions

static bool is_end()
{
    return *lexer.current == '\0';
}

static void next()
{
    lexer.current++;
}

static char advance()
{
    lexer.current++;
    return *(lexer.current - 1);
}

static char current()
{
    return *lexer.current;
}

static char peek()
{
    if (*lexer.current != '\0')
    {
        return *(lexer.current + 1);
    }
    return '\0';
}

static bool match(char expected)
{
    if (is_end())
        return false;
    if (*lexer.current != expected)
        return false;
    lexer.current++;
    return true;
}

static void skip_whitespaces_and_comments()
{
    for (;;)
    {
        switch (current())
        {
        case ' ':
        case '\r':
        case '\t':
            next();
            break;
        case '\n':
            lexer.line++;
            next();
            break;
        case '/':
            if (peek() == '/')
            {
                while (peek() != '\n' && !is_end())
                    next();
                next();
                break;
            }
            else if (peek() == '*')
            {
                while (!(current() == '*' && peek() == '/') && !is_end())
                {
                    if (current() == '\n')
                        lexer.line++;
                    next();
                }
                next(); // current = '/'
                next(); // current = Next Char
                break;
            }
            else
            {
                return;
            }
        default:
            return;
        }
    }
}

static bool is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static spl_token create_token(spl_token_type type)
{
    spl_token token;
    token.type = type;
    token.start = lexer.start;
    token.length = (int)(lexer.current - lexer.start);
    token.line = lexer.line;
    return token;
}

static spl_token_type check_keyword(int start, int length, const char *rest, spl_token_type type)
{
    if (lexer.current - lexer.start == start + length && memcmp(lexer.start + start, rest, length) == 0)
    {
        return type;
    }

    return TK_IDENTIFIER;
}


static spl_token_type identifierType()
{
    switch (lexer.start[0])
    {
    case 'e':
        return check_keyword(1, 3, "lse", TK_ELSE);
    case 'i':
        return check_keyword(1, 1, "f", TK_IF);
    case 'n':
        return check_keyword(1, 3, "ull", TK_NULL);
    case 't':
        return check_keyword(1, 3, "rue", TK_TRUE);
    case 'f':
        if (lexer.current - lexer.start > 1)
        {
            switch (lexer.start[1])
            {
                case 'a':
                    return check_keyword(2, 3, "lse", TK_FALSE);
                case 'o':
                    return check_keyword(2, 1, "r", TK_FOR);
            }
        }
        break;
    case 'p':
        return check_keyword(1, 4, "rint", TK_PRINT);
    case 'v':
        return check_keyword(1, 2, "ar", TK_VAR);
    case 'w':
        return check_keyword(1, 4, "hile", TK_WHILE);
    }

    return TK_IDENTIFIER;
}

static spl_token identifier()
{
    while (is_alpha(current()) || is_digit(current()))
        advance();

    return create_token(identifierType());
}

static spl_token floatDot()
{
    while (is_digit(peek()) && !is_end())
    {
        advance();
    }
    if (peek() == 'f')
    {
        advance();
    }
    advance();
    return create_token(TK_NUMBER_VAL);
}

// [0-9][.[0-9]+]?f?
static spl_token number()
{
    while (is_digit(current()) && !is_end())
    {
        advance();
    }
    if (current() == '.')
    {
        advance();
        while (is_digit(current()) && !is_end())
        {
            advance();
        }
        if (current() == 'f')
        {
            advance();
        }
        return create_token(TK_NUMBER_VAL);
    }
    if (current() == 'f')
    {
        advance();
        return create_token(TK_NUMBER_VAL);
    }
    return create_token(TK_NUMBER_VAL);
}

static spl_token string()
{
    while (peek() != '"' && !is_end())
    {
        if (peek() == '\n')
            lexer.line++;
        advance();
    }
    // if (is_end()) // TODO return errorToken;
    //     printf("unterminated String\n");
    advance(); // current = "
    advance(); // current = new Char
    return create_token(TK_STRING_VAL);
}

//--------------------------------------
// Public Functions
void spl_lex_init(const char *source)
{
    lexer.file_start = source;
    lexer.start = source;
    lexer.current = source;
    lexer.line = 1;
}

void spl_lex_free()
{
    lexer.file_start = NULL;
    lexer.start = NULL;
    lexer.current = NULL;
    lexer.line = 1;
}

spl_token next_token()
{
    skip_whitespaces_and_comments();
    lexer.start = lexer.current;
    if (is_end())
        return create_token(TK_EOF);
    char c = advance();
    if (is_alpha(c))
        return identifier();
    if (is_digit(c))
        return number();
    switch (c)
    {
    // single character tokens
    case '(':
        return create_token(TK_LEFT_PAREN);
    case ')':
        return create_token(TK_RIGHT_PAREN);
    case '{':
        return create_token(TK_LEFT_BRACE);
    case '}':
        return create_token(TK_RIGHT_BRACE);
    case '[':
        return create_token(TK_LEFT_BRACKET);
    case ']':
        return create_token(TK_RIGHT_BRACKET);
    case '.':
        return is_digit(current()) ? floatDot() : create_token(TK_DOT);
    case ';':
        return create_token(TK_SEMICOLON);
    case '!':
        return create_token(TK_BANG);
    // single or double character tokens
    case '-':
        return create_token(TK_MINUS);
    
    case '+':
        return create_token(TK_PLUS);
    
    case '/':
        return create_token(TK_SLASH);
    case '*':
        return create_token(TK_STAR);
    case '&':
        return create_token(TK_AND);
    case '|':
        return create_token(TK_OR);
    case '=':
    {
        switch (current())
        {
        case '=':
            next();
            return create_token(TK_EQUAL_EQUAL);
        default:
            return create_token(TK_EQUAL);
        }
    }
    case '>':
    {
        switch (current())
        {
        case '=':
            next();
            return create_token(TK_GREATER_EQUAL);
        default:
            return create_token(TK_GREATER);
        }
    }
    case '<':
    {
        switch (current())
        {
        case '=':
            next();
            return create_token(TK_LESS_EQUAL);
        default:
            return create_token(TK_LESS);
        }
    }
    case '"':
        return string();
    }
    return create_token(TK_ERROR);
}