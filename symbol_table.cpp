/*
requirements
#include<stdlilb.h>
#include"utils.h"
#include"string_utils.h"
#include"memory_arena.cpp"
*/

struct LTL_symbol {
    LTL_TOKEN_TYPE type;
    u64 count;
    f64 average;
    
    union {
        s64 int_value;
        f64 float_value;
        LTL_expression *ltl_atom;
        char string_value[STRING_LENGTH];
    };

    LTL_symbol *next;
    char *name;
};

struct Symbol_table {
    LTL_symbol *table;
    LTL_symbol *next_guess;
};

u32 length(Symbol_table *table) {
    LTL_symbol *t = table->table;
    u32 result = 0;

    while (t) {
        t = t->next;
        result++;
    }

    return result;
}

bool is_symbol_in_table(Symbol_table *table, char *name) {
    if (table->next_guess && equals(table->next_guess->name, name)) return true;
    LTL_symbol *t = table->table;

    while (t) {
        if (equals(t->name, name)) {
            table->next_guess = t->next;
            return true;
        }

        t = t->next;
    }

    return false;
}

LTL_symbol *_get_symbol(Symbol_table *table, char *name) {
    if (!is_symbol_in_table(table, name)) return NULL;
    
    LTL_symbol *t;

    if (table->next_guess && equals(table->next_guess->name, name)) {
        t = table->next_guess,
        table->next_guess = table->next_guess->next;
        return t;
    }

    t = table->table;
    
    while (t) {
        if (equals(t->name, name)) {
            table->next_guess = t->next;
            return t;
        }

        t = t->next;
    }
    
    return NULL;
}

LTL_symbol *get_symbol_reference(char *name, Symbol_table *table) {
    return _get_symbol(table, name);
}

LTL_symbol *get_symbol(Symbol_table *table, char *name) {
    LTL_symbol *t = table->table;

    while (t) {
        if (equals(t->name, name)) return t;

        t = t->next;
    }
    
    return 0;
}

void update_symbol(Symbol_table *table, char *name, LTL_TOKEN_TYPE type, LTL_expression *value) {
    LTL_symbol *symbol_to_update = _get_symbol(table, name);
    symbol_to_update->type = type;
    symbol_to_update->ltl_atom = value;
}

void update_symbol(Symbol_table *table, char *name, LTL_TOKEN_TYPE type, s64 value) {
    LTL_symbol *symbol_to_update = _get_symbol(table, name);
    symbol_to_update->type = type;
    symbol_to_update->int_value = value;
    float avg = symbol_to_update->average;
    int c = symbol_to_update->count;
    symbol_to_update->average = ((avg * (float)c) + value) / ((float)c + 1);
    ++symbol_to_update->count;
}

void update_symbol(Symbol_table *table, char *name, LTL_TOKEN_TYPE type, f64 value) {
    LTL_symbol *symbol_to_update = _get_symbol(table, name);
    symbol_to_update->type = type;
    symbol_to_update->float_value = value;
    float avg = symbol_to_update->average;
    int c = symbol_to_update->count;
    symbol_to_update->average = ((avg * (float)c) + value) / ((float)c + 1.0);
    ++symbol_to_update->count;
}

void update_symbol(Symbol_table *table, char *name, LTL_TOKEN_TYPE type, char *value) {
    LTL_symbol *symbol_to_update = _get_symbol(table, name);
    symbol_to_update->type = type;
    strcpy(symbol_to_update->string_value, value);
    ++symbol_to_update->count;
}

void _put_symbol(Symbol_table *table, LTL_symbol *var) {
    LTL_symbol *t = table->table;

    if (!t) {
        table->table = var;
        var->next = 0;
    } else {
        while (table->table->next) table->table = table->table->next;

        table->table->next = var;
        var->next = 0;
        table->table = t;
    }
}

#define INSERT_BOOL_BASIC(_table, _name, _value) if (!is_symbol_in_table(_table, _name)) {\
            LTL_symbol *s = GET_MEMORY(LTL_symbol);\
            s->name = GET_MEMORY_COUNT(char, BUFFER_SIZE + 1);\
            strcpy(s->name, _name);\
            s->type = TOKEN_INTEGER;\
            s->int_value = _value;\
            _put_symbol(_table, s);\
        }\

