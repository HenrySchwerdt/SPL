# SPL


## Grammar

```
program         -> statement*

statement       -> variable_declaration
                 | assignment_statement
                 | if_statement
                 | while_statement
                 | print_statement

variable_declaration -> 'var' variable_name '=' (number_literal | string_literal) ';'

assignment_statement -> variable_name '=' expression ';'

if_statement    -> 'if' '(' expression ')' block ('else' block)?

while_statement -> 'while' '(' expression ')' block

print_statement -> 'print' expression ';'

block           -> '{' statement* '}'

expression      -> term (add_operator term)*

term            -> factor (mul_operator factor)*

factor          -> number_literal
                 | string_literal
                 | variable_name
                 | '(' expression ')'

add_operator    -> '+' | '-'

mul_operator    -> '*' | '/'

variable_name   -> letter alnum*

number_literal  -> digit+

string_literal  -> '"' (char | escape)* '"'

char            -> any character except '"'

escape          -> '\' ( '"' | '\' )

digit           -> '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'

letter          -> 'a' | 'b' | ... | 'z' | 'A' | 'B' | ... | 'Z'

alnum           -> letter | digit

```
