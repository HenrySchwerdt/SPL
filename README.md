# SPL


## Grammar

```
program         : statement*

statement       : expression_statement
                | if_statement
                | for_statement
                | while_statement
                | variable_declaration_statement
                | function_declaration_statement

expression_statement : expression ';'

if_statement    : 'if' '(' expression ')' statement ('else' statement)?

for_statement   : 'for' '(' expression_statement expression_statement? ';' expression? ')' statement

while_statement : 'while' '(' expression ')' statement

variable_declaration_statement : type identifier ('=' expression)? ';'

function_declaration_statement : type identifier '(' parameter_list? ')' compound_statement

parameter_list  : parameter (',' parameter)*
parameter       : type identifier

compound_statement : '{' statement* '}'

expression      : assignment_expression

assignment_expression : ternary_expression
                      | ternary_expression assignment_operator assignment_expression

ternary_expression : logical_or_expression ('?' expression ':' ternary_expression)?

logical_or_expression   : logical_and_expression ('||' logical_and_expression)*
logical_and_expression  : equality_expression ('&&' equality_expression)*
equality_expression     : relational_expression (('==' | '!=') relational_expression)*
relational_expression   : additive_expression (('>' | '<' | '>=' | '<=') additive_expression)*
additive_expression     : multiplicative_expression (('+' | '-') multiplicative_expression)*
multiplicative_expression : unary_expression (('*' | '/' | '%') unary_expression)*

unary_expression    : ('+' | '-' | '!' | '++' | '--')? primary_expression
primary_expression  : identifier
                    | literal
                    | '(' expression ')'
                    | function_call_expression

function_call_expression : identifier '(' argument_list? ')'

argument_list       : expression (',' expression)*

identifier          : [a-zA-Z_][a-zA-Z0-9_]*
literal             : integer_literal | float_literal | string_literal
integer_literal     : [0-9]+
float_literal       : [0-9]*'.'[0-9]+
string_literal      : '"' [^"]* '"'

type                : 'int' | 'float' | 'bool' | 'void'
```
