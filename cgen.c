/****************************************************/
/*File: cgen.c                                     */
/*The code generator implementation                */
/*for the C- compiler                              */
/*(generates code for the TM machine)              */
/*Compiler Construction: Principles and Practice   */
/*Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"

/*tmpOffset is the memory offset for temps
   It is decremented each time a temp is
   stored, and incremeted when loaded again
*/
static int tmpOffset = 0;

extern int g;
static int ml;
static char buffer[1000];
static char *local_id[100];
static int local_id_index = 100;
static char *param_list[100];
static int param_index = 100;
static int param_num = 0;
static int temp;
static int is_in_func = FALSE;
static int arg_num = 0;
static int main_loc = 0;
static int func_num = 0;
static int nparam = 0;

/*prototype for internal recursive code generator */
static void cGen(TreeNode *tree);
static void genParam(TreeNode *tree);
int arg_post(int num, TreeNode *tree);
int flag = 0;

int local_offset_lookup(char *id) {
    int i;
    for(i = local_id_index; i < 100; i++) {
        if(local_id[i] != '\0' && !strcmp(local_id[i], id))
            return i - local_id_index;
    }

    return -1;
}

int param_offset_lookup(char *id) {
    int i;
    for(i = param_index; i < 100; i++) {
        if(param_list[i] != '\0' && !strcmp(param_list[i], id))
        return i - param_index;
    }
}

/*Procedure genStmt generates code at a statement node */
static void genStmt(TreeNode *tree) {
    TreeNode *p1, *p2, *p3;
    int savedLoc1, savedLoc2, currentLoc, loc, offset;

    switch(tree->kind.stmt) {
        case IfK:
            if(TraceCode) emitComment("-> if_stmt");
            p1 = tree->child[0];
            p2 = tree->child[1];
            p3 = tree->child[2];
            /*generate code for test expression */
            cGen(p1);
            savedLoc1 = emitSkip(1);
            emitComment("if_stmt: jump to else belongs here");
            /*recurse on then part */
            cGen(p2);
            savedLoc2 = emitSkip(1);
            emitComment("if_stmt: jump to end belongs here");
            currentLoc = emitSkip(0);
            emitBackup(savedLoc1);
            emitRM_Abs("JEQ", ac, currentLoc, "if_stmt: jump to else");
            emitRestore();
            /*recurse on else part */
            cGen(p3);
            currentLoc = emitSkip(0);
            emitBackup(savedLoc2);
            emitRM_Abs("LDA", pc, currentLoc, "jump to end");
            emitRestore();
            if(TraceCode)  emitComment("<- if_stmt");
            break; /*if_k */

        case WhileK:
            if(TraceCode) emitComment("-> while_stmt");
            p1 = tree->child[0];
            p2 = tree->child[1];
            savedLoc1 = emitSkip(0);
            emitComment("while_stmt: jump after body comes back here");
            /*generate code for body */
            cGen(p1);
            /*generate code for test */
            savedLoc2 = emitSkip(1);
            emitComment("while_stmt: jump to end belongs here");

            cGen(p2);
            emitRM("LDC", pc, savedLoc1, 0, "unconditional jump");
            currentLoc = emitSkip(0);
            emitBackup(savedLoc2);
            emitRM_Abs("JEQ", ac, currentLoc, "while_stmt: jump to end");
            emitRestore();
            if(TraceCode)  emitComment("<- while_stmt");
            break; /*repeat */

        case ReturnK:
            if(TraceCode) emitComment("-> return");
            if(tree->child[0]) cGen(tree->child[0]);
            if(TraceCode) emitComment("<- return");
            break;

        default:
            break;
    }
} /*genStmt */

