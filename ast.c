#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode *make_string_node(char *str) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_STRING;
    node->key = NULL;
    node->string_val = str;
    node->child = node->next = NULL;
    return node;
}

ASTNode *make_number_node(char *num_str) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_NUMBER;
    node->key = NULL;
    node->num_val = atof(num_str);
    free(num_str);
    node->child = node->next = NULL;
    return node;
}

ASTNode *make_bool_node(int value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_BOOL;
    node->key = NULL;
    node->bool_val = value;
    node->child = node->next = NULL;
    return node;
}

ASTNode *make_null_node() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_NULL;
    node->key = NULL;
    node->child = node->next = NULL;
    return node;
}

ASTNode *make_object_node(ASTNode *pairs) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_OBJECT;
    node->key = NULL;
    node->child = pairs;
    node->next = NULL;
    return node;
}

ASTNode *make_array_node(ASTNode *elements) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_ARRAY;
    node->key = NULL;
    node->child = elements;
    node->next = NULL;
    return node;
}

ASTNode *make_pair_node(char *key, ASTNode *value) {
    // Remove surrounding quotes if present
    if (key[0] == '"' && key[strlen(key)-1] == '"') {
        key[strlen(key)-1] = '\0';
        value->key = strdup(key+1);
    } else {
        value->key = strdup(key);
    }
    free(key);
    return value;
}

ASTNode *make_pair_list(ASTNode *pair, ASTNode *rest) {
    pair->next = rest;
    return pair;
}

ASTNode *make_array_list(ASTNode *element, ASTNode *rest) {
    element->next = rest;
    return element;
}

void print_indent(int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
}

void print_ast(ASTNode *node, int indent) {
    if (!node) return;

    while (node) {
        print_indent(indent);

        if (node->key)
            printf("\"%s\": ", node->key);

        switch (node->type) {
            case AST_STRING:
                printf("\"%s\"\n", node->string_val);
                break;
            case AST_NUMBER:
                printf("%lf\n", node->num_val);
                break;
            case AST_BOOL:
                printf(node->bool_val ? "true\n" : "false\n");
                break;
            case AST_NULL:
                printf("null\n");
                break;
            case AST_OBJECT:
                printf("{\n");
                print_ast(node->child, indent + 1);
                print_indent(indent);
                printf("}\n");
                break;
            case AST_ARRAY:
                printf("[\n");
                print_ast(node->child, indent + 1);
                print_indent(indent);
                printf("]\n");
                break;
        }

        node = node->next;
    }
}
