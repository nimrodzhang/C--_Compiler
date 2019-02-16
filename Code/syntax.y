%{
    #include "syntaxTree.h"
    #include "lex.yy.c"

    extern int yylineno;
    extern Node* root;
    extern int errors; 

    void yyerror(char *msg);
    void myerror(char* msg);
    int errorlineno = 0;
%}

%union {
    struct SyntaxTree* node;
}

%token <node> SEMI COMMA ASSIGNOP RELOP
%token <node> PLUS MINUS STAR DIV
%token <node> AND OR DOT NOT TYPE 
%token <node> LP RP LB RB LC RC
%token <node> STRUCT RETURN IF ELSE WHILE
%token <node> INT FLOAT ID

%right ASSIGNOP  
%left OR 
%left AND 
%left RELOP
%left PLUS MINUS 
%left STAR DIV
%right NOT
%left DOT LB RB LP RP

%nonassoc LOWER_THAN_ELSE 
%nonassoc ELSE

%type <node> Program ExtDefList ExtDef ExtDecList Specifier
%type <node> StructSpecifier OptTag Tag VarDec FunDec VarList
%type <node> ParamDec CompSt StmtList Stmt DefList Def DecList
%type <node> Dec Exp Args

%%


Program :   ExtDefList    {$$ = insertNode("Program", 1, $1); root = $$;}
        ;
ExtDefList  :   ExtDef ExtDefList   {$$ = insertNode("ExtDefList", 2, $1, $2);}
            |   /* empty */         {$$ = NULL;}
            ;
ExtDef  :   Specifier ExtDecList SEMI   {$$ = insertNode("ExtDef", 3, $1, $2, $3);}
        |   Specifier SEMI              {$$ = insertNode("ExtDef", 2, $1, $2);}
        |   Specifier FunDec CompSt     {$$ = insertNode("ExtDef", 3, $1, $2, $3);}
        |   Specifier error SEMI        {errors++; myerror("Syntax error.");}
	|   Specifier FunDec SEMI	{$$ = insertNode("ExtDef", 3, $1, $2, $3);}
        ;
ExtDecList  :   VarDec                  {$$ = insertNode("ExtDecList", 1, $1);}
            |   VarDec COMMA ExtDecList {$$ = insertNode("ExtDecList", 3, $1, $2, $3);}
            ;


Specifier   :   TYPE            {$$ = insertNode("Specifier", 1, $1);}
            |   StructSpecifier {$$ = insertNode("Specifier", 1, $1);}
            ;
StructSpecifier :   STRUCT OptTag LC DefList RC {$$ = insertNode("StructSpecifier", 5, $1, $2, $3, $4, $5);}
                |   STRUCT Tag                  {$$ = insertNode("StructSpecifier", 2, $1, $2);}
                ;
OptTag  :   ID  {$$ = insertNode("OptTag", 1, $1);}
        |       {$$ = NULL;}
        ;
Tag :   ID  {$$ = insertNode("Tag", 1, $1);}
    ;


VarDec  :   ID                  {$$ = insertNode("VarDec", 1, $1);}
        |   VarDec LB INT RB    {$$ = insertNode("VarDec", 4, $1, $2, $3, $4);}
        //|   VarDec LB error RB  {errors++; myerror("Syntax error.");}
        ;
FunDec  :   ID LP VarList RP    {$$ = insertNode("FunDec", 4, $1, $2, $3, $4);}
        |   ID LP RP            {$$ = insertNode("FunDec", 3, $1, $2, $3);}
        |   error RP            {errors++; myerror("Syntax error.");}
        ;
VarList :   ParamDec COMMA VarList  {$$ = insertNode("VarList", 3, $1, $2, $3);}
        |   ParamDec                {$$ = insertNode("VarList", 1, $1);}
        ;
ParamDec    :   Specifier VarDec   {$$ = insertNode("ParamDec", 2, $1, $2);}
            //|   error COMMA         {errors++; myerror("Missing \",\".");}
            //|   error RP            {errors++; myerror("Missing \")\".");}
            ;


CompSt  :   LC DefList StmtList RC  {$$ = insertNode("CompSt", 4, $1, $2, $3, $4);}
       // |   LC error RC             {errors++; myerror("Syntax error.");}
        ;
