#ifndef AST_H
#define AST_H

typedef enum {
    AST_OBJECT,
    AST_ARRAY,
    AST_STRING,
    AST_NUMBER,
    AST_BOOL,
    AST_NULL
} ASTType;

typedef struct ASTNode {
    ASTType type;
    char *key; // For object fields
    union {
        char *string_val;
        double num_val;
        int bool_val;
    };
    struct ASTNode *child;
    struct ASTNode *next;
} ASTNode;

// Constructors
ASTNode *make_string_node(char *str);
ASTNode *make_number_node(char *num_str);
ASTNode *make_bool_node(int value);
ASTNode *make_null_node();
ASTNode *make_object_node(ASTNode *pairs);
ASTNode *make_array_node(ASTNode *elements);
ASTNode *make_pair_node(char *key, ASTNode *value);
ASTNode *make_pair_list(ASTNode *pair, ASTNode *rest);
ASTNode *make_array_list(ASTNode *element, ASTNode *rest);

// AST printer
void print_ast(ASTNode *node, int indent);

#endif
