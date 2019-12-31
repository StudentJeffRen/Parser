//
//  parse.c
//  parse
//
//  Created by JeffRen on 2019/12/25.
//  Copyright © 2019 JeffRen. All rights reserved.
//

#include "util.h"
#include "parse.h"
#include "parse_print.h"
#include "tokenListIO.h"

FILE* listing;
int lineno;
int Error;

static TokenNode* node;

void printToken(TokenType token, const char* tokenString) {
    switch (token) {
        case IF:
        case NUM:
        case ELSE:
        case RETURN:
        case VOID:
        case WHILE:
            fprintf(listing,
                    "reserved word: %s\n", tokenString);
            break;
        case ASSIGN: fprintf(listing,"=\n"); break;
        case LT: fprintf(listing,"<\n"); break;
        case LTE: fprintf(listing,"<=\n"); break;
        case GT: fprintf(listing,">\n"); break;
        case GTE: fprintf(listing,">=\n"); break;
        case EQ: fprintf(listing,"==\n"); break;
        case NEQ: fprintf(listing,"!=\n"); break;
        case LPAR: fprintf(listing,"(\n"); break;
        case RPAR: fprintf(listing,")\n"); break;
        case LBR: fprintf(listing,"[\n"); break;
        case RBR: fprintf(listing,"]\n"); break;
        case LCUR: fprintf(listing,"{\n"); break;
        case RCUR: fprintf(listing,"}\n"); break;
        case SEMI: fprintf(listing,";\n"); break;
        case COMMA: fprintf(listing,",\n"); break;
        case PLUS: fprintf(listing,"+\n"); break;
        case MINUS: fprintf(listing,"-\n"); break;
        case STAR: fprintf(listing,"*\n"); break;
        case OVER: fprintf(listing,"/\n"); break;
        case MOD: fprintf(listing,"%%\n"); break;
        case EOP: fprintf(listing,"EOF\n"); break;
        case ARROW: fprintf(listing,"-->\n"); break;
        case SMILE: fprintf(listing,":)\n"); break;
        case ENTER: fprintf(listing,"ENTER\n"); break;
        case NONE: fprintf(listing,"NONE\n"); break;
        case NUMBER:
            fprintf(listing,
                    "NUMBER, val= %s\n",tokenString);
            break;
        case ID:
            fprintf(listing,
                    "ID, name= %s\n",tokenString);
            break;
        case STRING:
            fprintf(listing,
                    "STRING, val= %s\n",tokenString);
            break;
        case COMMENT:
            fprintf(listing,
                    "COMMENT, val= %s\n",tokenString);
            break;
        case ERROR:
            fprintf(listing,
                    "ERROR: %s\n",tokenString);
            break;
        default: /* should never happen */
            fprintf(listing,"Unknown token: %d\n",token);
    }
}

static void syntaxError(char * message) {
    fprintf(listing, "\n>>> ");
    fprintf(listing, "Syntax error at line %d: %s", lineno, message);
    Error = TRUE;
}

void next() {
    node = node->next;
}

static TreeNode * stmt_sequence(void);
static TreeNode * statement(void);
static TreeNode * para_list(void);
static TreeNode * arg_list(void);
static TreeNode * declare_stmt(void);
static TreeNode * func_stmt(void);
static TreeNode * func_dcl(void);
static TreeNode * param_dcl(void);
static TreeNode * var_dcl(void);
static TreeNode * if_stmt(void);
static TreeNode * while_stmt(void);
//static TreeNode * expression_stmt(void);
static TreeNode * compound_stmt(void);
static TreeNode * assign_stmt(void);
static TreeNode * assign(void);
static TreeNode * return_stmt(void);
static TreeNode * expression(void);
static TreeNode * simple_exp(void);
static TreeNode * call_exp(void);
static TreeNode * term(void);
static TreeNode * factor(void);

char * copyString(const char * s) {
    int n;
    char * t;
    if(s == NULL)
        return NULL;
    n = strlen(s) + 1;
    t = malloc(n);
    if(t == NULL)
        fprintf(listing,"Out of memory error at line %d\n", lineno);
    else
        strcpy(t,s);
    
    return t;
}

static void match(TokenType expected) {
    if(node->token->type == expected) {
        next();
    } else {
        syntaxError("unexpected token -> ");
        printToken(node->token->type, node->token->string);
        fprintf(listing,"      ");
    }
}

