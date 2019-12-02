/****************************************************/
/* File: globals.h                                  */
/* Yacc/Bison Version					            */
/* Global types and vars for C- compiler            */
/* Compiler Construction: Principles and Practice   */
/****************************************************/

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef YYPARSER
/* the name of the following file may change */
#include "cm.tab.h"

/* ENDFILE is implicitly defined by Yacc/Bison,
*  and not included in the tab.h file
*/

#define ENDFILE 0
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* MAXRESERVED = the number of reserved words */
#define MAXRESERVED 8
/*
typedef enum
{
	//book-keeping tokens
	ENDFILE, ERROR,
	//reserved words
	IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE, INT, RETURN, WHILE, VOID,
	//multicharacter tokens
	ID, NUM,
	//special symbols
	ASSIGN, EQ, LT, LE, NE, GT, GE, PLUS, MINUS, TIMES, OVER, LPAREN, RPAREN, SEMI, LBRACKET, RBRACKET, COMMA, LSQUARE, RSQUARE, COMMENT
} TokenType;
*/

/* Yacc/Bison generates its own integer values
   for token
   */
typedef int TokenType;

extern FILE* source;	/* source code text file */
extern FILE* listing;	/* listing output text file */
extern FILE* code;		/* code text file for TM simulator */

extern int lineno;		/* source line number for listing */
extern int g;

/*************************************************/
/******* Syntax tree for parsing *****************/
/*************************************************/

typedef enum { StmtK, ExpK, DeclK } NodeKind;
typedef enum { IfK, WhileK, ReturnK } StmtKind;
typedef enum { OpK, ConstK, IdK, CallK } ExpKind;
typedef enum { VarK, ParamSeqK, ParamK, FuncK, TypeK, CmpndK } DeclKind;

//ExpType is used for type checking

typedef enum { Void, Int, Array, FuncError, VarError } ExpType;

#define MAXCHILDREN 3

typedef struct treeNode
{
	struct treeNode *child[MAXCHILDREN];
	struct treeNode *sibling;
	int lineno;
	NodeKind nodekind;
	union { StmtKind stmt; ExpKind exp; DeclKind decl; } kind;
	union { TokenType op;
			int val;
			char *name; } attr;
	int array_size;
	int param_num;
	int memloc;
	ExpType type; /* for type checking of exps */
} TreeNode;

/************************************************/
/*********** Flags for tracing ******************/
/************************************************/

/* EchoSource = TRUE causes the source program to
	be echoed to the listing file with line numbers
	during parsing
*/

extern int EchoSource;

/* TraceScan = TRUE causes token information to be
	printed to the listing file as each token is
	recognized by the scanner
	*/

extern int TraceScan;

/* TraceParse = TRUE causes the syntax tree to be
   printed to the listing file in linearized form
   (using indents for children)
   */

extern int TraceParse;

/* TraceAnalyze = TRUE causes symbol table inserts
   and lookups to be reported to the listing file
   */
extern int TraceAnalyze;

/* TraceCode = TRUE causes comments to be written
   to the TM code file as code is generated
   */
extern int TraceCode;

extern int Error;
#endif
