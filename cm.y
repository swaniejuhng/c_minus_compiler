/****************************************************/
/* File: cm.y                                       */
/* The C- Yacc/Bison specification file             */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

%{

#define YYPARSER
#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
YYSTYPE func_call;
static char * savedName1;		/* for use in assignments */
static char * savedName2;
static char * savedName3;
static char * savedName4;
static char * savedName5;
static char * savedName6;
static char * savedName7;
static int savedLineNo;			/* ditto */
static int savedArrSize;
static TreeNode * savedTree;	/* stores syntax tree for later return */
static int yylex(void);

%}

%token IF ELSE INT RETURN VOID WHILE
%token ID NUM
%token ASSIGN EQ NE LT LE GT GE PLUS MINUS TIMES OVER LPAREN RPAREN LBRACKET RBRACKET LSQUARE RSQUARE SEMI COMMA COMMENT
%token ERROR ENDFILE

%right ASSIGN
%nonassoc EQ NE LT LE GT GE
%left PLUS MINUS
%left TIMES OVER
%left LPAREN RPAREN

%%

/*Grammar for TINY C- parser */

program			: dclrtn_seq
					{ savedTree = $1; }
				;

dclrtn_seq 		: dclrtn_seq dclrtn
					{ YYSTYPE t = $1;
					  if(t != NULL) {
					  	while(t->sibling != NULL) { t = t->sibling; }
						t->sibling = $2;
						$$ = $1;
					  }
					  else $$ = $2;
					}
				| dclrtn { $$ = $1; }
				;

dclrtn 			: var_dclrtn { $$ = $1;}
				| func_dclrtn { $$ = $1; }
				;

var_dclrtn 		: type_spec ID
					{ savedName1 = copyString(tokenString);
					  savedLineNo = lineno;
					}
				  SEMI
					{ $$ = newDeclNode(VarK);
					  $$->child[0] = $1;
					  $$->type = Int;
					  $$->attr.name = savedName1;
					  $$->lineno = savedLineNo;
					}
				| type_spec ID
					{ savedName2 = copyString(tokenString);
					  savedLineNo = lineno;
					}
				  LSQUARE NUM { savedArrSize = atoi(tokenString); }
				  RSQUARE SEMI
					{ $$ = newDeclNode(VarK);
				  	  $$->child[0] = $1;
					  $$->type = Array;
				  	  $$->attr.name = savedName2;
				  	  $$->lineno = savedLineNo;
				  	  $$->array_size = savedArrSize;
					}
				;

func_dclrtn 	: type_spec ID
					{ savedName3 = copyString(tokenString);
					  //savedLineNo = lineno;
					}
				  LPAREN params RPAREN cmpnd_stmt
					{ $$ = newDeclNode(FuncK);
					  $$->param_num = 0;
					  func_call = $$;
				  	  $$->child[0] = $1;
				  	  $$->child[1] = $5;
				  	  $$->child[2] = $7;
				  	  $$->attr.name = savedName3;
				  	  $$->lineno = lineno;
					}
				;

type_spec 		: INT
					{ $$ = newDeclNode(TypeK);
					  $$->type = Int;
					}
				| VOID
					{ $$ = newDeclNode(TypeK);
					  $$->type = Void;
					}
				;

params			: param_seq
					{ $$ = newDeclNode(ParamSeqK);
					  $$->child[0] = $1;
					}
				| VOID { $$ = newDeclNode(ParamSeqK); }
				|	   { $$ = newDeclNode(ParamSeqK); }
				;

param_seq		: param_seq COMMA param
					{ YYSTYPE t = $1;
					  if(t != NULL)	{
					  	while(t->sibling != NULL) { t = t->sibling; }
						t->sibling = $3;
						$$ = $1;
					  }
					  else $$ = $3;
					}
				| param { $$ = $1; }
				;

param			: INT ID
					{ savedName4 = copyString(tokenString);
					  savedLineNo = lineno;
					  $$ = newDeclNode(ParamK);
					  $$->type = Int;
					  $$->attr.name = savedName4;
					  $$->lineno = savedLineNo;
					}
				| INT ID
					{ savedName5 = copyString(tokenString);
					  savedLineNo = lineno;
					}
				  LSQUARE RSQUARE
					{ $$ = newDeclNode(ParamK);
					  $$->type = Array;
					  $$->attr.name = savedName5;
					  $$->lineno = savedLineNo;
					}
				;

cmpnd_stmt 		: LBRACKET local_dclrtns stmt_seq RBRACKET
					{ $$ = newDeclNode(CmpndK);
					  $$->child[0] = $2;
					  $$->child[1] = $3;
					}
			  	;

local_dclrtns 	: local_dclrtns var_dclrtn
					{ YYSTYPE t = $1;
					  if(t != NULL) {
					  	while (t->sibling != NULL) { t = t->sibling; }
						t->sibling = $2;
						$$ = $1;
					  }
					  else {if($2!=NULL){$$=$2;}}
					}
				| { $$ = NULL; }
				;

stmt_seq		: stmt_seq stmt
					{ YYSTYPE t = $1;
					  if(t != NULL) {
					  	while(t->sibling != NULL) {	t = t->sibling; }
						t->sibling = $2;
						$$ = $1;
					  }
					  else $$ = $2;
					}

				| { $$ = NULL; }
				;

stmt			: exp_stmt { $$ = $1; }
				| cmpnd_stmt { $$ = $1; }
				| if_stmt { $$ = $1; }
				| while_stmt { $$ = $1; }
				| return_stmt	{ $$ = $1; }
				;