void put_symbol(Symbol_table *table, char *name, LTL_TOKEN_TYPE type, LTL_expression *value) {
    if ((strcmp(name, "true") == 0) || (strcmp(name, "false") == 0)) {
        return;
    }

    if (is_symbol_in_table(table, name)) {
        update_symbol(table, name, type, value);
        return;
    }

    LTL_symbol *s = GET_MEMORY(LTL_symbol);
    
    s->name = GET_MEMORY_COUNT(char, BUFFER_SIZE + 1);
    strcpy(s->name, name);
    s->type = type;
    s->ltl_atom = value;
    
    _put_symbol(table, s);
}

void put_symbol(Symbol_table *table, char *name, LTL_TOKEN_TYPE type, s64 value) {
    if ((strcmp(name, "true") == 0) || (strcmp(name, "false") == 0)) {
        return;
    }

    if (is_symbol_in_table(table, name)) {
        update_symbol(table, name, type, value);
        return;
    }

    LTL_symbol *s = GET_MEMORY(LTL_symbol);
    
    s->name = GET_MEMORY_COUNT(char, BUFFER_SIZE + 1);
    strcpy(s->name, name);
    s->type = type;
    s->int_value = value;
    s->count = 1;
    s->average = (f32) value;
    
    _put_symbol(table, s);
}

void put_symbol(Symbol_table *table, char *name, LTL_TOKEN_TYPE type, f64 value) {
    if ((strcmp(name, "true") == 0) || (strcmp(name, "false") == 0)) {
        return;
    }

    if (is_symbol_in_table(table, name)) {
        update_symbol(table, name, type, value);
        return;
    }

    LTL_symbol *s = GET_MEMORY(LTL_symbol);

    s->name = GET_MEMORY_COUNT(char, BUFFER_SIZE + 1);
    strcpy(s->name, name);
    s->type = type;
    s->float_value = value;
    s->count = 1;
    s->average = value;
    
    _put_symbol(table, s);
}

void put_symbol(Symbol_table *table, char *name, LTL_TOKEN_TYPE type, char *value) {
    if ((strcmp(name, "true") == 0) || (strcmp(name, "false") == 0)) {
        return;
    }

    if (is_symbol_in_table(table, name)) {
        update_symbol(table, name, type, value);
        return;
    }

    LTL_symbol *s = GET_MEMORY(LTL_symbol);

    s->name = GET_MEMORY_COUNT(char, BUFFER_SIZE + 1);
    strcpy(s->name, name);
    s->type = type;
    strcpy(s->string_value, value);
    s->count = 1;
    
    _put_symbol(table, s);
}

char *symbol_type_to_string(LTL_TOKEN_TYPE type) {
    if (type == TOKEN_INTEGER) return "integer";
    if (type == TOKEN_FLOAT) return "float";
    if (type == TOKEN_STRING) return "string";
    if (type == TOKEN_LTL_ATOM) return "atom";
    return "ERROR";
}

void print_symbol_table_content(Symbol_table *table) {
    printf("\n\tNAME\t--\tTYPE\t--\tVALUE\t--\tAVERAGE(count)\n");
    printf("\t#####################################################################\n");
    LTL_symbol *t = table->table;


    while (t) {
        if (t->type == TOKEN_INTEGER) printf("\t%s\t--\t%s\t--\t%lld\t--\t%f(%lld)\n", t->name, symbol_type_to_string(t->type), t->int_value, t->average, t->count);
        if (t->type == TOKEN_FLOAT) printf("\t%s\t--\t%s\t--\t%f--\t%f(%lld)\n", t->name, symbol_type_to_string(t->type), t->float_value, t->average, t->count);
        if (t->type == TOKEN_STRING) printf("\t%s\t--\t%s\t--\t%s\n", t->name, symbol_type_to_string(t->type), t->string_value);
        if (t->type == TOKEN_LTL_ATOM) {
            char formula[BUFFER_SIZE] = {};
            ltl_tree_to_string_formula(t->ltl_atom, formula);

            printf("\t%s\t--\t%s\t--\t%s\n", t->name, symbol_type_to_string(t->type), formula);
        }

        t = t->next;
    }

    printf("\t---------------------------------------------------------------------\n\n");
}
