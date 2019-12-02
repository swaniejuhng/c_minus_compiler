/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the C- compiler                              */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"



 void filePrint(char *tk, const char *tokenString) {
     if(!strncmp(tk, "COMMA", 5) || !(strncmp(tk, "MINUS", 5))
     || !(strncmp(tk, "WHILE", 5)) || !(strncmp(tk, "ERROR", 5)))
         fprintf(listing, "%5d\t\t %s \t\t%s\n", lineno, tk, tokenString);
     else if(strlen(tk) >= 5)
         fprintf(listing, "%5d\t\t %s \t%s\n", lineno, tk, tokenString);
     else
         fprintf(listing, "%5d\t\t %s \t\t%s\n", lineno, tk, tokenString);
 }

 void printToken(TokenType token, const char *tokenString) {
     switch(token) {
         case IF: filePrint("IF", tokenString); break;
         case ELSE: filePrint("ELSE", tokenString); break;
         case INT: filePrint("INT", tokenString); break;
         case WHILE: filePrint("WHILE", tokenString); break;
         case RETURN: filePrint("RETURN", tokenString); break;
         case EQ: filePrint("==", tokenString); break;
         case NE: filePrint("!=", tokenString); break;
         case LE: filePrint("<=", tokenString); break;
         case LT: filePrint("<", tokenString); break;
         case GE: filePrint(">=", tokenString); break;
         case GT: filePrint(">", tokenString); break;
         //case INCR: filePrint("++", tokenString); break;
         case PLUS: filePrint("+", tokenString); break;
        // case DECR: filePrint("--", tokenString); break;
         case MINUS: filePrint("-", tokenString); break;
         case TIMES: filePrint("*", tokenString); break;
         case OVER: filePrint("/", tokenString); break;
         case ASSIGN: filePrint("=", tokenString); break;
         case LPAREN: filePrint("(", tokenString); break;
         case RPAREN: filePrint(")", tokenString); break;
         case LBRACKET: filePrint("{", tokenString); break;
         case RBRACKET: filePrint("}", tokenString); break;
         case LSQUARE: filePrint("[", tokenString); break;
         case RSQUARE: filePrint("]", tokenString); break;
         case SEMI: filePrint(";", tokenString); break;
         case COMMA: filePrint(",", tokenString); break;
         case NUM: filePrint("NUM", tokenString); break;
         case ID: filePrint("ID", tokenString); break;
         case COMMENT: filePrint("ERROR", "Comment Error"); break;
         case ERROR: filePrint("ERROR", tokenString); break;
         case ENDFILE: filePrint("EOF", ""); exit(0); break;
         default: fprintf(listing, "Unknown token: %d\n", token); /* should never happen */
     }
 }