StmtList    :   Stmt StmtList   {$$ = insertNode("StmtList", 2, $1, $2);}
            |                   {$$ = NULL;}
            ;
Stmt    :   Exp SEMI                                {$$ = insertNode("Stmt", 2, $1, $2);}
	|   error SEMI				    {errors++; myerror("Syntax error.");}
        |   CompSt                                  {$$ = insertNode("Stmt", 1, $1);}
        |   RETURN Exp SEMI                         {$$ = insertNode("Stmt", 3, $1, $2, $3);}
	|   RETURN error SEMI			    {errors++; myerror("Syntax error.");}
        |   IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$ = insertNode("Stmt", 5, $1, $2, $3, $4, $5);}
        |   IF LP Exp RP Stmt ELSE Stmt             {$$ = insertNode("Stmt", 7, $1, $2, $3, $4, $5, $6, $7);}
        //|   IF LP Exp RP error ELSE Stmt            {errors++; myerror("Missing \";\".");}
        |   WHILE LP Exp RP Stmt                    {$$ = insertNode("Stmt", 5, $1, $2, $3, $4, $5);}
        ;


DefList :   Def DefList {$$ = insertNode("DefList", 2, $1, $2);}
        |               {$$ = NULL;}
	;
Def :   Specifier DecList SEMI  {$$ = insertNode("Def", 3, $1, $2, $3);}
    ;
DecList :   Dec                 {$$ = insertNode("DecList", 1, $1);}
        |   Dec COMMA DecList   {$$ = insertNode("DecList", 3, $1, $2, $3);}
        ;
Dec :   VarDec              {$$ = insertNode("Dec", 1, $1);}
    |   VarDec ASSIGNOP Exp {$$ = insertNode("Dec", 3, $1, $2, $3);}
    ;


Exp :   Exp ASSIGNOP Exp    {$$ = insertNode("Exp", 3, $1, $2, $3);}
    |   Exp AND Exp         {$$ = insertNode("Exp", 3, $1, $2, $3);}
    |   Exp OR Exp          {$$ = insertNode("Exp", 3, $1, $2, $3);}
    |   Exp RELOP Exp       {$$ = insertNode("Exp", 3, $1, $2, $3);}
    |   Exp PLUS Exp        {$$ = insertNode("Exp", 3, $1, $2, $3);}
    |   Exp MINUS Exp       {$$ = insertNode("Exp", 3, $1, $2, $3);}
    |   Exp STAR Exp        {$$ = insertNode("Exp", 3, $1, $2, $3);}
    |   Exp DIV Exp         {$$ = insertNode("Exp", 3, $1, $2, $3);}
    |   LP Exp RP           {$$ = insertNode("Exp", 3, $1, $2, $3);}
    |   MINUS Exp           {$$ = insertNode("Exp", 2, $1, $2);}
    |   NOT Exp             {$$ = insertNode("Exp", 2, $1, $2);}
    |   ID LP Args RP       {$$ = insertNode("Exp", 4, $1, $2, $3, $4);}
    |   ID LP RP            {$$ = insertNode("Exp", 3, $1, $2, $3);}
    |   Exp LB Exp RB       {$$ = insertNode("Exp", 4, $1, $2, $3, $4);}
    |   Exp LB error RB     {errors++; myerror("Missing \"]\".");}
    |   Exp DOT ID          {$$ = insertNode("Exp", 3, $1, $2, $3);}
    |   ID                  {$$ = insertNode("Exp", 1, $1);}
    |   INT                 {$$ = insertNode("Exp", 1, $1);}
    |   FLOAT               {$$ = insertNode("Exp", 1, $1);}
    ;
Args    :   Exp COMMA Args  {$$ = insertNode("Args", 3, $1, $2, $3);}
        |   Exp             {$$ = insertNode("Args", 1, $1);}
        ;



%%

void yyerror(char* msg) {}

void myerror(char *msg) {
    if(errorlineno != yylineno) {
        printf("Error type B at line %d: %s\n", yylineno, msg);
        errorlineno = yylineno;
    }
}
