/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "symtab.h"

/* SIZE is the size of the hash table */
#define SIZE 211
#define MAXSCOPE 10

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* the hash function */
static int hash(char *key) {
    int temp = 0;
    int i = 0;
    while(key[i] != '\0') {
        temp = ((temp << SHIFT) + key[i]) % SIZE;
        ++i;
    }
    return temp;
}

/* the list of line numbers of the source
 * code in which a variable is referenced
 */
typedef struct LineListRec {
    int lineno;
    struct LineListRec *next;
} *LineList;

/* The record in the bucket lists for
 * each variable, including name,
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */
typedef struct BucketListRec {
    char *name;
    LineList lines;
    int scope;
    int vpf;
    int type;
    int param_num;
    int array_size;
    int memloc ; /* memory location for variable */
    struct BucketListRec *next;
} *BucketList;

static BucketList hashTable[SIZE][MAXSCOPE];

/* the hash table */
//static BucketList hashTable[SIZE];

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */

void st_insert(char *name, int lineno, int scope, int loc, int vpf, int array_size, int type, int param_num) {
    int h = hash(name);
    BucketList l = hashTable[h][scope];

    while((l != NULL) && (!strcmp(name, l->name)))
        l = l->next;

    if(l == NULL) { /* variable not yet in table */
        l = hashTable[h][scope];
        l = (BucketList)malloc(sizeof(struct BucketListRec));
        l->name = name;
        l->lines = (LineList)malloc(sizeof(struct LineListRec));
        l->lines->lineno = lineno;
        l->memloc = loc;
        l->vpf = vpf;
        l->type = type;
        l->array_size = array_size;
        l->param_num = param_num;
        l->scope = scope;
        l->lines->next = NULL;
        l->next = hashTable[h][scope];
        hashTable[h][scope] = l;
    }

    else { /* found in table, so just add line number */
        LineList t = l->lines;
        while(t->next != NULL) t = t->next;
        t->next = (LineList)malloc(sizeof(struct LineListRec));
        t->next->lineno = lineno;
        t->next->next = NULL;
    }
} /* st_insert */

void line_insert(char *name, int scope, int lineno) {
    int j, found = 0;
    int h = hash(name);
    BucketList l;

    for(j = scope; j >= 0; j--) {
        l = hashTable[h][j];
        while(1) {
            if(l == NULL) break;
            else if(!strcmp(name, l->name)) { found = 1; break; }
            else l = l->next;
        }
        if(found) break;
    }

    if(found) {
        LineList t = l->lines;
        while((t->next != NULL) && (t->lineno != lineno)) t = t->next;
        if(t->lineno == lineno) return;
        t->next = (LineList)malloc(sizeof(struct LineListRec));
        t->next->lineno = lineno;
        t->next->next = NULL;
    }

    else printf("ERROR in line %d: undeclared variable %s\n", lineno, name);
}

/* Function st_lookup returns the memory
 * location of a variable or -1 if not found
 */
int st_lookup(char *name, int scope) {
    int i, flag = 0;
    int h = hash(name);
    BucketList l;
    for(i = scope; i >= 0; i--) {
        l = hashTable[h][i];
        while(1) {
            if((l != NULL) && (strcmp(name, l->name) != 0))
                l = l->next;
            else { flag = 1; break; }
        }
        if(flag == 1)   break;
    }

    if(l == NULL) return -1;
    else return l->memloc;
}

int find_sym_type(char *name, int scope, int flag, int param_num) {
    int i, found = 0;
    int h = hash(name);
    BucketList l;

    for(i = scope; i >= 0; i--) {
        l = hashTable[h][i];
        while(1) {
            if((l != NULL) && (strcmp(name, l->name) != 0))
                l = l->next;
            else if(l == NULL) break;
            else { found = 1; break; }
        }
        if(found == 1)   break;
    }

    if(l != NULL) {
        // CallK
        //if((flag == 1) && (param_num < 0))       return FuncError;
        // VarK
        //else if((flag == 0) && (param_num >= 0)) return VarError;
        return l->type;
    }
    else return -1;
}

int find_sym_param_num(char *name, int scope) {
    int i, found = 0;
    int h = hash(name);
    BucketList l;

    for(i = scope; i >= 0; i--) {
        l = hashTable[h][i];
        while(1) {
            if((l != NULL) && (strcmp(name, l->name) != 0))
                l = l->next;
            else if(l == NULL) break;
            else { found = 1; break; }
        }
        if(found == 1)   break;
    }

    if(l != NULL)   return l->type;
    else return -1;
}


void st_delete(int scope) {
    int i, j;

    for(i = 0; i < SIZE; i++)
        hashTable[i][scope] = NULL;
}

/* Procedure printSymtab prints a formatted
 * listing of the symbol table contents
 * to the listing file
 */
void printSymtab(FILE *listing, int scope) {
    int i, j;
    BucketList l, temp;
    fprintf(listing, "\n");
    fprintf(listing, "Name  Scope   Loc   V/P/F   Array?   ArrSize   Type   Line Numbers\n");
	fprintf(listing, "------------------------------------------------------------------\n");

    for(i = 0; i < SIZE; i++) {
        l = hashTable[i][scope];
        temp = l;
        while(l != NULL) {
            LineList t = l->lines;
            fprintf(listing, "%-6s ", l->name);
            fprintf(listing, " %-3d  ", l->scope);
            fprintf(listing, " %-5d  ", l->memloc);

            switch(l->vpf) {
                case 0 : fprintf(listing, "%-7s", "Var"); break;
                case 1 : fprintf(listing, "%-7s", "Par  "); break;
                case 2 : fprintf(listing, "%-7s", "Func"); break;
                default: fprintf(listing, "%-7s", "");
            }

            if(l->type == Array) {
                fprintf(listing, "Array  ");
                fprintf(listing, "  %-4d  ", l->array_size);
            }
            else {
                fprintf(listing, "No     ");
                fprintf(listing, "  -     ");
            }

            if(l->type == Void)         fprintf(listing, "    %-6s", "void");
            else if(l->type == Int)     fprintf(listing, "    %-6s", "int");
            else if(l->type == Array)   fprintf(listing, "    %-6s", "array");
            else                        fprintf(listing, "    %-6s", "");

            while(t != NULL) {
                fprintf(listing, "%4d ", t->lineno);
                t = t->next;
            }

            fprintf(listing, "\n");
            l = l->next;
        }
        hashTable[i][scope] = NULL;
    }
    fprintf(listing, "\n");
} /* printSymtab */