TreeNode *newStmtNode(StmtKind kind) {
	TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
	int i;
	if(t == NULL)  fprintf(listing, "Out of memory error at line %d\n", lineno);
	else {
		for(i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = StmtK;
		t->kind.stmt = kind;
		t->lineno = lineno;
	}

	return t;
}

/* Function newExpNode creates a new expression node for syntax tree construction
 */

TreeNode *newExpNode(ExpKind kind) {
	TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
	int i;
	if(t == NULL)  fprintf(listing, "Out of memory error at line %d\n", lineno);
	else {
		for(i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = ExpK;
		t->kind.exp = kind;
		t->lineno = lineno;
		t->type = Void;
	}

	return t;
}

TreeNode *newDeclNode(DeclKind kind) {
	TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
	int i;
	if(t == NULL)	fprintf(listing, "Out of memory error at line %d\n", lineno);
	else {
		for(i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = DeclK;
		t->kind.exp = kind;
		t->lineno = lineno;
	}

	return t;
}

/* Function copyString allocates and makes a new copy of an existing string
 */

char *copyString(char *s) {
	int n;
	char *t;
	if(s == NULL) return NULL;
	n = strlen(s) + 1;
	t = malloc(n);
	if(t == NULL)  fprintf(listing, "Out of memory error at line %d\n", lineno);
	else strcpy(t, s);
	return t;
}

/* Variable indentno is used by printTree to store currnet number of spaces to indent
 */

static int indentno = 0;

/* macros to increase/decrease indentation */

#define INDENT indentno+=2
#define UNINDENT indentno-=2

/* printSpaces indents by printing spaces */

static void printSpaces(void) {
	int i;
	for(i = 0; i < indentno; i++)
		fprintf(listing, " ");
}

/* procedure printTree prints a syntax tree to the listing file using indentation to indicate subtrees
 */

void printTree(TreeNode *tree) {
	int i;
	INDENT;
	while(tree != NULL) {
		printSpaces();
		if(tree->nodekind == StmtK) {
			switch(tree->kind.stmt) {
				case IfK: fprintf(listing, "If\n"); break;
				case WhileK: fprintf(listing, "While\n"); break;
				case ReturnK: fprintf(listing, "Return\n"); break;
				default: fprintf(listing, "Unknown StmtNode kind\n"); break;
			}
		}

		else if(tree->nodekind == ExpK) {
			switch(tree->kind.exp) {
				case OpK:
					if(tree->attr.op == LE) fprintf(listing, "Op: <=\n");
					else if(tree->attr.op == ASSIGN) fprintf(listing, "Op: =\n");
					else if(tree->attr.op == LT) fprintf(listing, "Op: <\n");
					else if(tree->attr.op == GT) fprintf(listing, "Op: >\n");
					else if(tree->attr.op == GE) fprintf(listing, "Op: >=\n");
					else if(tree->attr.op == EQ) fprintf(listing, "Op: ==\n");
					else if(tree->attr.op == NE) fprintf(listing, "Op: !=\n");
					else if(tree->attr.op == PLUS)  fprintf(listing, "Op: +\n");
					else if(tree->attr.op == MINUS) fprintf(listing, "Op: -\n");
					else if(tree->attr.op == TIMES)	fprintf(listing, "Op: *\n");
					else if(tree->attr.op == OVER)	fprintf(listing, "Op: /\n");
					break;
				case ConstK: fprintf(listing, "const: %d\n", tree->attr.val); break;
				case IdK: fprintf(listing, "ID: %s\n", tree->attr.name); break;
				case CallK: fprintf(listing, "Call procedure = %s\n", tree->attr.name); break;
				default: fprintf(listing, "Unknown ExpNode kind\n"); break;
			}
		}

		else if(tree->nodekind == DeclK) {
			switch(tree->kind.decl) {
				case VarK:
					fprintf(listing, "ID: %s\n", tree->attr.name);
					printSpaces();
					if(tree->child[0]->type == Int)	fprintf(listing,"Type: int\n");
					else if(tree->child[0]->type == Void) fprintf(listing,"Type: void\n");
					else if(tree->child[0]->type == Array)
						fprintf(listing,"Type: array %d\n", tree->child[0]->array_size);
					break;
				case TypeK:
					if(tree->type == Int) fprintf(listing, "Type = Int\n");
					else if(tree->type==Void) fprintf(listing, "Type = Void\n");
					break;
				case FuncK:	fprintf(listing, "Function = %s\n", tree->attr.name); break;
				case ParamK: fprintf(listing, "Parameter = %s\n", tree->attr.name); break;
				case CmpndK: fprintf(listing, "Compound statement\n"); break;
				default: fprintf(listing, "Unknown DeclNode kind\n"); break;
			}
		}

		else fprintf(listing, "Unknown node kind\n");
		for(i = 0; i < MAXCHILDREN; i++) {
			if(tree->nodekind==DeclK && tree->kind.decl==VarK) break;
			printTree(tree->child[i]);
		}
		tree = tree->sibling;
	}
	UNINDENT;
}
