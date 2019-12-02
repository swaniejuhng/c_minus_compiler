/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

/* counter for variable memory locations */
static int check_return;
static int check_void;
static int array_as_arg = 0;
static int param_loc = 0;
static int localvar_loc = -4;
static int func_loc = 0;
static int globalvar_loc = 0;
static int path = 0;
static int actual_param_num = -1;
static int formal_param_num = -1;
static TreeNode *temp;
int scope = 0;
int inflag = 0;

char error_msg_heap[256] = "";
char num_str[5] = "";

extern int g;
g = 0;
/* Procedure traverse is a generic recursive
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc
 * in postorder to tree pointed to by t
 */
static void traverse(TreeNode *t, void (* preProc)(TreeNode *), void (* postProc)(TreeNode *), int mode) {
    int new_scope = 0;
    int exit_scope = 0;
    static int func_decl_flag = 0;
    //static int count = 0;

    if(t != NULL) {
        preProc(t);

      { int i;
        if((path == 0) && (t->nodekind == DeclK) && (t->kind.stmt == CmpndK))
            ;

        if((t->nodekind == DeclK) && (t->kind.decl == FuncK) && (mode == 0)) {
            scope++;
            param_loc = 4 * formal_param_num; localvar_loc = -4;
            func_decl_flag = 1;
        }

        else if((t->nodekind == DeclK) && (t->kind.decl == CmpndK)) {
            if(func_decl_flag == 1) {
                func_decl_flag = 0;
                new_scope = 1;
            }
            else {
                scope++;
                new_scope = 1;
            }
        }

        for(i = 0; i < MAXCHILDREN; i++)
            traverse(t->child[i], preProc, postProc, mode);

      }
        if((t->nodekind == DeclK) && (t->kind.decl == CmpndK) && (mode == 0)) {
            printSymtab(listing, scope);
            scope--;
            exit_scope = 1;
        }

        if(path == 0) {
            if(exit_scope == 1) {
                postProc(t);
                exit_scope = 0;
            }
        }

        else postProc(t);

        traverse(t->sibling, preProc, postProc, mode);
    }
}

/* nullProc is a do-nothing procedure to
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode *t) {
    if(t == NULL) return;
    else return;
}

static void typeError(TreeNode *t, char *message, char *name) {
    fprintf(listing, "ERROR in line %d: %s (%s)\n", t->lineno, message, name);
    Error = TRUE;
}

/* Procedure insertNode inserts
 * identifiers stored in t into
 * the symbol table
 */
