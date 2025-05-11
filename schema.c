#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "schema.h"

Table *create_table(const char *name) {
    Table *table = malloc(sizeof(Table));
    table->name = strdup(name);
    table->columns = NULL;
    table->rows = NULL;
    table->next = NULL;
    return table;
}

void add_column(Table *table, const char *name) {
    Column *col = table->columns;
    while (col) {
        if (strcmp(col->name, name) == 0) return; // already exists
        col = col->next;
    }

    Column *new_col = malloc(sizeof(Column));
    new_col->name = strdup(name);
    new_col->next = table->columns;
    table->columns = new_col;
}

int add_row(Table *table, KVPair *fields) {
    if (!table) {
        fprintf(stderr, "add_row: table is NULL!\n");
        exit(1);
    }
    Row *row = malloc(sizeof(Row));
    if (!row) {
        fprintf(stderr, "add_row: malloc failed!\n");
        exit(1);
    }
    static int id_counter = 1;
    row->id = id_counter++;
    row->fields = fields;
    row->next = table->rows;
    table->rows = row;
    return row->id;
}

KVPair *make_kv(const char *key, const char *value) {
    if (!key || !value) {
        fprintf(stderr, "make_kv: key or value is NULL! key=%s, value=%s\n",
                key ? key : "NULL", value ? value : "NULL");
        return NULL;
    }
    
    fprintf(stderr, "DEBUG: make_kv allocating for key='%s', value='%s'\n", key, value);
    
    KVPair *kv = malloc(sizeof(KVPair));
    if (!kv) {
        fprintf(stderr, "make_kv: malloc failed!\n");
        exit(1);
    }
    
    kv->key = strdup(key);
    kv->value = strdup(value);
    
    fprintf(stderr, "DEBUG: make_kv success - kv=%p, key=%p, value=%p\n", 
            kv, kv->key, kv->value);
    
    kv->next = NULL;
    return kv;
}

char *stringify_ast_value(ASTNode *node) {
    char buffer[64];
    switch (node->type) {
        // In stringify_ast_value() in schema.c:
        case AST_STRING:
            if (node->string_val[0] == '"') {
                char *temp = strdup(node->string_val + 1);
                temp[strlen(temp)-1] = '\0'; // Remove trailing quote
                return temp;
            }
            return strdup(node->string_val);
        case AST_NUMBER:
            snprintf(buffer, sizeof(buffer), "%.2lf", node->num_val);
            return strdup(buffer);
        case AST_BOOL: return strdup(node->bool_val ? "true" : "false");
        case AST_NULL: return strdup("NULL");
        default: return strdup("[complex]");
    }
}

void write_csv(Table *table, const char *dir) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s.csv", dir ? dir : ".", table->name);
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("write_csv");
        return;
    }

    // Write header
    fprintf(f, "id");
    Column *columns = table->columns;
    while (columns) {
        // Skip if column already written
        Column *prev = table->columns;
        int duplicate = 0;
        while (prev != columns) {
            if (strcmp(prev->name, columns->name) == 0) {
                duplicate = 1;
                break;
            }
            prev = prev->next;
        }
        if (!duplicate) {
            fprintf(f, ",%s", columns->name);
        }
        columns = columns->next;
    }
    fprintf(f, "\n");

    for (Row *r = table->rows; r; r = r->next) {
        fprintf(f, "%d", r->id);
        for (Column *c = table->columns; c; c = c->next) {
            KVPair *kv = r->fields;
            const char *val = "";
            while (kv) {
                if (strcmp(kv->key, c->name) == 0) {
                    val = kv->value;
                    break;
                }
                kv = kv->next;
            }
            fprintf(f, ",\"%s\"", val);
        }
        fprintf(f, "\n");
    }

    fclose(f);
}

void free_tables(Table *table) {
    while (table) {
        Table *next = table->next;
        free(table->name);
        // Free columns
        Column *col = table->columns;
        while (col) {
            Column *cnext = col->next;
            free(col->name);
            free(col);
            col = cnext;
        }
        // Free rows
        Row *row = table->rows;
        while (row) {
            Row *rnext = row->next;
            KVPair *kv = row->fields;
            while (kv) {
                KVPair *kvn = kv->next;
                free(kv->key);
                free(kv->value);
                free(kv);
                kv = kvn;
            }
            free(row);
            row = rnext;
        }
        free(table);
        table = next;
    }
}
