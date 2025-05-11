#include "ast.h"
#include "parser.h"
#include <stdio.h>
#include "schema.h"

// Declare root node from parser
extern ASTNode *root;

int main(int argc, char **argv) {
    if (yyparse() == 0) {
        printf("AST:\n");
        printf("====================================\n");
        print_ast(root, 0);
        printf("====================================\n");
        process_value(root, "root", 0, NULL);
        printf("Tables:\n");
        printf("====================================\n");
        generate_csvs(".");
        printf("====================================\n");
        cleanup_all();
        printf("Cleanup done.\n");
    } else {
        printf("Parsing failed.\n");
    }

    return 0;
}