static void insertNode(TreeNode *t) {
    int lookup_scope;
    int actual_param_num = -1;
    int fomral_param_num = -1;
    TreeNode *temp;
    int ParamError = 0;
    switch(t->nodekind) {
        case StmtK:
            switch(t->kind.stmt) {
                case IfK:      break;
                case WhileK:   break;
                case ReturnK:  break;
                default: break;
            } break;

        case ExpK:
            switch(t->kind.exp) {
                case OpK:     break;
                case ConstK:  break;
                case IdK:
                    t->type = find_sym_type(t->attr.name, scope, 1, formal_param_num);
                    if(t->type == -1) {
                        strcat(error_msg_heap, "ERROR in line ");
                        memset(num_str, 0, 5); sprintf(num_str, "%d", t->lineno);
                        strcat(error_msg_heap, num_str);
                        strcat(error_msg_heap, ": undeclared variable ");
                        strcat(error_msg_heap, t->attr.name);
                        strcat(error_msg_heap, "\n");
                    }
                    else line_insert(t->attr.name, scope, t->lineno);
                    break;

                case CallK:
                    if(t->child[0] != NULL) {
                        actual_param_num = 1;
                        temp = t->child[0];
                        while(temp->sibling != NULL) {
                            if(temp->type == Void)
                                fprintf(listing, "ERROR in line %d: variable %s declared in void type\n", t->lineno, t->attr.name);
                            temp = temp->sibling;
                            actual_param_num++;
                        }
                    }
                    else actual_param_num = 0;
                    temp = t->child[0];
                    t->type = find_sym_type(t->attr.name, scope, 0, actual_param_num);
                    if(t->type == -1) {
                        strcat(error_msg_heap, "ERROR in line ");
                        memset(num_str, 0, 5); sprintf(num_str, "%d", t->lineno);
                        strcat(error_msg_heap, num_str);
                        strcat(error_msg_heap, ": undeclared function ");
                        strcat(error_msg_heap, t->attr.name);
                        strcat(error_msg_heap, "\n");
                    }
                    //formal_param_num = find_sym_param_num(t->attr.name, scope);
                    line_insert(t->attr.name, scope, t->lineno);
                    array_as_arg = 1;
                    break;
                default: break;
            } break;

        case DeclK:
            switch(t->kind.decl) {
                case VarK:
                    lookup_scope = st_lookup(t->attr.name, scope);
                    if(lookup_scope >= 0) {
                        strcat(error_msg_heap, "ERROR in line ");
                        memset(num_str, 0, 5); sprintf(num_str, "%d", t->lineno);
                        strcat(error_msg_heap, num_str);
                        strcat(error_msg_heap, ": variable ");
                        strcat(error_msg_heap, t->attr.name);
                        strcat(error_msg_heap, " already exists (first declared in line ");
                        memset(num_str, 0, 5); sprintf(num_str, "%d", t->lineno); //here!!!!!!!//
                        strcat(error_msg_heap, num_str);
                        strcat(error_msg_heap, ")\n");
                    }
                    else {
                        if(scope == 0) {
                            g++;
                            if(t->type == Int)          globalvar_loc += 4;
                            else if(t->type == Array)   globalvar_loc += (4 * t->array_size);
                            t->memloc = globalvar_loc;
                            st_insert(t->attr.name, t->lineno, scope, globalvar_loc, 0, t->array_size, t->type, actual_param_num);
                        }
                        else {
                            if(t->type == Int)          localvar_loc -= 4;
                            else if(t->type == Array)   localvar_loc -= (4 * t->array_size);
                            t->memloc = localvar_loc;
                            st_insert(t->attr.name, t->lineno, scope, localvar_loc, 0, t->array_size, t->type, actual_param_num);
                        }
                    }
                    break;

                case ParamK:
                    temp = t;
                    do {
                        lookup_scope = st_lookup(t->attr.name, scope);
                        if(lookup_scope < scope) {
                            t->memloc = param_loc;
                            st_insert(temp->attr.name, t->lineno, scope, param_loc, 1, 0, temp->type, actual_param_num);
                            param_loc -= 4;
                        }
                        temp = temp->sibling;
                    } while(temp != NULL);

                    if(ParamError == 1) {
                        strcat(error_msg_heap, "ERROR in line ");
                        memset(num_str, 0, 5); sprintf(num_str, "%d", t->lineno);
                        strcat(error_msg_heap, num_str);
                        strcat(error_msg_heap, ": parameter ");
                        strcat(error_msg_heap, t->attr.name);
                        strcat(error_msg_heap, " already exists");
                        strcat(error_msg_heap, "\n");
                        ParamError = 0;
                    }
                    break;

                case FuncK:
                    /*
                    if(strcmp(t->attr.name, "input") && strcmp(t->attr.name, "output")) {
                        if(t->child[1]->child[0] == NULL)  formal_param_num = 0;
                        else {
                            formal_param_num = 1;
                            temp = t->child[1]->child[0];
                            while(temp->sibling != NULL) {
                                temp = temp->sibling;
                                formal_param_num++;
                            }
                        }
                    }*/
                    lookup_scope = st_lookup(t->attr.name, scope);
                    if(lookup_scope >= 0) {
                        strcat(error_msg_heap, "ERROR in line ");
                        memset(num_str, 0, 5); sprintf(num_str, "%d", t->lineno);
                        strcat(error_msg_heap, num_str);
                        strcat(error_msg_heap, ": function ");
                        strcat(error_msg_heap, t->attr.name);
                        strcat(error_msg_heap, " already exists (first declared in line ");
                        memset(num_str, 0, 5); sprintf(num_str, "%d", t->lineno); //here!!!!!!!//
                        strcat(error_msg_heap, num_str);
                        strcat(error_msg_heap, ")\n");
                        ParamError = 0;
                    }
                    if(lookup_scope < scope) {
                        //t->memloc = func_loc++;
                        t->memloc = g++;
                        if(!strcmp(t->attr.name, "input"))
                            st_insert(t->attr.name, t->lineno, 0, t->memloc, 2, 0, Int, actual_param_num);
                        else if(!strcmp(t->attr.name, "output"))
                            st_insert(t->attr.name, t->lineno, 0, t->memloc, 2, 0, Void, actual_param_num);
                        else st_insert(t->attr.name, t->lineno, scope, t->memloc, 2, 0, t->child[0]->type, actual_param_num);
                    }
                    else {

                        st_insert(t->attr.name, t->lineno, scope, t->memloc, 2, t->child[0]->type, Int, actual_param_num);
                    }
                    break;

                case CmpndK :
                    if(!inflag) ;//location = moving_scope(location, 0);
                    else        inflag = 0;
                    //param_loc = 0;
                    break;
            }
        default: break;
    }
}

static void deleteSymtab(TreeNode *t) {
    st_delete(scope);
}

/* Function buildSymtab constructs the symbol
 * table by preorder traversal of the syntax tree
 */

void inout_func(TreeNode **syntaxTree) {
    TreeNode *input = newDeclNode(FuncK);
    input->sibling = *syntaxTree;
    *syntaxTree = input;
    input->lineno = 0;
    input->attr.name = malloc(sizeof(char) * 6);
    strcpy(input->attr.name, "input");
    input->type = Int;

    TreeNode *output = newDeclNode(FuncK);
    output->sibling = *syntaxTree;
    *syntaxTree = output;
    output->lineno = 0;
    output->attr.name = malloc(sizeof(char) * 7);
    strcpy(output->attr.name, "output");
    output->type = Void;

    TreeNode *param = newDeclNode(ParamK);
    param->type = Int;
    param->array_size = -1;
    param->attr.name = malloc(sizeof(char) * 4);
    strcpy(param->attr.name, "arg");
    param->lineno = 0;
    output->child[0] = param;
}

