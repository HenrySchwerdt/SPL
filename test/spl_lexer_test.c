#include "../src/spl_lexer.h"
#include "../include/acutest.h"

void should_identify_single_character_tokens(void)
{
    // given
    const char *source = "(){}[].;!";
    spl_lex_init(source);

    // when
    spl_token should_left_paren = next_token();
    spl_token should_right_paren = next_token();
    spl_token should_left_brace = next_token();
    spl_token should_right_brace = next_token();
    spl_token should_left_bracket = next_token();
    spl_token should_right_bracket = next_token();
    spl_token should_dot = next_token();
    spl_token should_semi_colon = next_token();
    spl_token should_bang = next_token();
    spl_token should_eof = next_token();

    // then
    TEST_CHECK(should_left_paren.type == TK_LEFT_PAREN);
    TEST_CHECK(should_left_paren.line == 1);
    TEST_CHECK(should_left_paren.length == 1);

    TEST_CHECK(should_right_paren.type == TK_RIGHT_PAREN);
    TEST_CHECK(should_right_paren.line == 1);
    TEST_CHECK(should_right_paren.length == 1);

    TEST_CHECK(should_left_brace.type == TK_LEFT_BRACE);
    TEST_CHECK(should_left_brace.line == 1);
    TEST_CHECK(should_left_brace.length == 1);

    TEST_CHECK(should_right_brace.type == TK_RIGHT_BRACE);
    TEST_CHECK(should_right_brace.line == 1);
    TEST_CHECK(should_right_brace.length == 1);

    TEST_CHECK(should_left_bracket.type == TK_LEFT_BRACKET);
    TEST_CHECK(should_left_bracket.line == 1);
    TEST_CHECK(should_left_bracket.length == 1);

    TEST_CHECK(should_right_bracket.type == TK_RIGHT_BRACKET);
    TEST_CHECK(should_right_bracket.line == 1);
    TEST_CHECK(should_right_bracket.length == 1);

    TEST_CHECK(should_dot.type == TK_DOT);
    TEST_CHECK(should_dot.line == 1);
    TEST_CHECK(should_dot.length == 1);

    TEST_CHECK(should_semi_colon.type == TK_SEMICOLON);
    TEST_CHECK(should_semi_colon.line == 1);
    TEST_CHECK(should_semi_colon.length == 1);

    TEST_CHECK(should_bang.type == TK_BANG);
    TEST_CHECK(should_bang.line == 1);
    TEST_CHECK(should_bang.length == 1);

    TEST_CHECK(should_eof.type == TK_EOF);

    spl_lex_free();
}

void should_increment_line_number_with_comments(void)
{
    // given
    const char *source = "/*This is a \n multi line \n comment*/\n(\n//Comment\n)";
    spl_lex_init(source);

    // when
    spl_token should_left_paren = next_token();
    spl_token should_right_paren = next_token();
    spl_token should_eof = next_token();

    // then
    TEST_CHECK(should_left_paren.type == TK_LEFT_PAREN);
    TEST_CHECK(should_left_paren.line == 4);
    TEST_CHECK(should_left_paren.length == 1);

    TEST_CHECK(should_right_paren.type == TK_RIGHT_PAREN);
    TEST_CHECK(should_right_paren.line == 6);
    TEST_CHECK(should_left_paren.length == 1);

    TEST_CHECK(should_eof.type == TK_EOF);
    spl_lex_free();
}

void should_identify_operators(void)
{
    // given
    const char *source = "- + * /";
    spl_lex_init(source);

    // when
    spl_token should_minus = next_token();
    spl_token should_plus = next_token();
    spl_token should_star = next_token();
    spl_token should_slash = next_token();
    spl_token should_eof = next_token();

    // then
    TEST_CHECK(should_minus.type == TK_MINUS);
    TEST_CHECK(should_minus.length == 1);
    TEST_CHECK(should_plus.type == TK_PLUS);
    TEST_CHECK(should_plus.length == 1);
    TEST_CHECK(should_star.type == TK_STAR);
    TEST_CHECK(should_star.length == 1);
    TEST_CHECK(should_slash.type == TK_SLASH);
    TEST_CHECK(should_slash.length == 1);
    TEST_CHECK(should_eof.type == TK_EOF);
    spl_lex_free();
}