TreeNode * newStmtNode(StmtKind kind) {
    TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else {
        for (i = 0; i < MAX_CHILDREN; i++)
            t->child[i] = NULL;
        t->rSibling = NULL;
        t->lSibling = NULL;
        t->nodeKind = STMT_ND;
        t->kind.stmt = kind;
        t->lineNum = lineno;
    }
    return t;
}

TreeNode * newExpNode(ExprKind kind) {
    TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
    int i;
    if (t==NULL)
        fprintf(listing,"Out of memory error at line %d\n",lineno);
    else {
        for (i = 0;i < MAX_CHILDREN; i++)
            t->child[i] = NULL;
        t->rSibling = NULL;
        t->lSibling = NULL;
        t->nodeKind = EXPR_ND;
        t->kind.expr = kind;
        t->lineNum = lineno;
        t->type = VOID_TYPE;
    }
    return t;
}

ExprType tokenType_to_expr(TokenType token) {
    if(token == NUM) {
        return NUM_TYPE;
    } else {
        return VOID_TYPE;
    }
}

/* stmt_sequence -> stmt_sequence statement | statment */
TreeNode * stmt_sequence(void) {
    TreeNode * t = statement();
    TreeNode * p = t;
    while ((node->token->type != EOP) && (node->token->type != ELSE)
           && (node->token->type != SMILE)) {
        TreeNode * q;
        if((node->prev->token->type != ENTER) && (node->token->type != ENTER))
            match(SEMI);
        q = statement();
        if (q != NULL) {
            if (t == NULL)
                t = p = q;
            else /* now p cannot be NULL either */
            {
                p->rSibling = q;
                p = q;
            }
        }
    }
    return t;
}

/* statement -> if_stmt | while_stmt | assign_stmt | compound_stmt | declare_stmt | ENTER | LBR | return_stmt */
TreeNode* statement(void) {
    TreeNode * t = NULL;
    
    switch (node->token->type) {
        case IF : t = if_stmt(); break;
        case WHILE : t = while_stmt(); break;
        case ID : t = assign_stmt(); break;
        case LCUR: t = compound_stmt(); break;
        case NUM:
        case VOID:
            t = declare_stmt();
            break;
        case ENTER: match(ENTER); break;
        case LBR: match(LBR); break;
        case RETURN: t = return_stmt(); break;
        default : syntaxError("unexpected token -> ");
            printToken(node->token->type, node->token->string);
            next();
            break;
    } /* end case */
    
    return t;
}

/* declare_stmt -> func_dcl | var_dcl */
TreeNode* declare_stmt() {
    TreeNode* t = NULL;
    
    // Function return type NUM or NUM*
    if((node->next->next->token->type == LPAR) || (node->next->next->next->token->type == LPAR)) {
        t = func_dcl();
    } else {
        t = var_dcl();
    }
    
    return t;
}

/* func_dcl -> type-specifier ID ( para-list ) func-stmt */
TreeNode* func_dcl() {
    TreeNode* t = newStmtNode(DCL_STMT);
    
    t->nodeKind = DCL_ND;
    t->kind.dcl = FUN_DCL;
    if(node->next->token->type == STAR) {
        t->attr.dclAttr.type = ADDR_TYPE;
    } else {
        t->attr.dclAttr.type = tokenType_to_expr(node->token->type);
    }
    
    match(node->token->type);
    
    t->attr.dclAttr.name = copyString(node->token->string);
    match(ID);
    
    t->child[0] = para_list();
    t->child[1] = func_stmt();
    
    return t;
}

/* func_stmt -> ARROW stmt_sequence SMILE */
TreeNode* func_stmt() {
    TreeNode* t = newStmtNode(FUNC_STMT);
    
    match(ARROW);
    t->child[0] = stmt_sequence();
    match(SMILE);
    
    return t;
}

/* para-list -> para-list, param_dcl | param_dcl */
TreeNode* para_list() {
    TreeNode* t = NULL;
    
    match(LPAR);
    if(node->token->type != VOID) {
        t = param_dcl();
        TreeNode* p = t;
        while(node->token->type != RPAR) {
            TreeNode* q;
            match(COMMA);
            q = param_dcl();
            if (q != NULL) {
                if (t == NULL)
                    t = p = q;
                else /* now p cannot be NULL either */
                {
                    p->rSibling = q;
                    p = q;
                }
            }
        }
    } else {
        TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
        int i;
        if (t == NULL)
            fprintf(listing, "Out of memory error at line %d\n", lineno);
        else {
            for (i = 0; i < MAX_CHILDREN; i++)
                t->child[i] = NULL;
            t->rSibling = NULL;
            t->lSibling = NULL;
            t->nodeKind = PARAM_ND;
            t->lineNum = lineno;
        }
        
        t->attr.dclAttr.type = VOID_TYPE;
        t->kind.param = VOID_PARAM;
        match(VOID);
        
    }
    match(RPAR);
    
    return t;
}

