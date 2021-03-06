/****************************************************

 File: parse.h                               
 A compiler for the C-Minus language

 MUST CS106 2016 Fall
 Programming designed by the teacher: Liang, Zhiyao
***************************************************/

#ifndef _PARSE_H_
#define _PARSE_H_


#include "scan.h"


typedef enum {DCL_ND, PARAM_ND, STMT_ND, EXPR_ND} NodeKind;
typedef enum {VAR_DCL, ARRAY_DCL, FUN_DCL} DclKind;
typedef enum {VAR_PARAM, ARRAY_PARAM, VOID_PARAM} ParamKind;
//No Need for ArgKind, since every argument is just an expression, 
//typedef enum {VAR_ARG, ARRAY_ARG} ArgKind;
//typedef enum {SLCT_STMT, ITER_STMT, EXPR_STMT, CMPD_STMT, RTN_STMT, NULL_STMT} StmtKind;
// On 11152013 replace ITER_STMT, with WHILE_STMT and DO_WHILE_STMT
// 12/nov/2014 add FOR_STMT
typedef enum {SLCT_STMT, WHILE_STMT, EXPR_STMT, CMPD_STMT, RTN_STMT, NULL_STMT, ASSIGN_STMT, FUNC_STMT, DCL_STMT} StmtKind;
/*Although assign  = is  an operator of C- according to our common sense, and it is true that according to its grammar, ( x = 3)*5 can appear in C-, which means x =3 has a return value 3, = is not treated as an operator according to the TINY grammar.  So in 2011-2013, it is considered necessary to to use Asn_EXPR
Now, in 12/nov/2014, assignment is back to the common sense, it is an operator. , so ASN_EXPR is removed. 
 */
// Distuiguish ID_EXPR and Array_EXPR
//typedef enum {OP_EXPR, CONST_EXPR, ID_EXPR, ARRAY_EXPR, CALL_EXPR,  ASN_EXPR} ExprKind;
// Since for loop is used, the comma expression is considered, since without it for loop cannot do much. Put it as  bonus for student. 
/* remove COMMA_EXPR, since commar is an operator */
/* remove array_expr, since [] is introduced as an operator */
typedef enum {OP_EXPR, CONST_EXPR, ID_EXPR, /* ARRAY_EXPR, */ CALL_EXPR} ExprKind;

/* The type of the value of an expression
   ExprType is used for type checking 
*/
/* No need for Boolean type, since C- uses integer as
   boolean valus */

typedef enum {VOID_TYPE, NUM_TYPE, INT_TYPE, ADDR_TYPE} ExprType;

#define MAX_CHILDREN 4


/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

typedef struct treeNode {
  struct treeNode * child[MAX_CHILDREN];
  struct treeNode * lSibling;
  struct treeNode * rSibling;
  // sibling is useful for declaration_list, param_list, local_declarations, statement_list, arg_list.
  struct treeNode * parent;
  /* parent is useful to check the containing structure of a node during parsing. So, the connected tree nodes can be found in all directions, up (to parents), down (to children), and horizontally (left and right to siblings).  */
  /* LineNum:  At the momemt in parsing, when this treeNode is constructed, what is the line number of the token being handled. */
  int lineNum; 
  NodeKind nodeKind;
  union {DclKind dcl; ParamKind param; StmtKind stmt; ExprKind expr;} kind;
  union{
    union {
      TokenType op; // used by Op_EXPR
      int val;      // used by Const_EXPR,
      const char * name;  // used by ID_EXPR, Call_EXPR, Array_EXPR
    } exprAttr;
    struct { // it is a struct, not union, because an array declaration need all the three fields.
      ExprType type; // used by all dcl and param
      const char * name;  // used by all dcl and param
      int size;     // used by array declaration
      /* size is only used for and array declaration; i.e., when  Dcl_Kind is Array_DCL. The requirement that size must be a constant should be checked by semantic analyzer.   For parameters, for Array_PARAM, size is ignored. For array element argument, the index is a child of the node, and should not be considered as dclAttr. */
    } dclAttr; // for declaration and parameters.
  }attr;
  ExprType type;
  /* type is for type-checking of exps, will be updated by type-checker,  the parser does not touch it.  */
  void * something; //can carry something possibly useful for other tasks of compiling
} TreeNode;


typedef struct parser Parser;

/* Each function has a parameter p, that is a pointer to the parser itself, in order to use the resources belong to the parser */
typedef struct parser{
	TreeNode * (* parse)(Parser * p); /* returning a parse tree, based on the tokenList that the parser knows */
	void (* set_token_list)(Parser * p, TokenList tokenList); /* let the parser remember some tokenList */
	void (* print_tree)(Parser * p,  TreeNode * tree); /* can print some parser tree */
	void (* free_tree)(Parser *p, TreeNode * tree); /* free the space of a parse tree */
	void * info; /* Some data belonging to this parser object. It can contain the tokenList that the parser knows. */
} Parser;




/*
extern TreeNode * syntaxTree;

extern TokenNode * thisTokenNode;
*/

#endif