/*Procedure genExp generates code at an expression node */
static void genExp(TreeNode *tree) {
    int loc;
    TreeNode *p1, *p2;
    int a = 0;
    int var_offset, base_reg;

    switch(tree->kind.exp) {
        case OpK:
            if(tree->attr.op == ASSIGN) {
                if(TraceCode) emitComment("-> assign");
                p1 = tree->child[0];
                p2 = tree->child[1];
                cGen(p2);

                if(tree->child[0]->type == Array) {
                    emitRM("ST", ac, --temp, mp, "push ac");
                    cGen(tree->child[0]->child[0]);
                    emitRM("LDA", ac1, 0, ac, "save index ac1");
                    loc = local_offset_lookup(tree->child[0]->attr.name);
                    if(loc == -1) {
                        loc = param_offset_lookup(tree->child[0]->attr.name);
                        if(loc == -1) {
                            loc = (tree->child[0]->memloc / 4) + 1;
                            emitRM("LDA", ac, loc, gp, "load address");
                            emitRO("ADD", ac1, ac, ac1, "address + index");
                        }
                        else {
                            emitRM("LD", ac, loc + 1, fp, "load address");
                            emitRO("ADD", ac1, ac, ac1, "address + index");
                        }
                    }
                    else {
                        emitRM("LDA", ac, loc, mp, "load address");
                        emitRO("ADD", ac1, ac, ac1, "address + index");
                    }
                    emitRM("LD", ac, temp++, mp, "op: load ac");
                    emitRM("ST", ac, 0, ac1, "address + index");
                }

                else if(tree->child[0]->type == Int) {
                    loc = local_offset_lookup(tree->child[0]->attr.name);
                    if(loc == -1) {
                        loc = local_offset_lookup(tree->child[0]->attr.name);
                        if(loc == -1) {
                            loc = tree->child[0]->memloc / 4;
                            emitRM("ST", ac, loc + 1, gp, "assign: store value");
                        }
                        else emitRM("ST", ac, loc + 1, mp, "assign: store value");
                    }
                    else emitRM("ST", ac, loc, mp, "assign: store value");
                }
                if(TraceCode) emitComment("<- assign");
            }

            else {
                if(TraceCode) emitComment("-> Op");
                p1 = tree->child[0];
                p2 = tree->child[1];
                /*gen code for ac = left arg */
                cGen(p1);
                /*gen code to push left operand */
                emitRM("ST", ac, --temp, mp, "op: push left");
                /*gen code for ac = right operand */
                cGen(p2);
                /*now load left operand */
                emitRM("LD", ac1, temp++, mp, "op: load left");
                switch(tree->attr.op) {
                    case PLUS:  emitRO("ADD", ac, ac1, ac, "op +(ADD)"); break;
                    case MINUS: emitRO("SUB", ac, ac1, ac, "op -(SUB)"); break;
                    case TIMES: emitRO("MUL", ac, ac1, ac, "op *(MUL)"); break;
                    case OVER:  emitRO("DIV", ac, ac1, ac, "op /(DIV)"); break;
                    case LT:
                        emitRO("SUB", ac, ac1, ac, "op <(LT)");
                        emitRM("JLT", ac, 2, pc, "br if true");
                        emitRM("LDC", ac, 0, ac, "false case");
                        emitRM("LDA", pc, 1, pc, "unconditional jump");
                        emitRM("LDC", ac, 1, ac, "true case");
                        break;
                    case LE:
                        emitRO("SUB", ac, ac1, ac, "op <=(LE)");
                        emitRM("JLT", ac, 2, pc, "br if true");
                        emitRM("LDC", ac, 0, ac, "false case");
                        emitRM("LDA", pc, 1, pc, "unconditional jump");
                        emitRM("LDC", ac, 1, ac, "true case");
                        break;
                    case GT:
                        emitRO("SUB", ac, ac1, ac, "op >(GT)");
                        emitRM("JLT", ac, 2, pc, "br if true");
                        emitRM("LDC", ac, 0, ac, "false case");
                        emitRM("LDA", pc, 1, pc, "unconditional jump");
                        emitRM("LDC", ac, 1, ac, "true case");
                        break;
                    case GE:
                        emitRO("SUB", ac, ac1, ac, "op >=(GE)");
                        emitRM("JLT", ac, 2, pc, "br if true");
                        emitRM("LDC", ac, 0, ac, "false case");
                        emitRM("LDA", pc, 1, pc, "unconditional jump");
                        emitRM("LDC", ac, 1, ac, "true case");
                        break;
                    case EQ:
                        emitRO("SUB", ac, ac1, ac, "op ==(EQ)");
                        emitRM("JEQ", ac, 2, pc, "br if true");
                        emitRM("LDC", ac, 0, ac, "false case");
                        emitRM("LDA", pc, 1, pc, "unconditional jump");
                        emitRM("LDC", ac, 1, ac, "true case");
                        break;
                    case NE:
                        emitRO("SUB", ac, ac1, ac, "op !=(NE)");
                        emitRM("JLT", ac, 2, pc, "br if true");
                        emitRM("LDC", ac, 0, ac, "false case");
                        emitRM("LDA", pc, 1, pc, "unconditional jump");
                        emitRM("LDC", ac, 1, ac, "true case");
                        break;
                    default:
                        emitComment("BUG: Unknown operator");
                        break;
                } /*case op */
                if(TraceCode)  emitComment("<- Op");
            }
            break; /*OpK */

        case ConstK:
            if(TraceCode) emitComment("-> Const");
            /*gen code to load integer constant using LDC */
            emitRM("LDC", ac, tree->attr.val, 0, "load constant");
            if(TraceCode)  emitComment("<- Const");
            break; /*ConstK */

        case IdK:
            if(TraceCode) {
                sprintf(buffer, "-> Id (%s)", tree->attr.name);
                emitComment(buffer);
            }
            if(tree->type == Array) emitRM("LDC", ac1, 0, 0, "");
            loc = local_offset_lookup(tree->attr.name);
            if(loc == -1) {
                loc = param_offset_lookup(tree->attr.name);
                if(loc == -1) {
                    loc = (tree->memloc / 4) + 1;
                    if(tree->type == Array) emitRM("LDA", ac, loc, gp, "id: load address of array variable to ac");
                    else emitRM("LD", ac, loc, gp, "id: load value of variable to ac");
                }
                else emitRM("LD", ac, loc + 1, fp, "id: load value of variable to ac");
            }
            else {
                if(tree->type == Array) emitRM("LDA", ac, loc, mp, "id: load address of array variable to ac");
                else emitRM("LD", ac, loc, mp, "id: load value of variable to ac");
            }

            if(tree->type == Array) {
                if(tree->child[0]) {
                    emitRM("ST", ac, --temp, mp, "push ac");
                    cGen(tree->child[0]);
                    emitRM("LDA", ac1, 0, ac, "save array size to ac1");
                    emitRM("LD", ac, temp++, mp, "load ac");
                    emitRO("ADD", ac1, ac1, ac, "get array location");
                    emitRM("LD", ac, 0, ac1, "get value");
                }
            }
            sprintf(buffer, "<- Id (%s)", tree->attr.name);
            if(TraceCode) emitComment(buffer);
            break; /*IdK */

        case CallK:
            sprintf(buffer, "->call function %s", tree->attr.name);
            if(TraceCode) emitComment(buffer);
            p1 = tree->child[0];
            a = 0;
            arg_num = arg_post(0, p1);
            emitRM("LDA", mp, -arg_num, mp, "increase stack after pushing arguments");
            temp = 0;
            if(TraceCode) emitComment("arguments are pushed");
            emitRM("LDC", ac1, 1, 0, "");
            emitRO("SUB", mp, mp, ac1, "mp = mp - 1");
            emitRM("ST", fp, 0, mp, "push fp");
            emitRM("LDA", fp, 0, mp, "copy sp to fp");
            emitRO("SUB", mp, mp, ac1, "mp = mp - 1");
            emitRM("LDC", ac1, 2, 0, "");
            emitRO("ADD", ac1, ac1, pc, "calculate return address");
            emitRM("ST", ac1, 0, mp, "push return address");
            loc = tree->memloc;
            emitRM("LD", pc, loc, gp, "jump to function");
            if(arg_num > 0) {
                emitRM("LDC", ac1, arg_num, 0, "");
                emitRO("ADD", mp, mp, ac1, "pop arguments");
            }
            sprintf(buffer, "<-call function %s", tree->attr.name);
            if(TraceCode) emitComment(buffer);
            break;

        default:
            break;
    }
} /*genExp */