/* param_dcl -> type-specifier ID | type-specifier ID [ ] | type-specifier * ID */
TreeNode* param_dcl() {
    TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else {
        for (i = 0; i < MAX_CHILDREN; i++)
            t->child[i] = NULL;
        t->rSibling = NULL;
        t->lSibling = NULL;
        t->nodeKind = PARAM_ND;
        t->lineNum = lineno;
    }
    
    if(node->next->token->type == STAR || node->next->next->token->type == LBR) {
        t->attr.dclAttr.type = ADDR_TYPE;
        t->kind.param = ARRAY_PARAM;
    } else {
        t->attr.dclAttr.type = tokenType_to_expr(node->token->type);
        t->kind.param = VAR_PARAM;
    }
    
    match(node->token->type);
    
    if(node->token->type == STAR)
        match(STAR);
    t->attr.dclAttr.name = copyString(node->token->string);
    match(ID);
    
    return t;
}

/* var_dcl -> type-specifier ID | type-specifier ID [ INT ] | type-specifier * ID */
TreeNode* var_dcl() {
    TreeNode* t = newStmtNode(DCL_STMT);
    
    t->nodeKind = DCL_ND;
    if(node->next->token->type == STAR || node->next->next->token->type == LBR) {
        t->attr.dclAttr.type = ADDR_TYPE;
        t->kind.dcl = ARRAY_DCL;
    } else {
        t->attr.dclAttr.type = tokenType_to_expr(node->token->type);
        t->kind.dcl = VAR_DCL;
    }
    
    match(node->token->type);
    if(node->token->type == STAR)
        match(STAR);
    
    t->attr.dclAttr.name = copyString(node->token->string);
    match(ID);
    
    if(node->token->type == LBR) {
        match(LBR);
        t->attr.dclAttr.size = atoi(node->token->string);
        match(RBR);
    }

    match(SEMI);

    return t;
}

/* assign_stmt -> assign ; */
TreeNode* assign_stmt() {
    TreeNode* t = NULL;
    t = assign();
    match(SEMI);
    return t;
}

/* assign -> ID = expression | expression
 Note: The name "assign" is not perfect, it doesn't only refer to assign op but also expression.
 */
TreeNode* assign() {
    TreeNode* t = newStmtNode(ASSIGN_STMT);
    
    if((node->token->type == ID) && (node->next->token->type == ASSIGN)) {
        TreeNode* p = newExpNode(ID_EXPR);
        
        p->attr.exprAttr.name = copyString(node->token->string);
        match(ID);
        
        t->attr.exprAttr.op = ASSIGN;
        t->child[0] = p;
        match(ASSIGN);
        t->child[1] = expression();
    } else {
        t = expression();
    }
    
    return t;
}

/* return_stmt -> return expression ; */
TreeNode * return_stmt() {
    TreeNode* t = newStmtNode(RTN_STMT);
    match(RETURN);
    t->child[0] = expression();
    match(SEMI);
    return t;
}

/* if_stmt -> if (expression ) compound_stmt | if (expression ) compound_stmt else compound_stmt */
TreeNode* if_stmt() {
    TreeNode* t = newStmtNode(SLCT_STMT);
    
    match(IF);
    if(t != NULL) {
        match(LPAR);
        t->child[0] = expression();
        match(RPAR);
    }
    if(t != NULL) {
        t->child[1] = compound_stmt();
    }
    if(node->token->type == ELSE) {
        match(ELSE);
        if(t != NULL) {
            t->child[2] = compound_stmt();
        }
    }
    
    return t;
}

/* while_stmt -> while ( expression ) compound_stmt */
TreeNode* while_stmt() {
    TreeNode* t = newStmtNode(WHILE_STMT);
    
    match(WHILE);
    if(t != NULL) {
        match(LPAR);
        t->child[0] = expression();
        match(RPAR);
    }
    if(t != NULL) {
        t->child[1] = compound_stmt();
    }
    
    return t;
}

/* compoud_stmt -> { stmt_sequence } */
TreeNode* compound_stmt() {
    TreeNode* t = newStmtNode(CMPD_STMT);
    
    match(LCUR);
    t = stmt_sequence();
    match(RCUR);
    
    return t;
}

