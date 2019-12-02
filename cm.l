/****************************************************/
/* File: tiny.l                                     */
/* Lex specification for TINY                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

%{
#include "globals.h"
#include "util.h"
#include "scan.h"
// lexeme of identifier or reserved word
char tokenString[MAXTOKENLEN + 1];
%}

digit           [0-9]
number          {digit}+
letter          [a-zA-Z]
identifier      {letter}+
newline         [\n\r]
whitespace      [ \t]+

%%

"if"			{ return IF; }
"else"			{ return ELSE; }
"int"			{ return INT; }
"return"		{ return RETURN; }
"while"			{ return WHILE; }
"void"			{ return VOID; }
"=="			{ return EQ; }
"!="			{ return NE; }
"<="			{ return LE; }
">="			{ return GE; }
"="				{ return ASSIGN; }
"<"				{ return LT; }
">"				{ return GT; }
"+"				{ return PLUS; }
"-"				{ return MINUS; }
"*"				{ return TIMES; }
"("				{ return LPAREN; }
")"				{ return RPAREN; }
";"				{ return SEMI; }
"{"				{ return LBRACKET; }
"}"				{ return RBRACKET; }
","				{ return COMMA; }
"["				{ return LSQUARE; }
"]"				{ return RSQUARE; }
{number}		{ return NUM; }
{identifier} 	{ return ID;}
{newline}		{ lineno++; }
{whitespace}	{ /* skip whitespace */ }
"/*"			{ register int c;
				  for( ; ; ) {
					  while((c = input()) != '*' && c != EOF) {
						  if(c == '\n') lineno++; /* eat up text of comment */
					  }
					  if(c == '*') {
						  while((c = input()) == '*')
							  ;
						  if(c == '/')
							  break;	/* found the end */
					  }
					  if(c == '\n') lineno++;
					  if(c == EOF) {
					  	  return COMMENT;
						  break;
					  }
				  }
				}
"/"				{ return OVER; }
.				{ return ERROR; }

%%

TokenType getToken(void) {
	static int firstTime = TRUE;
	TokenType currentToken;
	if(firstTime) {
		firstTime = FALSE;
		lineno++;
		yyin = source;
		yyout = listing;
	}

	currentToken = yylex();
	if(currentToken == ID || currentToken == NUM)
		strncpy(tokenString, yytext, MAXTOKENLEN);
	if (TraceScan) {
	//	fprintf(listing, "\t%d\t\t\t ", lineno);
	//	printToken(currentToken, tokenString);
	}
	return currentToken;
}