static void genDecl(TreeNode *tree) {
    TreeNode *p1, *p2, *p3;
    int savedLoc1, loc, size, mem, tempp = 0, a;
    char *tparam[8];
    switch(tree->kind.decl) {
        case CmpndK:
            if(TraceCode) emitComment("-> compound");
            p1 = tree->child[0];
            p2 = tree->child[1];
            p3 = tree->child[2];
            tempp = 0;
            while(p1) {
                if((p1->nodekind == DeclK) && (p1->kind.decl == VarK)) {
                     if(p1->type == Int) {
                         tempp++;
                         local_id[--local_id_index] = p1->attr.name;
                     }
                     else if(p1->type == Array) {
                         tempp += p1->array_size;
                         local_id_index = p1->array_size;
                         local_id[local_id_index] = p1->attr.name;
                     }
                }
                p1 = p1->sibling;
            }
            emitRM("LDC", ac1, tempp, 0, "ac1 = size of the local variable");
            emitRO("SUB", mp, mp, ac1, "allocate memory for local variable");
            cGen(p2);
            emitRM("LDC", ac1, tempp, 0, "ac1 = local var size");
            emitRO("ADD", mp, mp, ac1, "free");
            local_id_index += tempp;
            if(TraceCode) emitComment("<- coumpound");
            break;

        case FuncK:
            if(TraceCode) {
                sprintf(buffer, "-> Function (%s)", tree->attr.name);
                emitComment(buffer);
            }
            savedLoc1 = emitSkip(0);
            emitBackup(func_num);
            func_num += 2;
            mem = tree->memloc;
            emitRM("LDC", ac, savedLoc1, 0, "load location of the function");
            emitRM("ST", ac, mem, gp, "add into memory");
            emitRestore();
            if(!strcmp(tree->attr.name, "main")) {
                main_loc = savedLoc1;
                ml = tree->memloc;
            }

            if(!strcmp(tree->attr.name, "input")) emitRO("IN", ac, 0, 0, "read integer value");
            else if(!strcmp(tree->attr.name, "output")) {
                emitRM("LD", ac, 1, fp, "load first argument");
                emitRO("OUT", ac, 0, 0, "write ac");
            }
            else {
                nparam = 0;
                cGen(tree->child[1]);
                param_index -= nparam;
                cGen(tree->child[2]);
            }
            emitRM("LDA", mp, 0, fp, "copy fp to sp");
            emitRM("LD", fp, 0, mp, "pop fp");
            emitRM("LDC", ac1, 1, 0, "");
            emitRO("ADD", mp, mp, ac1, "mp = mp + 1");
            if(strcmp(tree->attr.name, "main")) emitRM("LD", pc, -2, mp, "jump to return address");

            if(TraceCode) {
                sprintf(buffer, "-> Function (%s)", tree->attr.name);
                emitComment(buffer);
            }
            break;

        case VarK:
            if(TraceCode) emitComment("-> var decl");
            if(tree->type == Array) size = tree->array_size;
            else size = 1;
            if(TraceCode) emitComment("-> var decl");
            break;

        case ParamK:
            if(TraceCode) emitComment("-> param");
            emitComment(tree->attr.name);
            p1 = tree;
            while(p1) {
                ++nparam;
                a = p1->memloc / 4;
                param_list[param_index - a] = p1->attr.name;
                p1 = p1->sibling;
            }
            if(TraceCode) emitComment("<- param");
            break;

        default:
            break;
    }
}