TreeNode *buildSymtab(TreeNode *syntaxTree) {
    inout_func(&syntaxTree);
    traverse(syntaxTree, insertNode, nullProc, 0);

    return syntaxTree;
/*
    if(TraceAnalyze) {
        fprintf(listing,"\nSymbol table:\n\n");
        printSymtab(listing);
    }
*/
    path++;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode *t) {
    switch(t->nodekind) {
        case ExpK:
            switch(t->kind.exp) {
                case OpK:
                    if(t->child[1] != NULL) {
                        if((((t->child[0]->kind.exp == IdK) || t->child[0]->kind.exp == CallK) && (t->child[0]->type == Void))
                        || (((t->child[1]->kind.exp == IdK) || t->child[1]->kind.exp == CallK) && (t->child[1]->type == Void))) {
                            typeError(t, "Op applied to non-integer", "");
                        }
                    }

                    if((t->attr.op == EQ) || (t->attr.op == LT) || (t->attr.op == LE)
                    || (t->attr.op == GT) || (t->attr.op == GE))
                        ;
                    else t->type = Int;
                    break;
                case ConstK:
                case IdK:
                    if(t->type == Int) {
                        if(t->child[0] != NULL)
                            fprintf(listing, "ERROR in line %d: variable %s is declared in int type \
                            thus it should not be accompanied by [index]\n", t->lineno, t->attr.name);
                    }
                    else if(t->type == Array) {
                        if(t->child[0] == NULL) {
                            if(!array_as_arg) fprintf(listing, "ERROR in line %d: variable %s is declared in array type \
                                                thus it should be accompanied by [index]\n", t->lineno, t->attr.name);
                            else array_as_arg = 0;
                        }
                    }
                    else if(t->type == VarError)
                        fprintf(listing, "ERROR in line %d: function %s is used like variable\n", t->lineno, t->attr.name);
                    //t->type = Int;
                    break;
                case CallK:
                    array_as_arg = 1;
                    if(t->type == FuncError)
                        fprintf(listing, "ERROR in line %d: variable %s is used like function\n", t->lineno, t->attr.name);

                    if(t->child[0] != NULL) {
                        actual_param_num = 1;
                        temp = t->child[0];
                        while(temp->sibling != NULL) {
                            if(temp->type == Void)
                                fprintf(listing, "ERROR in line %d: variable %s declared in void type\n", t->lineno, t->attr.name);
                            temp = temp->sibling;
                            actual_param_num++;
                        }
                    }
                    else actual_param_num = 0;

                    if(actual_param_num != formal_param_num) {
                        //fprintf(listing, "ERROR in line %d: the number of parameters of function %s do not match (%d, %d)\n", t->lineno, t->attr.name, actual_param_num, formal_param_num);
                        actual_param_num = formal_param_num = -1;
                    }
                    break;
                default: break;
            } break;
        case StmtK:
            switch(t->kind.stmt) {
                case ReturnK:
                    if(t->type == Void)
                        check_void = 1;
                    check_return = 1;
                    //temp = t;
                    break;

                default: break;
            } break;
        case DeclK:
            switch(t->kind.decl) {
                case FuncK:
                    if(t->child[1]->child[0] == NULL)   formal_param_num = 0;
                    else {
                        formal_param_num = 1;
                        temp = t->child[1]->child[0];
                        while(temp->sibling != NULL) {
                            temp = temp->sibling;
                            formal_param_num++;
                        }
                    }
                    if(check_return && t->child[0]->type == Void && !check_void)
                        typeError(temp, "If function type is void, return must not exist.", t->attr.name);
                    else if(check_return == 0 && t->child[0]->type == Int)
                        typeError(t, "If function type is int, return must exist.", t->attr.name);

                    if(!strcmp(t->attr.name, "main")) {
                        if(t->type == Int)
                            typeError(t, "Return value of main function must be void.", t->attr.name);
                        if(t->sibling != NULL)
                            typeError(t, "Main function must be located last.", t->attr.name);
                        if(t->child[1]->type != Void)
                            typeError(t, "Main function must not have parameters.", t->attr.name);
                    }
                    check_return = 0;
                    check_void = 0;
                    temp = NULL;
                    break;

                case VarK:
                    if(t->type == Void)
                        fprintf(listing, "ERROR in line %d: variable %s declared in void type\n", t->lineno, t->attr.name);
                    break;

                case ParamSeqK:
                    inflag = 1;
                    break;
                case ParamK:
                    //if((t->type != Void) && (t->child[0]->type == Void))
                    //    typeError(t, "Void is used by parameter type.", t->attr.name);
                    break;
                default: break;
            }
        default: break;
    }
}

/* Procedure typeCheck performs type checking
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode *syntaxTree) {
    fprintf(listing, "%s", error_msg_heap);
    traverse(syntaxTree, nullProc, checkNode, 1);
}
