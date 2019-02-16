%{
    #include "syntax.tab.h"
    #include "syntaxTree.h"
    extern int errors;
%}

%option yylineno

INT_D   0|([1-9][0-9]*)
INT_O   0[0-7]*
INT_O_ERROR 0[0-9a-zA-Z][0-9a-zA-Z]*
INT_H   0[xX][0-9a-fA-F]*
INT_H_ERROR [0][Xx][0-9a-zA-Z]*
INT     {INT_D}|{INT_O}|{INT_H}
FLOAT   {INT_D}\.[0-9]+
FLOAT_E {INT_D}?\.[0-9]*[eE][+-]?[0-9]+
ID      [a-zA-Z_][0-9a-zA-Z_]*
SEMI    ";"
COMMA   ","
ASSIGNOP    "="
RELOP   >|<|>=|<=|==|!=
PLUS    "+"
MINUS   "-"
STAR    "*"
DIV     "/"
AND     "&&"
OR      "||"
DOT     "."
NOT     "!"
TYPE    int|float
LP      "("
RP      ")"
LB      "["
RB      "]"
LC      "{"
RC      "}"
STRUCT  struct
RETURN  return
IF      if
ELSE    else
WHILE   while
SPACE   [ \r\n\t]
COMMENT "//"
ANNO	"/*"(([^\*]*(\*[^\/])?)*)"*/"
OTHER .

%%

{SEMI}      {yylval.node = newNode("SEMI", yytext); return SEMI;}
{COMMA}     {yylval.node = newNode("COMMA", yytext); return COMMA;}
{ASSIGNOP}  {yylval.node = newNode("ASSIGNOP", yytext); return ASSIGNOP;}
{RELOP}     {yylval.node = newNode("RELOP", yytext); return RELOP;}
{PLUS}      {yylval.node = newNode("PLUS", yytext); return PLUS;}
{MINUS}     {yylval.node = newNode("MINUS", yytext); return MINUS;}
{STAR}      {yylval.node = newNode("STAR", yytext); return STAR;}
{DIV}       {yylval.node = newNode("DIV", yytext); return DIV;}
{AND}       {yylval.node = newNode("AND", yytext); return AND;}
{OR}        {yylval.node = newNode("OR", yytext); return OR;}
{DOT}       {yylval.node = newNode("DOT", yytext); return DOT;}
{NOT}       {yylval.node = newNode("NOT", yytext); return NOT;}
{TYPE}      {yylval.node = newNode("TYPE", yytext); return TYPE;}
{LP}        {yylval.node = newNode("LP", yytext); return LP;}
{RP}        {yylval.node = newNode("RP", yytext); return RP;}
{LB}        {yylval.node = newNode("LB", yytext); return LB;}
{RB}        {yylval.node = newNode("RB", yytext); return RB;}
{LC}        {yylval.node = newNode("LC", yytext); return LC;}
{RC}        {yylval.node = newNode("RC", yytext); return RC;}
{STRUCT}    {yylval.node = newNode("STRUCT", yytext); return STRUCT;}
{RETURN}    {yylval.node = newNode("RETURN", yytext); return RETURN;}
{IF}        {yylval.node = newNode("IF", yytext); return IF;}
{ELSE}      {yylval.node = newNode("ELSE", yytext); return ELSE;}
{WHILE}     {yylval.node = newNode("WHILE", yytext); return WHILE;}
{SPACE}     {}

{INT}       {yylval.node = newNode("INT", yytext); return INT;}
{INT_H_ERROR}   {errors++; printf("Error type A at Line %d: Illegal hexadecimal number '%s'\n", yylineno, yytext); return INT;}
{INT_O_ERROR}   {errors++; printf("Error type A at Line %d: Illegal octal number '%s'\n", yylineno, yytext); return INT;}
{FLOAT}     {yylval.node = newNode("FLOAT", yytext); return FLOAT;}
{FLOAT_E}   {yylval.node = newNode("FLOAT", yytext); return FLOAT;}
{ID}        {yylval.node = newNode("ID", yytext); return ID;}
{COMMENT}   {char c = input();
            while(c!='\n')
	            c=input();}
{ANNO}	    {}
{OTHER}     {errors++; printf("Error type A at line %d: Mysterious character \"%s\".\n", yylineno, yytext);}

%%

  