/*Procedure cGen recursively generates code by
 *tree traversal
 */
static void cGen(TreeNode *tree) {
    if(tree != NULL) {
        switch(tree->nodekind) {
            case StmtK:
                genStmt(tree);
                break;

            case ExpK:
                genExp(tree);
                break;

            case DeclK:
                genDecl(tree);
                break;

            default:
                break;
        }
        cGen(tree->sibling);
    }
}

/**********************************************/
/*the primary function of the code generator */
/**********************************************/
/*Procedure codeGen generates code to a code
 *file by traversal of the syntax tree. The
 *second parameter(codefile) is the file name
 *of the code file, and is used to print the
 *file name as a comment in the code file
 */
void codeGen(TreeNode *syntaxTree, char *codefile) {
    char *s = malloc(strlen(codefile) + 7);
    strcpy(s, "File: ");
    strcat(s, codefile);
    emitComment("TINY Compilation to TM Code");
    emitComment(s);
    /*generate standard prelude */
    emitComment("Standard prelude:");
    emitRM("LD", mp, 0, ac, "load maxaddress from location 0");
    emitRM("ST", ac, 0, ac, "clear location 0");
    emitComment("End of standard prelude.");
    func_num = emitSkip(g * 2 + 1);
    /*generate code for TINY program */
    cGen(syntaxTree);
    emitBackup(func_num);
    emitRM("LDC", pc, main_loc, 0, "main");
    emitRestore();
    /*finish */
    emitComment("End of execution.");
    emitRO("HALT", 0, 0, 0, "");
}

int arg_post(int num, TreeNode *tree) {
    TreeNode *t[100];
    int i;
    TreeNode *p = tree;
    if(!p) return num;
    while(p) {
        t[num++] = p;
        p = p->sibling;
    }

    for(i = num - 1; i >= 0; i--) {
        genExp(t[i]);
        emitRM("ST", ac, --temp, mp, "push args");
    }
    return num;
}
