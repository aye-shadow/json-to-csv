#ifndef SCHEMA_H
#define SCHEMA_H

#include "ast.h"

typedef struct Column {
    char *name;
    struct Column *next;
} Column;

typedef struct Row {
    int id;
    struct KVPair *fields;
    struct Row *next;
} Row;

typedef struct KVPair {
    char *key;
    char *value; // stringified version of any scalar
    struct KVPair *next;
} KVPair;

typedef struct Table {
    char *name;
    Column *columns;
    Row *rows;
    struct Table *next;
} Table;

Table *create_table(const char *name);
void add_column(Table *table, const char *name);
int add_row(Table *table, KVPair *fields);
void write_csv(Table *table, const char *dir);
char *stringify_ast_value(ASTNode *node);
void free_tables(Table *table);

#endif
