/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert(char *name, int lineno, int scope, int loc, int vpf, int array_size, int type, int param_num);

void line_insert(char *name, int scope, int lineno);

/* Function st_lookup returns the memory
 * location of a variable or -1 if not found
 */
int st_lookup(char *name, int scope);

int find_sym_type(char *name, int scope, int flag, int param_num);

int find_sym_param_num(char *name, int scope);

void st_delete(int scope);

/* Procedure printSymtab prints a formatted
 * listing of the symbol table contents
 * to the listing file
 */
void printSymtab(FILE * listing, int scope);

#endif
