%{
#include "ast.h"
#include "parser.h"
#include <string.h>
#include <stdlib.h>

int line = 1;
int col = 1;

void update_position(const char *text);
%}

%option noyywrap

STRING      \"(\\.|[^"\\])*\"
NUMBER      -?[0-9]+(\.[0-9]+)?([eE][-+]?[0-9]+)?
WHITESPACE  [ \t\r]+
NEWLINE     \n
TRUE        true
FALSE       false
NULL        null

%%

{STRING}    { yylval.str = strdup(yytext); update_position(yytext); return STRING; }
{NUMBER}    { yylval.str = strdup(yytext); update_position(yytext); return NUMBER; }
{TRUE}      { update_position(yytext); return TRUE; }
{FALSE}     { update_position(yytext); return FALSE; }
{NULL}      { update_position(yytext); return NULLL; }

"{"         { update_position(yytext); return LBRACE; }
"}"         { update_position(yytext); return RBRACE; }
"["         { update_position(yytext); return LBRACKET; }
"]"         { update_position(yytext); return RBRACKET; }
":"         { update_position(yytext); return COLON; }
","         { update_position(yytext); return COMMA; }

{NEWLINE}   { line++; col = 1; }
{WHITESPACE} { update_position(yytext); }

.           { fprintf(stderr, "Lexical Error at line %d, column %d: unexpected character '%s'\n", line, col, yytext); exit(1); }

%%

void update_position(const char *text) {
    for (int i = 0; text[i]; i++) {
        if (text[i] == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
    }
}