/* deprecated */
//TreeNode* expression_stmt() {
//    TreeNode* t = expression();
//    match(SEMI);
//    return t;
//}

/* expression -> simple_exp | simple_exp relop simple_exp | call_exp */
TreeNode* expression() {
    TreeNode* t = simple_exp();
    
    if((node->token->type == LT) || (node->token->type == LTE) || (node->token->type == GT) || (node->token->type == GTE) || (node->token->type == EQ) || (node->token->type == NEQ)) {
        TreeNode* p = newExpNode(OP_EXPR);
        if(p != NULL) {
            p->child[0] = t;
            p->attr.exprAttr.op = node->token->type;
            t = p;
        }
        match(node->token->type);
        if(t != NULL) {
            t->child[1] = simple_exp();
        }
    }
    
    if(node->token->type == LPAR) {
        t = call_exp();
    }
    
    return t;
}

/* call_exp -> ID (arg_list) */
TreeNode* call_exp() {
    TreeNode* t = newExpNode(CALL_EXPR);
    t->attr.exprAttr.name = copyString(node->prev->token->string);
    match(node->token->type);
    t->child[0] = arg_list();
    return t;
}

/* arg_list -> arg_list, factor | factor */
TreeNode* arg_list() {
    TreeNode* t = NULL;
    
    match(LPAR);
    if(node->token->type != RPAR) {
        t = factor();
        TreeNode* p = t;
        while(node->token->type != RPAR) {
            TreeNode* q;
            match(COMMA);
            q = factor();
            if (q != NULL) {
                if (t == NULL)
                    t = p = q;
                else /* now p cannot be NULL either */
                {
                    p->rSibling = q;
                    p = q;
                }
            }
        }
    }
    match(RPAR);
    
    return t;
}

/* simple_exp -> term addop term */
TreeNode* simple_exp() {
    TreeNode* t = term();
    
    while((node->token->type == PLUS) || (node->token->type == MINUS)) {
        TreeNode* p = newExpNode(OP_EXPR);
        
        if(p != NULL) {
            p->child[0] = t;
            p->attr.exprAttr.op = node->token->type;
            t = p;
            match(node->token->type);
            t->child[1] = term();
        }
    }
    
    return t;
}

/* term -> factor mulop factor */
TreeNode* term() {
    TreeNode* t = factor();
    
    while((node->token->type == STAR) || (node->token->type == OVER)) {
        TreeNode* p = newExpNode(OP_EXPR);
        
        if(p != NULL) {
            p->child[0] = t;
            p->attr.exprAttr.op = node->token->type;
            t = p;
            match(node->token->type);
            p->child[1] = factor();
        }
    }
    
    return t;
}

/* factor -> ( expression ) | ID | NUMBER */
TreeNode* factor() {
    TreeNode* t = NULL;
    switch(node->token->type) {
        case NUMBER:
            t = newExpNode(CONST_EXPR);
            if((t != NULL) && (node->token->type) == NUMBER) {
                t->attr.exprAttr.val = atoi(node->token->string);
            }
            match(NUMBER);
            break;
        case ID:
            t = newExpNode(ID_EXPR);
            if ((t != NULL) && (node->token->type == ID)) {
                t->attr.exprAttr.name = copyString(node->token->string);
                match(ID);
                // Array:
                if(node->token->type == LBR) {
                    match(LBR);
                    char tail[20] = "[";
                    strcat(tail, node->token->string);
                    strcat(tail, "]");
                    char* array_name = strcat(t->attr.exprAttr.name, tail);
                    t->attr.exprAttr.name = array_name;
                    match(node->token->type);
                    match(RBR);
                }
            } else {
                match(ID);
            }
            break;
        case LPAR :
            match(LPAR);
            t = expression();
            match(RPAR);
            break;
        default:
            syntaxError("unexpected token -> ");
            printToken(node->token->type, node->token->string);
            next();
            break;
    }
    return t;
}

/* program -> stmt_sequence */
TreeNode* parse() {
    TreeNode* root = stmt_sequence();
    return root;
}

int main(int argc, const char * argv[]) {
    // insert code here...
    FILE * fp = fopen("arrayMaxMean_n_tklist.txt", "r");
    listing = fopen("errorlog.txt", "w+");
    if(fp==NULL){
        puts("Cannot open the file");
        return 0;
    }
    TokenList scanResult = read_token_list(fp);
    puts("Scanner is happy.");
    node = scanResult.head;
    TreeNode* root = parse();
    print_tree(root);
    
    printf("Hello, World!\n");
    return 0;
}
