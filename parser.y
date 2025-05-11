%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

extern int yylex();
extern int yyparse();
extern int line, col;
void yyerror(const char *s);
ASTNode *root;
%}

%union {
    char *str;
    ASTNode *node;
}

%token <str> STRING NUMBER
%token TRUE FALSE NULLL
%token LBRACE RBRACE LBRACKET RBRACKET COLON COMMA
%type <node> value object array pair pair_list value_list

%%

json: value { root = $1; };

value:
    STRING      { $$ = make_string_node($1); }
  | NUMBER      { $$ = make_number_node($1); }
  | TRUE        { $$ = make_bool_node(1); }
  | FALSE       { $$ = make_bool_node(0); }
  | NULLL       { $$ = make_null_node(); }
  | object
  | array
  ;

object:
    LBRACE pair_list RBRACE { $$ = make_object_node($2); }
  | LBRACE RBRACE           { $$ = make_object_node(NULL); }
  ;

pair_list:
    pair                    { $$ = make_pair_list($1, NULL); }
  | pair COMMA pair_list    { $$ = make_pair_list($1, $3); }
  ;

pair:
    STRING COLON value      { $$ = make_pair_node($1, $3); }
  ;

array:
    LBRACKET value_list RBRACKET { $$ = make_array_node($2); }
  | LBRACKET RBRACKET            { $$ = make_array_node(NULL); }
  ;

value_list:
    value                   { $$ = make_array_list($1, NULL); }
  | value COMMA value_list  { $$ = make_array_list($1, $3); }
  ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax Error at line %d, column %d: %s\n", line, col, s);
    exit(1);
}