void should_identify_comparisons(void)
{
    // given
    const char *source = "< > <= >= ==";
    spl_lex_init(source);

    // when
    spl_token should_less = next_token();
    spl_token should_greater = next_token();
    spl_token should_less_equal = next_token();
    spl_token should_greater_equal = next_token();
    spl_token should_equal_equal = next_token();
    spl_token should_eof = next_token();

    // then
    TEST_CHECK(should_less.type == TK_LESS);
    TEST_CHECK(should_less.length == 1);
    TEST_CHECK(should_greater.type == TK_GREATER);
    TEST_CHECK(should_greater.length == 1);
    TEST_CHECK(should_less_equal.type == TK_LESS_EQUAL);
    TEST_CHECK(should_less_equal.length == 2);
    TEST_CHECK(should_greater_equal.type == TK_GREATER_EQUAL);
    TEST_CHECK(should_greater_equal.length == 2);
    TEST_CHECK(should_equal_equal.type == TK_EQUAL_EQUAL);
    TEST_CHECK(should_equal_equal.length == 2);

    TEST_CHECK(should_eof.type == TK_EOF);
    spl_lex_free();
}


void should_identify_and_or(void)
{
    // given
    const char *source = "& |";
    spl_lex_init(source);

    // when
    spl_token should_and = next_token();
    spl_token should_or = next_token();
    spl_token should_eof = next_token();

    // then
    TEST_CHECK_(should_and.type == TK_AND, "Type: %d == %d", should_and.type, TK_AND);
    TEST_CHECK_(should_and.length == 1, "Length: %d == %d", should_and.length, 1);
    TEST_CHECK_(should_or.type == TK_OR, "Type: %d == %d", should_or.type, TK_OR);
    TEST_CHECK_(should_or.length == 1, "Length: %d == %d", should_or.length, 2);
    TEST_CHECK(should_eof.type == TK_EOF);
    spl_lex_free();
}


void should_identify_numbers(void)
{
    const char *types[] = {"1", "10", "10f", ".3", "10.3f"};
    const int lengths[] = {1, 2, 3, 2, 5};

    for (int i = 0; i < 5; i++)
    {
        // given
        spl_lex_init(types[i]);

        // when
        spl_token token = next_token();
        spl_token eof = next_token();
        // then
        TEST_CHECK_(token.type == TK_NUMBER_VAL, "Type: %d == %d", token.type, TK_NUMBER_VAL);
        TEST_CHECK_(token.length == lengths[i], "Length: %d == %d", token.length, lengths[i]);

        TEST_CHECK(eof.type == TK_EOF);

        spl_lex_free();
    }
}

void should_identify_reserved_words()
{
    const char *types[] = {"if", "else", "true", "false",
                           "for", "while", "null", "var", "print"};
    const int lengths[] = {2, 4, 4, 5, 3, 5, 4, 3, 5};
    int start_token = 24;

    for (int i = 0; i < 9; i++)
    {
        // given
        spl_lex_init(types[i]);

        // when
        spl_token token = next_token();
        spl_token eof = next_token();
        // then
        TEST_CHECK_(token.type == start_token, "Type: %d == %d", token.type, start_token);
        TEST_CHECK_(token.length == lengths[i], "Length: %d == %d", token.length, lengths[i]);

        TEST_CHECK(eof.type == TK_EOF);

        start_token += 1;

        spl_lex_free();
    }
}

void should_identify_identifier()
{
    const char *types[] = {"class_name", "struct4", "variable_name", "x"};
    const int lengths[] = {10, 7, 13, 1};

    for (int i = 0; i < 4; i++)
    {
        // given
        spl_lex_init(types[i]);

        // when
        spl_token token = next_token();
        spl_token eof = next_token();
        // then
        TEST_CHECK_(token.type == TK_IDENTIFIER, "Type: %d == %d", token.type, TK_IDENTIFIER);
        TEST_CHECK_(token.length == lengths[i], "Length: %d == %d", token.length, lengths[i]);

        TEST_CHECK(eof.type == TK_EOF);

        spl_lex_free();
    }
}

TEST_LIST = {
    {": Should increment line numbers in comments", should_increment_line_number_with_comments},
    {": Should identify '(){}[],.~:;?!' tokens", should_identify_single_character_tokens},
    {": Should identify '+ - * /' tokens", should_identify_operators},
    {": Should identify '< > <= >= ==' tokens", should_identify_comparisons},
    {": Should identify '& |' tokens", should_identify_and_or},
    {": Should identify number tokens", should_identify_numbers},
    {": Should identify reserved words", should_identify_reserved_words},
    {": Should identify identifiers", should_identify_identifier},
    {NULL, NULL}
};