exp_stmt 		: exp SEMI { $$ = $1; }
				| SEMI { $$=NULL; }
				;

if_stmt 		: IF LPAREN exp RPAREN stmt
					{ $$ = newStmtNode(IfK);
					  $$->child[0] = $3;
					  $$->child[1] = $5;
					}
				| IF LPAREN exp RPAREN stmt ELSE stmt
					{ $$ = newStmtNode(IfK);
					  $$->child[0] = $3;
					  $$->child[1] = $5;
					  $$->child[2] = $7;
					}
				;

while_stmt 		: WHILE LPAREN exp RPAREN stmt
					{ $$ = newStmtNode(WhileK);
					  $$->child[0] = $3;
					  $$->child[1] = $5;
					}
				;

return_stmt 	: RETURN SEMI { $$ = newStmtNode(ReturnK); }
				| RETURN exp SEMI
					{ $$ = newStmtNode(ReturnK);
					  $$->child[0] = $2;
					}
				;

exp 			: var ASSIGN exp
					{ $$ = newExpNode(OpK);
					  $$->child[0] = $1;
					  $$->child[1] = $3;
					  $$->attr.op = ASSIGN;
					}
				| simple_exp { $$ = $1; }
				;

var				: ID
					{ $$ = newExpNode(IdK);
					  $$->attr.name = copyString(tokenString);
					  $$->type = Int;
					}
				| ID
					{ savedName6 = copyString(tokenString);
					  savedLineNo = lineno;
					}
				  LSQUARE exp RSQUARE
					{ $$ = newExpNode(IdK);
					  $$->type = Array;
					  $$->child[0] = $4;
					  $$->attr.name = savedName6;
					  $$->lineno = savedLineNo;
					  $$->array_size = $4->attr.val;
					}
				;

simple_exp 		: addtv_exp EQ addtv_exp
					{ $$ = newExpNode(OpK);
					  $$->child[0] = $1;
					  $$->child[1] = $3;
					  $$->attr.op = EQ;
					}
				| addtv_exp LT addtv_exp
					{ $$ = newExpNode(OpK);
					  $$->child[0] = $1;
					  $$->child[1] = $3;
					  $$->attr.op = LT;
				    }
				| addtv_exp LE addtv_exp
				    { $$ = newExpNode(OpK);
					  $$->child[0] = $1;
					  $$->child[1] = $3;
					  $$->attr.op = LE;
				    }
				| addtv_exp NE addtv_exp
					{ $$ = newExpNode(OpK);
					  $$->child[0] = $1;
					  $$->child[1] = $3;
					  $$->attr.op = NE;
				   	}
				| addtv_exp GT addtv_exp
					{ $$ = newExpNode(OpK);
					  $$->child[0] = $1;
					  $$->child[1] = $3;
					  $$->attr.op = GT;
				    }
				| addtv_exp GE addtv_exp
					{ $$ = newExpNode(OpK);
					  $$->child[0] = $1;
					  $$->child[1] = $3;
					  $$->attr.op = GE;
				    }
				| addtv_exp { $$ = $1; }
				;

addtv_exp 		: addtv_exp PLUS term
					{ $$ = newExpNode(OpK);
					  $$->child[0] = $1;
					  $$->child[1] = $3;
					  $$->attr.op = PLUS;
					}
				| addtv_exp MINUS term
					{ $$ = newExpNode(OpK);
					  $$->child[0] = $1;
					  $$->child[1] = $3;
					  $$->attr.op = MINUS;
					}
				| term { $$ = $1; }
				;

term			: term TIMES factor
			 		{ $$ = newExpNode(OpK);
				 	  $$->child[0] = $1;
				 	  $$->child[1] = $3;
				 	  $$->attr.op = TIMES;
			 	  	}
				| term OVER factor
			 		{ $$ = newExpNode(OpK);
				 	  $$->child[0] = $1;
				 	  $$->child[1] = $3;
				 	  $$->attr.op = OVER;
			 	  	}
				| factor { $$ = $1; }
				;

factor			: LPAREN exp RPAREN { $$ = $2; }
				| var { $$ = $1; }
				| call { $$ = $1; }
				| NUM { $$ = newExpNode(ConstK); $$->attr.val = atoi(tokenString); }
				;

call			: ID
					{ savedName7 = copyString(tokenString);
					  savedLineNo = lineno;
					}
				  LPAREN args RPAREN
				  	{ $$ = newExpNode(CallK);
					  $$->child[0] = $4;
					  $$->attr.name = savedName7;
					  $$->lineno = savedLineNo;
					}
				;

args			: arg_seq { $$ = $1; }
				| { $$ = NULL; }
				;

arg_seq			: arg_seq COMMA exp
					{ YYSTYPE t = $1;
					  int count = 0;
					  if(t != NULL) {
					  	while(t->sibling != NULL) { t = t->sibling; printf("hey!\n"); }
						t->sibling = $3;
						func_call->param_num++;
						$$ = $1;
					  }
					  else { $$ = $3; }
					}
				| exp { $$ = $1; }
				;

%%

int yyerror(char * message) {
	fprintf(listing, "Syntax error at line %d: %s\n", lineno, message);
	fprintf(listing, "Current token: ");
	printToken(yychar, tokenString);
	Error = TRUE;
	return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with earlier versions of
 * the C- scanner
 */

static int yylex(void) {
	return getToken();
}

TreeNode *parse(void) {
	yyparse();
	return savedTree;
}
