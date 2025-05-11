#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "schema.h"

static Table *tables = NULL;

Table *get_or_create_table(const char *name) {
    Table *t = tables;
    while (t) {
        if (strcmp(t->name, name) == 0) return t;
        t = t->next;
    }
    Table *new = create_table(name);
    new->next = tables;
    tables = new;
    return new;
}

// Forward declaration
void process_value(ASTNode *node, const char *table_name, int parent_id, const char *parent_key);

int process_object(ASTNode *node, const char *table_name, int parent_id, const char *parent_key) {
    Table *t = get_or_create_table(table_name);
    KVPair *fields = NULL;

    if (parent_key && parent_id > 0) {
        char fk[64];
        snprintf(fk, sizeof(fk), "%s_id", parent_key);
        add_column(t, fk);

        char id_str[16];
        sprintf(id_str, "%d", parent_id);
        KVPair *fkpair = make_kv(fk, id_str);
        if (!fkpair) {
            fprintf(stderr, "Error: make_kv returned NULL for key=%s, val=%s\n", fk, id_str ? id_str : "NULL");
            exit(1);
        }
        fkpair->next = fields;
        fields = fkpair;
    }

    ASTNode *arrays_to_process[32];
    int arrays_count = 0;

    ASTNode *field = node->child;
    while (field) {
        if (field->type == AST_OBJECT) {
            if (!field->key) {
                fprintf(stderr, "Error: AST_OBJECT field has NULL key!\n");
                exit(1);
            }
            int child_id = process_object(field, field->key, 0, NULL);
    
            char fk[64];
            snprintf(fk, sizeof(fk), "%s_id", field->key);
            add_column(t, fk);
    
            char id_str[16];
            sprintf(id_str, "%d", child_id);
            KVPair *fkpair = make_kv(fk, id_str);
            if (!fkpair) {
                fprintf(stderr, "Error: make_kv returned NULL for key=%s, val=%s\n", fk, id_str ? id_str : "NULL");
                exit(1);
            }
            fkpair->next = fields;
            fields = fkpair;
    
        } else if (field->type == AST_ARRAY) {
            if (!field->key) {
                fprintf(stderr, "Error: AST_ARRAY field has NULL key!\n");
                exit(1);
            }
            arrays_to_process[arrays_count++] = field;
        } else {
            if (!field->key) {
                fprintf(stderr, "Error: Scalar field has NULL key!\n");
                exit(1);
            }
            add_column(t, field->key);
            char *val = stringify_ast_value(field);
            fprintf(stderr, "DEBUG: stringify returned val=%p ('%s')\n", val, val);
            KVPair *kv = make_kv(field->key, val);
            free(val);
            if (!kv) {
                fprintf(stderr, "Error: Failed to create KVPair for key=%s\n", field->key);
                exit(1);
            }
            kv->next = fields;  
            fields = kv;
            if (!fields) {
                fprintf(stderr, "Error: Failed to add field to linked list\n");
                exit(1);
            }
        }
        field = field->next;
    }

    if (!fields) {
        fprintf(stderr, "Warning: No fields for table '%s'\n", t->name);
    } else {
        KVPair *kv = fields;
        fprintf(stderr, "Adding row to table '%s' with fields: ", t->name);
        while (kv) {
            fprintf(stderr, "%s=%s ", kv->key, kv->value);
            kv = kv->next;
        }
        fprintf(stderr, "\n");
    }
    
    int row_id = add_row(t, fields);
    if (!t->rows) {
        fprintf(stderr, "Error: Table '%s' has no rows after add_row\n", t->name);
        exit(1);
    }

    // Now process arrays with correct parent_id
    for (int i = 0; i < arrays_count; ++i) {
        ASTNode *arr = arrays_to_process[i];
        process_value(arr, arr->key, row_id, table_name);
    }
    return row_id; // latest inserted row (head of list)
}

void process_array(ASTNode *node, const char *table_name, int parent_id, const char *parent_key) {
    if (!table_name || !parent_key) {
        fprintf(stderr, "Error: process_array called with NULL table_name or parent_key\n");
        exit(1);
    }
    ASTNode *elem = node->child;
    int index = 0;

    if (!elem) return;

    if (elem->type == AST_OBJECT) {
        while (elem) {
            process_object(elem, table_name, parent_id, parent_key);
            elem = elem->next;
            index++;
        }
    } else {
        // Scalar array → junction table
        char junction_name[64];
        snprintf(junction_name, sizeof(junction_name), "%s_%s", parent_key, table_name);
        Table *jt = get_or_create_table(junction_name);

        add_column(jt, "index");
        add_column(jt, "value");

        char fk_col[64];
        snprintf(fk_col, sizeof(fk_col), "%s_id", parent_key);
        add_column(jt, fk_col);

        while (elem) {
            KVPair *fields = NULL;

            // index
            char idx[16];
            sprintf(idx, "%d", index);
            fields = make_kv("index", idx);
            if (!fields) {
                fprintf(stderr, "Error: make_kv returned NULL for key=\"index\", val=%s\n", idx ? idx : "NULL");
                exit(1);
            }

            // value
            char *val = stringify_ast_value(elem);
            KVPair *valkv = make_kv("value", val);
            if (!valkv) {
                fprintf(stderr, "Error: make_kv returned NULL for key=\"value\", val=%s\n", val ? val : "NULL");
                exit(1);
            }
            valkv->next = fields;
            fields = valkv;

            // FK - Fix this part to use the actual parent_id
            char idstr[16];
            sprintf(idstr, "%d", parent_id);  // Use the provided parent_id
            KVPair *fkpair = make_kv(fk_col, idstr);
            if (!fkpair) {
                fprintf(stderr, "Error: make_kv returned NULL for key=%s, val=%s\n", fk_col, idstr ? idstr : "NULL");
                exit(1);
            }
            fkpair->next = fields;
            fields = fkpair;

            add_row(jt, fields);
            free(val);  // Don't forget to free the stringified value

            elem = elem->next;
            index++;
        }
    }
}

void process_value(ASTNode *node, const char *table_name, int parent_id, const char *parent_key) {
    printf("DEBUG: process_value called with...");
    if (!node) return;
    printf("table_name=%s, parent_id=%d, parent_key=%s\n", table_name, parent_id, parent_key);

    switch (node->type) {
        case AST_OBJECT:
            process_object(node, table_name, parent_id, parent_key);
            break;
        case AST_ARRAY:
            process_array(node, table_name, parent_id, parent_key);
            break;
        default:
            // Scalars are already handled during object processing
            break;
    }
}

void generate_csvs(const char *dir) {
    Table *t = tables;
    while (t) {
        write_csv(t, dir);
        t = t->next;
    }
}

void cleanup_all() {
    free_tables(tables);
    tables = NULL;  // Important to prevent dangling pointer
}

