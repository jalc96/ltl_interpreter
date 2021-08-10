#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<cstring>
#include<math.h>
#include<intrin.h>

#define BUFFER_SIZE 256
#define FOLDERS_COUNT 256
#define FOLDERS_NAME_LENGTH 256
#define NXT
#define NOMINNMAX

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

#define LEAP_YEAR(Y) (((1970 + Y) > 0) && !((1970 + Y) % 4) && (((1970 + Y) % 100) || !((1970 + Y) % 400)))

#include"utils.cpp"

#ifndef STRING_LENGTH
#define STRING_LENGTH 512
#endif

#include"string_utils.cpp"
#include"memory_arena.cpp"

inline void error(char *s) {
    printf("\n");
    RED("    ERROR");
    printf(": %s\n", s);
    exit(1);
}

struct Console_parameters {
    bool formula = false;
    char *f_value = 0;
    bool input_file = false;
    char *i_value = 0;
    bool debug_info = false;
    bool memory_used = false;
    bool elapsed = false;
    bool help = false;
    bool usage = false;
    bool tutorial = false;
};

global_variable Console_parameters parameters = {};

enum LTL_TOKEN_TYPE {
    TOKEN_LTL_ERROR,
    TOKEN_EOF,
    
    TOKEN_VARIABLE,
    TOKEN_LTL_ATOM,
    
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_STRING_LITERAL,
    TOKEN_DATE,
    
    TOKEN_GREATER_THAN,
    TOKEN_LESS_THAN,
    TOKEN_GREATER_THAN_OR_EQUALS,
    TOKEN_LESS_THAN_OR_EQUALS,
    TOKEN_EQUALS_TO,
    TOKEN_NOT_EQUALS_TO,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_ADD,
    TOKEN_MINUS,
    TOKEN_UNARY_MINUS,
    TOKEN_MULTIPLICATION,
    TOKEN_MODULUS,
    TOKEN_DIVISION,
    TOKEN_ASSIGNMENT,
    TOKEN_LTL_ALWAYS,
    TOKEN_LTL_NEXT,
    TOKEN_LTL_EVENTUALLY,
    TOKEN_LTL_UNTIL,
    TOKEN_LTL_RELEASE,
    TOKEN_LTL_NOT,
    TOKEN_LTL_OR,
    TOKEN_LTL_AND,
    TOKEN_CONSEQUENCE,
    TOKEN_BICONDITIONAL,
    
    TOKEN_UNDERSCORE,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_RIGHT_CURLY_BRACE,
    TOKEN_LEFT_CURLY_BRACE,
    TOKEN_LTL_FUNCTION_CALL,
};

global_variable memory_arena *ARENA;
global_variable Color_rgb *csv_debug_colors;
global_variable Color_rgb *tree_debug_colors;
global_variable Color_rgb *machines_debug_colors;

struct LTL_expression;
void ltl_tree_to_string_formula(LTL_expression *tree, char *formula);

#define VARIABLE_NAME_LENGTH 256
#include"symbol_table.cpp"
#include"parser.cpp"

global_variable Symbol_table *SYMBOL_TABLE;
global_variable Symbol_table EMPTY_SYMBOL_TABLE = {};

char final_never_claim[4096] = {};
#include"ltl2ba.h"

#if _WIN32
#include"win32_platform.cpp"
#else
#include"linux_platform.cpp"
#endif

struct State_machine;
void print_machine(State_machine *state_machine);
void put_line_into_symbol_table(Csv_header *header, char *line, Symbol_table *st);
inline bool execute_machine(State_machine *state_machine, u32 start, u32 finish);


#define MESSAGES_PER_QUEUE 500000

struct Trace {
    u32 first_free;
    Trace *next;
    char messages[MESSAGES_PER_QUEUE][STRING_LENGTH];
};

global_variable Trace *T;

void free_trace(Trace *trace) {
    if (trace) {
        free_trace(trace->next);
        free(trace);
    }
}

void enqueue(Trace *trace, char *text) {
    while (trace->next) {
        trace = trace->next;
    }

    if (trace->first_free >= MESSAGES_PER_QUEUE) {
        trace->next = (Trace *)malloc(sizeof(Trace));

        if (!trace->next) {
            error("run out of memory");
        }

        trace = trace->next;
        trace->first_free = 0;
        trace->next = 0;
    }

    char *msg = trace->messages[trace->first_free++];
    strcpy(msg, text);
}

char *get_message(Trace *trace, u32 i) {
    u32 chunk = i / MESSAGES_PER_QUEUE;
    u32 index = i % MESSAGES_PER_QUEUE;

    while (chunk > 0) {
        trace = trace->next;
        chunk--;
        assert_msg(trace, "trace next wasnt initialized");
    }

    return trace->messages[index];
}

void print(Trace *trace) {
    u32 index = 0;

    while (trace) {
        for (u32 i = 0; i < trace->first_free; i++) {
            printf("%d: %s\n", index, trace->messages[i]);
            index++;
        }

        trace = trace->next;
    }
}

u32 length(Trace *trace) {
    u32 result = 0;

    while (trace) {
        result += trace->first_free;
        trace = trace->next;
    }

    return result;
}

u32 replacing_index = 0;

void replace_formulas(LTL_expression *tree, bool from_arithmetic=false) {
    char replacing_name[BUFFER_SIZE] = {};

    if (is_relop(tree->type)) {
        ltl_tree_to_string_formula(tree, tree->formula);
        sprintf(replacing_name, "a%d", replacing_index++);
        LTL_expression *store_tree = GET_MEMORY(LTL_expression);
        (*store_tree) = (*tree);
        put_symbol(SYMBOL_TABLE, replacing_name, TOKEN_LTL_ATOM, store_tree);
        strcpy(tree->formula, replacing_name);
        tree->type = TOKEN_VARIABLE;
    } else if (is_binary_operator(tree->type)) {
        replace_formulas(tree->left);
        replace_formulas(tree->right);
    } else if (is_binary_aritmetic_operator(tree->type)) {
        replace_formulas(tree->left, true);
        replace_formulas(tree->right, true);
    } else if (tree->type == TOKEN_UNARY_MINUS) {
        replace_formulas(tree->right, true);
    } else if (is_unary_operator(tree->type)) {
        replace_formulas(tree->right);
    } else if (tree->type == TOKEN_INTEGER) {
        s32 value = to_int(tree->formula);

        if (from_arithmetic) {

        } else {
            if (value == 1) {
                strcpy(tree->formula, "true");
            } else {
                strcpy(tree->formula, "false");
            }
        }
    }
}

struct LTL_condition {
    LTL_expression *tree_condition;
    u16 next_index;
    char condition[BUFFER_SIZE];

    bool is_true(Symbol_table *st);
};

bool LTL_condition::is_true(Symbol_table *st=0) {
    Symbol_table *symbol_table;

    if (!st) {
        symbol_table = SYMBOL_TABLE;
    } else {
        symbol_table = st;
    }

    if (!tree_condition) {
        if (is_symbol_in_table(symbol_table, condition)) {
            LTL_symbol *s = get_symbol(symbol_table, condition);
            tree_condition = s->ltl_atom;
        } else {
            Input_buffer eval;
            eval.raw_buffer = condition;
            eval.count = length(eval.raw_buffer);
            eval.reset();
            eval.get_next_token();
            tree_condition = ltl_expression(&eval);
            get_variable_references_from_symbol_table(tree_condition, SYMBOL_TABLE);
        }
    }

    Operation_value r = {};
    bool atom_evaluation = eval_tree(tree_condition, &r);
    return atom_evaluation;
}

#define NUMBER_OF_CONDITIONS 20
#define NUMBER_OF_NODES 40

struct LTL_node {
    u16 condition_last;
    bool is_accept_state;
    char name[BUFFER_SIZE];
    LTL_condition conditions[NUMBER_OF_CONDITIONS];
};

struct LTL_graph {
    s32 current_index;
    bool already_blocked;
    u16 nodes_last;
    bool result;
    LTL_node nodes[NUMBER_OF_NODES];

    bool evaluate_step(bool debug, Trace *trace, u32 steps_to_skip);
    u16 get_index_by_name(char *node_name) {
        u16 i = 0;
        
        while (i <= nodes_last) {
            if (equals(node_name, nodes[i].name)) return i;
            ++i;
        }

        error("node name not in graph");
        return -1;
    }
};

#define STACK_SIZE 300000

struct Stack_item {
    s32 child_index;
    s32 trace_index;
    LTL_node *node;
};

struct Nodes_stack{
    u32 head;
    Nodes_stack *next;
    Stack_item data[STACK_SIZE];
};

void push(Nodes_stack *stack, LTL_node *node, u32 trace_index, u32 child_index=0) {
    while (stack->next) {
        stack = stack->next;
    }

    if (stack->head >= STACK_SIZE) {
        stack->head--;
        stack->next = (Nodes_stack *)malloc(sizeof(Nodes_stack));

        if (stack->next) {
            stack = stack->next;
            stack->head = 0;
            stack->next = 0;
        } else {
            error("run out of memory");
        }
    }

    stack->data[stack->head].node = node;
    stack->data[stack->head].child_index = child_index;
    stack->data[stack->head].trace_index = trace_index;
    stack->head++;
}

void print(Nodes_stack *stack) {
    while (stack) {
        if (stack->head == STACK_SIZE) {
            printf("[%d], ", stack->head + 1);
        } else {
            printf("[%d], ", stack->head);
        }

        stack = stack->next;
    }

    putchar('\n');
}

Stack_item *pop(Nodes_stack *stack) {
    Nodes_stack *pre = 0;

    while (stack->next) {
        pre = stack;
        stack = stack->next;
    }

    if (stack->head == 0) {
        free(stack);
        if (pre) pre->next = 0;
        stack = pre;
    }

    if (!stack) return 0;

    stack->head--;
    Stack_item *result = &stack->data[stack->head];

    return result;
}

u64 times_evaluated = 0;

bool LTL_graph::evaluate_step(bool debug, Trace *trace, u32 steps_to_skip) {
    // NOTE: steps_to_skip is used for the first trace line to read, if we have to skip 2 (2 consecutive nexts) the first line entered into the symbol table is the 2
    Nodes_stack *stack = (Nodes_stack *)malloc(sizeof(Nodes_stack));
    stack->head = 0;
    stack->next = 0;

    push(stack, &nodes[0], steps_to_skip, nodes[0].condition_last - 1);

    u32 l = length(trace);

    Stack_item *state = pop(stack);
    LTL_condition *child;

    bool result = false;

    while (state) {
        times_evaluated++;

        if (state->trace_index >= l) {
            if (result) {
                break;
            }
            
            state = pop(stack);

            while (state && state->child_index < 0) {
                state = pop(stack);
            }

            continue;
        }

        put_line_into_symbol_table(HEADER, get_message(trace, state->trace_index), SYMBOL_TABLE);
        child = &state->node->conditions[state->child_index--];

        if (child->is_true()) {
            // printf("%s -> %s <%s> [%d](%s)\n", state->node->name, nodes[child->next_index].name, child->condition, state->trace_index, get_message(trace, state->trace_index));
            
            if ((state->trace_index + 1) == l) {
                // if its the end of the trace we need to check if the state is accept state
                result = nodes[child->next_index].is_accept_state;
            }

            push(stack, state->node, state->trace_index, state->child_index);
            push(stack, &nodes[child->next_index], state->trace_index + 1, nodes[child->next_index].condition_last - 1);
            state = pop(stack);
        }

        while (state && state->child_index < 0) {
            state = pop(stack);
        }
    }

    return result;
}

LTL_graph *create_graph(Input_buffer *never_claim_process) {
    LTL_graph *graph = GET_MEMORY(LTL_graph);
    graph->current_index = 0;
    graph->already_blocked = false;
    graph->result = true;
    graph->nodes_last = 0;
    
    never_claim_process->get_next_token();
    if (!equals(never_claim_process->current_token.value, "never")) error("not never_claim_init");
    never_claim_process->get_next_token();
    if (!equals(never_claim_process->current_token.value, "{")) error("not { symbol");
    never_claim_process->get_next_token();
    
    // First detect and store all the node names

    while (never_claim_process->current_char != 0) {
        LTL_node *node = &graph->nodes[graph->nodes_last++];
        strcpy(node->name, never_claim_process->current_token.value);
        node->is_accept_state = strstr(never_claim_process->current_token.value, "accept") != NULL;

        while (!equals(never_claim_process->current_token.value, ";") && never_claim_process->current_char != 0) {
            never_claim_process->get_next_token();
        }
        
        never_claim_process->get_next_token();
    }

    never_claim_process->reset();
    // Now make the connections between nodes
    never_claim_process->get_next_token();
    never_claim_process->get_next_token();
    never_claim_process->get_next_token();
    
    u32 state_index = 0;
    LTL_node *current_node = &graph->nodes[state_index];
    LTL_condition *condition;

    LTL_node *current_node_conditions;

    while (never_claim_process->current_char != 0) {
        current_node_conditions = &graph->nodes[graph->get_index_by_name(never_claim_process->current_token.value)];

        /*
        if
            repeat:
                if skip node references itself jump repeat
                :
                :
                (
                strcpy(condition->condition, token->value)
                )
                ->
                goto
                condition->next_index = get_index(...)
                \n
            jump repeat
        fi
        ;
        */
        while (!equals(never_claim_process->current_token.value, "if") && !equals(never_claim_process->current_token.value, "skip") && !equals(never_claim_process->current_token.value, "false") ) {
            never_claim_process->get_next_token();
        }

        if (equals(never_claim_process->current_token.value, "false")) {
            condition = &current_node->conditions[current_node_conditions->condition_last++];
            strcpy(condition->condition, "0");
            
            break;
        }

        if (equals(never_claim_process->current_token.value, "skip")) {
            condition = &current_node->conditions[current_node_conditions->condition_last++];
            strcpy(condition->condition, "1");
            
            condition->next_index = graph->get_index_by_name(current_node_conditions->name);
            break;
        }

        never_claim_process->get_next_token();

        while (!equals(never_claim_process->current_token.value, "fi")) {
            // :
            never_claim_process->get_next_token();
            // :
            never_claim_process->get_next_token();

            if (!equals(never_claim_process->current_token.value, "(")) error("( expected");
            // (
            never_claim_process->get_next_token();

            // (a0) ([|| &&] (an))*
            condition = &current_node->conditions[current_node->condition_last++];
            strcpy(condition->condition, never_claim_process->current_token.value);

            never_claim_process->get_next_token();

            while (!equals(never_claim_process->current_token.value, "->")) {
                if (equals(never_claim_process->current_token.value, "(") || equals(never_claim_process->current_token.value, ")")) {
                    never_claim_process->get_next_token();
                    continue;
                }

                strcat(condition->condition, never_claim_process->current_token.value);
                never_claim_process->get_next_token();
            }
            
            // ->
            never_claim_process->get_next_token();
            
            if (!equals(never_claim_process->current_token.value, "goto")) error("goto expected");
            
            // goto
            never_claim_process->get_next_token();
            
            condition->next_index = graph->get_index_by_name(never_claim_process->current_token.value);
            never_claim_process->get_next_token();

            if (equals(never_claim_process->current_token.value, "fi")) {
                never_claim_process->get_next_token();
                never_claim_process->get_next_token();
                break;
            }
        }

        if (equals(never_claim_process->current_token.value, "}")) break;

        ++state_index;
        current_node = &graph->nodes[state_index];
    }

    return graph;
}

void print_graph_debug_info(LTL_graph *graph) {
    for (u32 i = 0; i < graph->nodes_last; ++i) {
        LTL_node n = graph->nodes[i];
        printf("%s\n", n.name);

        for (u32 j = 0; j < n.condition_last; ++j) {
            LTL_condition c = n.conditions[j];
            printf("  %s -> %s\n", c.condition, graph->nodes[c.next_index].name);
        }
    }
}

u32 count_consecutive_next_in_single_operations(LTL_expression *tree) {
    u32 result = 0;

    if (tree->type == TOKEN_LTL_NEXT) {
        ++result;
        result += count_consecutive_next_in_single_operations(tree->right);
    } else if (tree->type == TOKEN_LTL_EVENTUALLY || tree->type == TOKEN_LTL_ALWAYS) {
        result += count_consecutive_next_in_single_operations(tree->right);
    }

    return result;
}

void trim_consecutive_next(LTL_expression *tree) {
    if (is_unary_operator(tree->type)) {
        if (tree->type == TOKEN_LTL_NEXT) {
            *tree = *tree->right;
            trim_consecutive_next(tree);
        } else {
            trim_consecutive_next(tree->right);
        }
    }
}

void print_colored_message(char *message) {
    Input_buffer d = {};
    d.raw_buffer = message;
    d.count = length(message);
    d.reset();

    u32 index_colors = 0;

    while (!d.has_ended()) {
        d.get_next_csv_token();

        if (is_leaf(d.current_token.type) || d.current_token.type == TOKEN_UNARY_MINUS) {
            Color_rgb color = csv_debug_colors[index_colors];
            u32 r = color.r;
            u32 g = color.g;
            u32 b = color.b;
            color_text(d.current_token.value, r, g, b);
        } else if (d.current_token.type == TOKEN_COMMA) {
            index_colors++;
            putchar(',');
            putchar(' ');
        }
    }

    putchar('\n');
}

bool has_intervals(LTL_expression *tree) {
    if (is_binary_operator(tree->type)) {
        if (tree->interval) return true;
        if (has_intervals(tree->left)) return true;
        return has_intervals(tree->right);
    } else if (is_unary_operator(tree->type)) {
        if (tree->interval) return true;
        return has_intervals(tree->right);
    } else if (is_relop(tree->type)){
        return tree->interval != NULL;
    } else if (is_basic_type(tree->type) || tree->type == TOKEN_VARIABLE || is_binary_aritmetic_operator(tree->type) || tree->type == TOKEN_UNARY_MINUS){
        return false;
    } else {
        char msg[BUFFER_SIZE];
        sprintf(msg, "not recognized node to check for intervals, <%s>", to_string(tree->type));
        error(msg);
        return false;
    }
}

bool every_node_able_to_have_interval_has_interval(LTL_expression *tree) {
    if (is_ltl_operator(tree->type) && tree->interval == NULL) return false;

    if (is_binary_operator(tree->type)) {
        if (!every_node_able_to_have_interval_has_interval(tree->left)) return false;
        return every_node_able_to_have_interval_has_interval(tree->right);
    } else if (is_unary_operator(tree->type)) {
        return every_node_able_to_have_interval_has_interval(tree->right);
    } else {
        return true;
    }
}

bool has_nexts(LTL_expression *tree) {
    if (is_binary_operator(tree->type)) {
        if (has_nexts(tree->left)) return true;
        return has_nexts(tree->right);
    } else if (is_unary_operator(tree->type)) {
        if (tree->type == TOKEN_LTL_NEXT) return true;
        return has_nexts(tree->right);
    } else {
        return false;
    }
}

enum STATE_MACHINE_TYPE {
    MACHINE_FORMULA,
    MACHINE_NOT,
    MACHINE_LOGIC, 
    MACHINE_UNTIL,
};

char *to_string(STATE_MACHINE_TYPE type) {
    if (type == MACHINE_FORMULA) return "FORMULA";
    if (type == MACHINE_NOT) return "NOT";
    if (type == MACHINE_LOGIC) return "LOGIC";
    if (type == MACHINE_UNTIL) return "UNTIL";
    return "error";
}

enum FORMULA_STATE_FUNCTION {
    NONE,
    AVERAGE,
};

struct Formula {
    u32 id;
    u32 depth;
    FORMULA_STATE_FUNCTION function;
    LTL_expression *formula;
    Csv_header *header;
    Symbol_table st;

    bool process_event(u32 start, u32 finish);
};

bool Formula::process_event(u32 start, u32 finish) {
    char debug_formula_eval[BUFFER_SIZE] = {};
    ltl_tree_to_string_formula(formula, debug_formula_eval);

    char msg[BUFFER_SIZE];
    char pre[BUFFER_SIZE];
    Color_rgb color = machines_debug_colors[id];

    if (parameters.debug_info) {
        u32 index;

        for (index = 0; index < (depth * 4); index++) {
            pre[index] = ' ';
        }
        
        pre[index] = 0;

        Color_rgb color = machines_debug_colors[id];
        char msg[BUFFER_SIZE] = {};
        sprintf(msg, "%s[%d] ", pre, id);
        color_text(msg, color.r, color.g, color.b);

        printf("<Formula machine> \"%s\"\n", debug_formula_eval);
        printf("%s    [%d, %d)\n", pre, start, finish);
        
        pre[index++] = ' ';
        pre[index++] = ' ';
        pre[index++] = ' ';
        pre[index++] = ' ';
        pre[index] = ' ';
        pre[index + 1] = ' ';
        pre[index + 2] = ' ';
        pre[index + 3] = 0;

        print_header(header, pre);

        pre[index] = 0;
    }

    bool result = true;

    bool symbols_referenced = false;
    Operation_value formula_result;

    for (u32 i = start; i < finish; i++) {
        if (parameters.debug_info) {
            sprintf(msg, "%s%d: ", pre, i);
            color_text(msg, color.r, color.g, color.b);
            print_colored_message(get_message(T, i));
        }

        put_line_into_symbol_table(header, get_message(T, i), &st);

        if (!symbols_referenced) {
            get_variable_references_from_symbol_table(formula, &st);
            get_variable_references_from_symbol_table(header, &st);
            symbols_referenced = true;
        }

        if (function == NONE) {
            bool e = eval_tree(formula, &formula_result);
            result = result && e;
        }
    }

    if (function != NONE) {
        bool e = eval_tree(formula, &formula_result);
        result = result && e;
    }

    return result;
}

struct Not {
    u32 id;
    u32 depth;
    State_machine *c1;

    bool process_event(u32 start, u32 finish);
};

bool Not::process_event(u32 start, u32 finish) {
    if (parameters.debug_info) {
        Color_rgb color = machines_debug_colors[id];
        char msg[BUFFER_SIZE];
        char pre[BUFFER_SIZE];
        u32 index;

        for (index = 0; index < (depth * 4); index++) {
            pre[index] = ' ';
        }
        
        pre[index] = 0;
        
        sprintf(msg, "%s[%d] ", pre, id);
        color_text(msg, color.r, color.g, color.b);
        printf("<Not machine>\n");

        printf("%s    [%d, %d)\n", pre, start, finish);

        pre[index++] = ' ';
        pre[index++] = ' ';
        pre[index++] = ' ';
        pre[index++] = ' ';
        pre[index] = ' ';
        pre[index + 1] = ' ';
        pre[index + 2] = ' ';
        pre[index + 3] = 0;

        print_header(HEADER, pre);

        pre[index] = 0;

        for (u32 i = start; i < finish; i++) {
            sprintf(msg, "%s%d: ", pre, i);
            color_text(msg, color.r, color.g, color.b);
            print_colored_message(get_message(T, i));
        }
    }

    bool result = !execute_machine(c1, start, finish);
    return result;
}

struct Logic {
    LTL_TOKEN_TYPE logic_type;
    u32 id;
    u32 depth;
    State_machine *c1;
    State_machine *c2;

    bool process_event(u32 start, u32 finish);
};

char *logic_machine_type_to_string(LTL_TOKEN_TYPE logic_type) {
    if (logic_type == TOKEN_LTL_OR) {
        return "OR";
    } else if (logic_type == TOKEN_LTL_AND) {
        return "AND";
    } else if (logic_type == TOKEN_CONSEQUENCE) {
        return "CONSEQUENCE";
    } else if (logic_type == TOKEN_BICONDITIONAL) {
        return "BICONDITIONAL";
    }

    return "ERROR";
}

bool Logic::process_event(u32 start, u32 finish) {
    if (parameters.debug_info) {
        Color_rgb color = machines_debug_colors[id];
        char msg[BUFFER_SIZE];
        char pre[BUFFER_SIZE];
        u32 index;

        for (index = 0; index < (depth * 4); index++) {
            pre[index] = ' ';
        }
        
        pre[index] = 0;
        
        sprintf(msg, "%s[%d] ", pre, id);
        color_text(msg, color.r, color.g, color.b);
        printf("<Logic machine %s>\n", logic_machine_type_to_string(logic_type));

        printf("%s    [%d, %d)\n", pre, start, finish);

        pre[index++] = ' ';
        pre[index++] = ' ';
        pre[index++] = ' ';
        pre[index++] = ' ';
        pre[index] = ' ';
        pre[index + 1] = ' ';
        pre[index + 2] = ' ';
        pre[index + 3] = 0;

        print_header(HEADER, pre);

        pre[index] = 0;

        for (u32 i = start; i < finish; i++) {
            sprintf(msg, "%s%d: ", pre, i);
            color_text(msg, color.r, color.g, color.b);
            print_colored_message(get_message(T, i));
        }
    }

    bool l = execute_machine(c1, start, finish);
    bool r = execute_machine(c2, start, finish);
    bool result;

    if (logic_type == TOKEN_LTL_OR) {
        result = l || r;
    } else if (logic_type == TOKEN_LTL_AND) {
        result = l && r;
    } else if (logic_type == TOKEN_CONSEQUENCE) {
        result = !l || r;
    } else if (logic_type == TOKEN_BICONDITIONAL) {
        result = l == r;
    }

    return result;
}

struct Until {
    u32 id;
    u32 depth;
    Interval interval;
    Csv_header *header;
    Symbol_table st;
    State_machine *c1;
    State_machine *c2;

    bool process_event(u32 start, u32 finish);
};

bool Until::process_event(u32 start, u32 finish) {
    Color_rgb color = machines_debug_colors[id];
    char msg[BUFFER_SIZE];
    char pre[BUFFER_SIZE];
    
    if (parameters.debug_info) {

        u32 index;

        for (index = 0; index < (depth * 4); index++) {
            pre[index] = ' ';
        }

        pre[index] = 0;

        char lower_bound[BUFFER_SIZE] = {};
        ltl_tree_to_string_formula(interval.lower_bound, lower_bound);
        sprintf(msg, "%s[%d] ", pre, id);
        color_text(msg, color.r, color.g, color.b);

        if (interval.higher_bound){
            char higher_bound[BUFFER_SIZE] = {};
            ltl_tree_to_string_formula(interval.higher_bound, higher_bound);
            printf("<Until machine> {%s, %s}\n", lower_bound, higher_bound);
        } else {
            printf("<Until machine> {%s}\n", lower_bound);
        }

        printf("%s    [%d, %d)\n", pre, start, finish);

        pre[index++] = ' ';
        pre[index++] = ' ';
        pre[index++] = ' ';
        pre[index++] = ' ';
        pre[index] = ' ';
        pre[index + 1] = ' ';
        pre[index + 2] = ' ';
        pre[index + 3] = 0;

        print_header(header, pre);
        pre[index] = 0;
    }

    u32 c1_start = start;
    u32 c1_finish;
    bool c1_result;
    u32 c2_start;
    u32 c2_finish;
    bool c2_result;

    bool wait_p = true;
    bool result = false;
    bool lower_is_one = false;

    bool symbols_referenced = false;
    Operation_value bound_result;

    get_variable_references_from_symbol_table(interval.lower_bound, &EMPTY_SYMBOL_TABLE, true);
    get_variable_references_from_symbol_table(interval.higher_bound, &EMPTY_SYMBOL_TABLE, true);
    
    for (s32 i = start; i < finish; i++) {
        if (parameters.debug_info) {
            sprintf(msg, "%s%d: ", pre, i);
            color_text(msg, color.r, color.g, color.b);
            print_colored_message(get_message(T, i));
        }

        c1_result = false;
        c2_result = false;
        lower_is_one = false;
        

        if (wait_p && eval_tree(interval.lower_bound, &bound_result, true, true)) {
            // este check necesita ser con la tabla de simbolos vacia porque el lower bound puede ser 1
            c2_start = i;
            wait_p = false;
            lower_is_one = true;
        }

        put_line_into_symbol_table(header, get_message(T, i), &st);

        if (!symbols_referenced) {
            get_variable_references_from_symbol_table(interval.lower_bound, &st);
            get_variable_references_from_symbol_table(interval.higher_bound, &st);
            get_variable_references_from_symbol_table(header, &st);
            symbols_referenced = true;
        }

        if (wait_p) {
            if (eval_tree(interval.lower_bound, &bound_result)) {
                c1_finish = i;
                
                c1_result = execute_machine(c1, c1_start, c1_finish);

                if (c1_result) {
                    c2_start = i;
                    wait_p = false;
                } else {
                    c1_start = i;
                }
            }

            if (c1_result) {
                if (eval_tree(interval.higher_bound, &bound_result)) {
                    c2_finish = min(i + 1, finish);
                    c2_result = execute_machine(c2, c2_start, c2_finish);

                    if (c2_result) {
                        result = true;
                        break;
                    } else {
                        c1_start = i;
                        wait_p = true;
                    }
                }
            }
        } else {
            if (eval_tree(interval.higher_bound, &bound_result)) {
                if (lower_is_one) {
                    c2_finish = min(i + 1, finish);
                } else {
                    c2_finish = i;
                }
                c2_result = execute_machine(c2, c2_start, c2_finish);

                if (c2_result) {
                    result = true;
                    break;
                } else {
                    c1_start = i;
                    wait_p = true;
                }
            }
        }
    }

    return result;
}

struct State_machine {
    STATE_MACHINE_TYPE type;
    bool result;

    union {
        Formula formula;
        Not ltl_not;
        Logic logic;
        Until until;
    } as;
};

inline bool execute_machine(State_machine *state_machine, u32 start, u32 finish) {
    if (state_machine->type == MACHINE_FORMULA) {
        return state_machine->as.formula.process_event(start, finish);
    } else if (state_machine->type == MACHINE_NOT) {
        return state_machine->as.ltl_not.process_event(start, finish);
    } else if (state_machine->type == MACHINE_LOGIC) {
        return state_machine->as.logic.process_event(start, finish);
    } else if (state_machine->type == MACHINE_UNTIL) {
        return state_machine->as.until.process_event(start, finish);
    }

    return false;
}

u32 thread_index = 0;

u32 make_interval_machines(LTL_expression *tree, State_machine *machine, u32 depth=0) {
    u32 current_index;

    if (is_leaf(tree->type) || is_relop(tree->type)) {
        machine->type = MACHINE_FORMULA;
        machine->as.formula.depth = depth;

        if (is_relop(tree->type) && (tree->left->type == TOKEN_LTL_FUNCTION_CALL || tree->right->type == TOKEN_LTL_FUNCTION_CALL)) {
            machine->as.formula.function = AVERAGE;
        } else {
            machine->as.formula.function = NONE;
        }

        machine->as.formula.formula = tree;
        current_index = thread_index++;
        machine->as.formula.id = current_index;
        INSERT_BOOL_BASIC(&machine->as.formula.st, "true", 1);
        INSERT_BOOL_BASIC(&machine->as.formula.st, "false", 0);
    } else if (tree->type == TOKEN_LTL_NOT) {
        machine->as.ltl_not.c1 = GET_MEMORY(State_machine);
        make_interval_machines(tree->right, machine->as.ltl_not.c1, depth + 1);
        
        machine->type = MACHINE_NOT;
        machine->as.ltl_not.depth = depth;
        current_index = thread_index++;
        machine->as.ltl_not.id = current_index;
    } else if (is_binary_logic_operator(tree->type)) {
        machine->as.logic.c1 = GET_MEMORY(State_machine);
        make_interval_machines(tree->left, machine->as.logic.c1, depth + 1);

        machine->as.logic.c2 = GET_MEMORY(State_machine);
        make_interval_machines(tree->right, machine->as.logic.c2, depth + 1);

        machine->type = MACHINE_LOGIC;
        machine->as.logic.depth = depth;
        machine->as.logic.logic_type = tree->type;
        current_index = thread_index++;
        machine->as.logic.id = current_index;
    } else if (tree->type == TOKEN_LTL_EVENTUALLY) {
        // -<> p = 1 U p
        machine->as.until.c1 = GET_MEMORY(State_machine);
        make_interval_machines(&TREE_ONE, machine->as.until.c1, depth + 1);
        
        machine->as.until.c2 = GET_MEMORY(State_machine);
        make_interval_machines(tree->right, machine->as.until.c2, depth + 1);

        machine->type = MACHINE_UNTIL;
        machine->as.until.depth = depth;
        machine->as.until.interval = *tree->interval;
        INSERT_BOOL_BASIC(&machine->as.until.st, "true", 1);
        INSERT_BOOL_BASIC(&machine->as.until.st, "false", 0);
        current_index = thread_index++;
        machine->as.until.id = current_index;
    } else if (tree->type == TOKEN_LTL_ALWAYS) {
        // -[] p = ! <> !p
        LTL_expression *tree_always = GET_MEMORY(LTL_expression);
        tree_always->type = TOKEN_LTL_NOT;

        tree_always->right = GET_MEMORY(LTL_expression);
        tree_always->right->type = TOKEN_LTL_EVENTUALLY;
        tree_always->right->interval = tree->interval;

        tree_always->right->right = GET_MEMORY(LTL_expression);
        tree_always->right->right->type = TOKEN_LTL_NOT;
        tree_always->right->right->right = tree->right;

        current_index = make_interval_machines(tree_always, machine, depth + 1);
    } else if (tree->type == TOKEN_LTL_RELEASE) {
        // p V q = !(!p U !q)
        LTL_expression *tree_release = GET_MEMORY(LTL_expression);
        tree_release->type = TOKEN_LTL_NOT;

        tree_release->right = GET_MEMORY(LTL_expression);
        tree_release->right->type = TOKEN_LTL_UNTIL;
        tree_release->right->interval = tree->interval;

        tree_release->right->left = GET_MEMORY(LTL_expression);
        tree_release->right->left->type = TOKEN_LTL_NOT;
        tree_release->right->left->right = tree->left;

        tree_release->right->right = GET_MEMORY(LTL_expression);
        tree_release->right->right->type = TOKEN_LTL_NOT;
        tree_release->right->right->right = tree->right;

        current_index = make_interval_machines(tree_release, machine);
    } else if (tree->type == TOKEN_LTL_UNTIL) {
        machine->as.until.c1 = GET_MEMORY(State_machine);
        make_interval_machines(tree->left, machine->as.until.c1, depth + 1);
        
        machine->as.until.c2 = GET_MEMORY(State_machine);
        make_interval_machines(tree->right, machine->as.until.c2, depth + 1);
        
        machine->type = MACHINE_UNTIL;
        machine->as.until.depth = depth;
        machine->as.until.interval = *tree->interval;
        current_index = thread_index++;
        machine->as.until.id = current_index;
        INSERT_BOOL_BASIC(&machine->as.until.st, "true", 1);
        INSERT_BOOL_BASIC(&machine->as.until.st, "false", 0);
    } else {
        char msg[BUFFER_SIZE];
        sprintf(msg, "formula <%s> not suported for intervals", to_string(tree->type));
        error(msg);
    }

    return current_index;
}

void put_line_into_symbol_table(Csv_header *header, char *line, Symbol_table *st) {
    Input_buffer message_input;
    message_input.raw_buffer = line;
    message_input.count = length(line);
    message_input.reset();
    u32 index = 0;

    while (!message_input.has_ended()) {
        bool empty_field = false;
        message_input.get_next_csv_token();

        s8 sign = 1;
        bool has_pre_stuff = false;
        
        if (message_input.current_token.type == TOKEN_UNARY_MINUS) {
            sign = -1;

            if (message_input.current_char == '"') {
                message_input.report_error_in_input("unary minus with quoted string value in input data");
            }

            message_input.get_next_csv_token();

            if (message_input.current_token.type == TOKEN_STRING) {
                u32 i = length(message_input.current_token.value);

                while (i > 0) {
                    message_input.current_token.value[i] = message_input.current_token.value[i - 1];
                    i--;
                }

                message_input.current_token.value[i] = '-';
            }
        }
        
        Operation_value value;

        if (message_input.current_token.type == TOKEN_COMMA) {
            empty_field = true;

            if (header->symbol) {
                if (header->symbol->type == TOKEN_INTEGER) {
                    value.int_value = 0;
                } else if (header->symbol->type == TOKEN_FLOAT) {
                    value.float_value = 0.0;
                } else if (header->symbol->type == TOKEN_STRING) {
                    value.string_value[0] = 0;
                }
            } else {
                value.type = TOKEN_INTEGER;
                value.int_value = 0;
            }
        } else {
            if (!is_leaf(message_input.current_token.type)) {
                char msg[BUFFER_SIZE];
                sprintf(msg, "'%s'. Data read from csv line does not consist of any of the basic types supported (int, float, string, date), got %s <%s>", line, message_input.current_token.value, to_string(message_input.current_token.type));
                message_input.report_error_in_input(msg);
            }

            eval_csv_factor(&message_input, &value);
        }

        if (value.type == TOKEN_INTEGER) {
            value.int_value = sign * value.int_value;
        
            if (header->symbol) {
                if (header->symbol->type != TOKEN_INTEGER) {
                    char msg[BUFFER_SIZE];
                    sprintf(msg, "column '%s' changed type from <%s> to <%s> with the value: %lld", header->name, to_string(header->symbol->type), to_string(TOKEN_INTEGER), value.int_value);
                    message_input.report_error_in_input(msg, false);
                }

                header->symbol->int_value = value.int_value;
                header = header->next;
            } else {
                put_symbol(st, get_header(header, index++), TOKEN_INTEGER, value.int_value);
            }
        } else if (value.type == TOKEN_FLOAT) {
            value.float_value = ((f64)sign) * value.float_value;

            if (header->symbol) {
                if (header->symbol->type != TOKEN_FLOAT) {
                    char msg[BUFFER_SIZE];
                    sprintf(msg, "column '%s' changed type from <%s> to <%s> with the value: %f", header->name, to_string(header->symbol->type), to_string(TOKEN_FLOAT), value.float_value);
                    message_input.report_error_in_input(msg, false);
                }

                header->symbol->float_value = value.float_value;
                header = header->next;
            } else {
                put_symbol(st, get_header(header, index++), TOKEN_FLOAT, value.float_value);
            }
        } else if (value.type == TOKEN_STRING) {
            if (header->symbol) {
                if (header->symbol->type != TOKEN_STRING) {
                    char msg[BUFFER_SIZE];
                    sprintf(msg, "column '%s' changed type from <%s> to <%s> with the value: %s", header->name, to_string(header->symbol->type), to_string(TOKEN_STRING), value.string_value);
                    message_input.report_error_in_input(msg, false);
                }

                strcpy(header->symbol->string_value, value.string_value);
                header = header->next;
            } else {
                put_symbol(st, get_header(header, index++), TOKEN_STRING, value.string_value);
            }
        }

        if (message_input.has_ended()) {
            if (message_input.current_token.type != TOKEN_EOF && !empty_field) {
                message_input.report_error_in_input("unable to finish the input file processing");
            }
        } else {
            if (message_input.current_token.type != TOKEN_COMMA) {
                char msg[BUFFER_SIZE];
                sprintf(msg, "expected comma ',' in input file got '%s' <%s>", message_input.current_token.value, to_string(message_input.current_token.type));
                message_input.report_error_in_input(msg);
            }
        }
    }
}

void print_path_not_found_information(char *file_path) {
    char sub_paths[FOLDERS_COUNT][FOLDERS_NAME_LENGTH] = {};
    char sub_path_to_test[FOLDERS_COUNT * FOLDERS_NAME_LENGTH] = {};
    char suggestion[FOLDERS_NAME_LENGTH] = {};
    u32 sub_path_index_i = 0;
    u32 sub_path_index_j = 0;
    u32 file_path_index = 0;
    u32 file_path_length = length(file_path);
    char c;

    while (file_path_index < file_path_length) {
        c = file_path[file_path_index];

        if (c == '/' || c == '\\') {
            sub_paths[sub_path_index_i][sub_path_index_j] = 0;
            sub_path_index_i++;
            sub_path_index_j = 0;
        } else {
            sub_paths[sub_path_index_i][sub_path_index_j] = c;
            sub_path_index_j++;
        }
        
        file_path_index++;
    }

    u32 test_i = 0;
    u32 i = 0;
    u32 j = 0;
    u32 first_wrong = 0;
    u32 last_that_existed_index = 0;
    c = sub_paths[test_i][j++];

    while (c != 0) {
        sub_path_to_test[i++] = c;
        c = sub_paths[test_i][j++];
    }
    
    sub_path_to_test[i++] = '/';

    test_i++;
    j = 0;

    while (directory_exists(sub_path_to_test)) {
        c = sub_paths[test_i][j++];
        last_that_existed_index = i;

        while (c != 0) {
            sub_path_to_test[i++] = c;
            c = sub_paths[test_i][j++];
        }

        sub_path_to_test[i++] = '/';

        first_wrong = test_i;
        test_i++;
        j = 0;
    }

    sub_path_to_test[last_that_existed_index] = 0;
    
    printf("\nNow the first part where the path starts being wrong is highlighted on the screen:\n");
    printf("\n    ");
    GREEN(sub_path_to_test);

    for (u32 i = first_wrong; i <= sub_path_index_i; i++) {
        RED(sub_paths[i]);

        if (i != sub_path_index_i) RED("/");
    }

    printf("\n");

#if !CONSOLE_COLORS
    u32 start = length((char *)sub_path_to_test) + 4;
    u32 end = length((char *)sub_paths[first_wrong]);

    for (u32 m = 0; m < (file_path_length + 4); m++) {
        if (m < start) {
            printf(" ");
        } else {
            printf("^");
        }
    }
    
    printf("\n");
#endif

    char similar_paths[10][256] = {};
    u32 similar_count = has_similar_paths(sub_path_to_test, sub_paths[first_wrong], similar_paths);

    if (similar_count > 0) {
        printf("\nThese are similar paths that exist:\n");

        for (i = 0; i < similar_count; i++) {
            printf("    ");
            GREEN(sub_path_to_test);
            YELLOW_GREEN(similar_paths[i]);
            printf("\n");
        }
    }
}

u32 tree_max_depth(LTL_expression *tree) {
    u32 left = 0;
    u32 right = 0;

    if (is_leaf(tree->type)) {
        return 1;
    }

    if (is_unary_operator(tree->type)) {
        right = tree_max_depth(tree->right);
        return right + 1;
    }

    if (is_binary_operator(tree->type) || is_relop(tree->type) || is_binary_aritmetic_operator(tree->type)) {
        left = tree_max_depth(tree->left);
        right = tree_max_depth(tree->right);
        return max(left, right) + 1;
    }

    char msg[BUFFER_SIZE];
    sprintf(msg, "tree_max_depth le falta un tipo de nodo por comprobar: <%s>", to_string(tree->type));
    error(msg);

    return -1;
}

void line_to_header(char *line, State_machine *state_machine) {
    if (state_machine->type == MACHINE_FORMULA) {
        state_machine->as.formula.header = GET_MEMORY(Csv_header);
        line_to_header(line, state_machine->as.formula.header);
    } else if (state_machine->type == MACHINE_NOT) {
        line_to_header(line, state_machine->as.ltl_not.c1);
    } else if (state_machine->type == MACHINE_LOGIC) {
        line_to_header(line, state_machine->as.logic.c1);
        line_to_header(line, state_machine->as.logic.c2);
    } else if (state_machine->type == MACHINE_UNTIL) {
        state_machine->as.until.header = GET_MEMORY(Csv_header);
        line_to_header(line, state_machine->as.until.header);

        line_to_header(line, state_machine->as.until.c1);
        line_to_header(line, state_machine->as.until.c2);
    }
}

s32 main(s32 argc, char *argv[]) {
    // DOWNLOAD ltl2ba: http://www.lsv.fr/~gastin/ltl2ba/ltl2ba-1.3.tar.gz

    /*
    java + c:
        http://malinsky.eu/blog/how-to-call-a-c-function-from-java/
    */
#if CONSOLE_COLORS
    setup_console();
#endif
    u64 program_time = get_time_microseconds();

    u32 p = 1;

    if (argc > 0) {
        while (p < argc) {
            if (equals(argv[p], "-f") || equals(argv[p], "--formula")) {
                if (parameters.formula) {
                    printf("--formula already set");
                    return -1;
                }

                parameters.formula = true;
                ++p;

                if (p >= argc) {
                    printf("--formula expects a value after the parameter");
                    return -1;
                }

                parameters.f_value = argv[p];
            } else if (equals(argv[p], "-i") || equals(argv[p], "--input_file")) {
                if (parameters.input_file) {
                    printf("--input_file already set");
                    return -1;
                }

                parameters.input_file = true;
                ++p;

                if (p >= argc) {
                    printf("--input_file expects a value after the parameter");
                    return -1;
                }

                parameters.i_value = argv[p];
            } else if (equals(argv[p], "-d") || equals(argv[p], "--debug_info")) {
                if (parameters.debug_info) {
                    printf("--debug already set");
                    return -1;
                }

                parameters.debug_info = true;
            } else if (equals(argv[p], "-e") || equals(argv[p], "--elapsed")) {
                if (parameters.elapsed) {
                    printf("--elapsed already set");
                    return -1;
                }

                parameters.elapsed = true;
            } else if (equals(argv[p], "-t") || equals(argv[p], "--tutorial")) {
                if (parameters.tutorial) {
                    printf("--tutorial already set");
                    return -1;
                }

                parameters.tutorial = true;
            } else if (equals(argv[p], "-h") || equals(argv[p], "--help")) {
                if (parameters.help) {
                    printf("--help already set");
                    return -1;
                }

                parameters.help = true;
            } else {
                printf("parameter '%s' not recognized, this program only accepts:\n", argv[p]);
                parameters.help = true;
                break;
            }

            ++p;
        }
    }

    if (parameters.tutorial || argc == 1) {
        printf("This interpreter accepts linear temporal logic formulas ([] x==1, (<> y > 0) || ([] y < -2), as well as interval temporal logic ([]_{1}x == 1, <>_{y>4}true).\n");
        printf("The basic data types are integer, float, string and date.\n");
        printf("The string data must be enclosed between single quotes ' or escaped double quotes \".\n");
        printf("Dates must be in UTC format YYYY-MM-DDThh:mm:ss.ms with milliseconds(ms) being 3 digits.\n");
        printf("The input data is interpreted as a csv file with only ascii character allowed.\n");
        printf("You can apply the average function to an integer or float variable like this avg(x) ([]_{x>=1, x>=10} avg(y) >= 3).\n");
        printf("This is an example of the usage of the program: ./ltl_interpreter.exe --formula \"[]_{status == 'running'} avg(y) >= 3\" --input_file \"../my_folder/../my_file.csv\"\n");
        printf("All the accepted commands are shown below:\n");
        printf("\n");
        parameters.help = true;
    }

    if (parameters.help) {
        printf("    -f, --formula <formula>: this is to introduce the formula to evaluate, it expects a value enclosed in double quotes like this \"freq==2 U freq > 10\"\n");
        printf("    -i, --input_file <path_to_file>: this is to set the path to the input file with the traces you want to test the formula\n");
        printf("    -d, --debug_info: shows debug information about the process of evaluating the formula over the input file\n");
        printf("    -e, --elapsed: shows the elapsed time\n");
        printf("    -h, --help: shows the help information\n");
        printf("    -t, --tutorial: shows a little tutorial of the program\n");
        return 0;
    }

    memory_arena mem;
    mem.size = 20 * MEGABYTE;
    mem.base = (u8 *)calloc(1, mem.size);
    mem.index = 0;

    ARENA = &mem;

    Symbol_table st;
    st.table = 0;
    st.next_guess = 0;
    SYMBOL_TABLE = &st;
    INSERT_BOOL_BASIC(SYMBOL_TABLE, "true", 1);
    INSERT_BOOL_BASIC(SYMBOL_TABLE, "false", 0);
    INSERT_BOOL_BASIC(&EMPTY_SYMBOL_TABLE, "true", 1);
    INSERT_BOOL_BASIC(&EMPTY_SYMBOL_TABLE, "false", 0);

    char *file_path;

    if (parameters.input_file) {
        file_path = parameters.i_value;
    } else {
        error("input file not introduced, please introduce it with the flag -i, --input_file <path_to_file> enclosed with double quotes like this \"../path/to/file.csv\"");
    }

    Input_buffer streaming = {};
    bool file_read_sucess = read_entire_file(&streaming, file_path);

    if (!file_read_sucess) {
        print_path_not_found_information(file_path);
        return -1;
    }

    streaming.reset();

    if (length(streaming.raw_buffer) <= 0) {
        char msg[BUFFER_SIZE];
        sprintf(msg, "'%s' is an empty file", file_path);
        error(msg);
    }
    
    char t[STRING_LENGTH];
    t[0] = 0;
    char *line = (char *)t;
    get_csv_line(&streaming, line);
    strcpy(header_string, line);
    HEADER = GET_MEMORY(Csv_header);
    line_to_header(line, HEADER);
    u32 header_length = length(HEADER);
    csv_debug_colors = GET_MEMORY_COUNT(Color_rgb, sizeof(Color_rgb) * header_length);

    for (u32 i = 0; i <= header_length; i++) {
        f32 H = (f32)generate_angle(header_length, i);
        HSVtoRGB(H, 70, 100, &csv_debug_colors[i]);
    }

    char *input_formula;

    if (parameters.formula) {
        input_formula = parameters.f_value;
    } else {
        error("formula not introduced, please introduce it with the flag -f, --formula <formula> enclosed with double quotes like this \"<>x==1\"");
    }
    
    STACK_INPUT_BUFFER(ltl, input_formula);

    LTL_expression *ast = ltl_expression(&ltl);

    u32 tree_depth = tree_max_depth(ast);
    tree_debug_colors = GET_MEMORY_COUNT(Color_rgb, sizeof(Color_rgb) * tree_depth);

    for (u32 i = 0; i < tree_depth; i++) {
        f32 H = (f32)generate_angle(tree_depth, i);
        HSVtoRGB(H, 70, 100, &tree_debug_colors[i]);
    }

    if (ltl.current_index < ltl.count || ltl.current_token.type != TOKEN_EOF) {
        print_ltl_tree(SYMBOL_TABLE, ast);
        char msg[BUFFER_SIZE];
        sprintf(msg, "not able to completely process the entire formula, last token processed was: '%s' <%s>", ltl.current_token.value, to_string(ltl.current_token.type));
        ltl.report_syntax_error(msg, ltl.current_token.position);
    }


    u32 wait_time = 0;
    State_machine *state_machines = GET_MEMORY(State_machine);

    bool intervals = has_intervals(ast);
    u32 result_index;
    u32 steps_to_skip;
    LTL_graph *graph;

    T = (Trace *)malloc(sizeof(Trace));
    T->first_free = 0;
    T->next = 0;

    if (parameters.debug_info) {
        printf("---------------------------------");
        color_text("BEFORE EVALUATION", 230, 230, 100);
        printf("---------------------------------\n\n");
    }

    if (parameters.debug_info) {
        printf("input formula: ");
        color_text(ltl.raw_buffer, 255, 255, 0);
        putchar('\n');
        putchar('\n');
        print_ltl_tree(SYMBOL_TABLE, ast);
    }

    if (intervals) {
        if (has_nexts(ast)) {
            error("next operand 'O' not allowed for interval formulas\n");
        }

        if (!every_node_able_to_have_interval_has_interval(ast)) {
            error("combination of LTL formulas with and without intervals is not suported, please introduce one at a time. Only the LTL operands [], <>, U, V can have intervals\n");
        }

        result_index = make_interval_machines(ast, state_machines);
        line_to_header(line, state_machines);

        machines_debug_colors = GET_MEMORY_COUNT(Color_rgb, sizeof(Color_rgb) * (result_index + 1));

        for (u32 i = 0; i < (result_index + 1); i++) {
            f32 H = (f32)generate_angle((result_index + 1), i);
            HSVtoRGB(H, 50, 65, &machines_debug_colors[i]);
        }
    } else {
        steps_to_skip = count_consecutive_next_in_single_operations(ast);

        if (steps_to_skip > 0) {
            trim_consecutive_next(ast);
            assert_msg(count_consecutive_next_in_single_operations(ast) == 0, "the next states triming is not working properly");
        }

        replace_formulas(ast);

        char formula[BUFFER_SIZE] = {};
        ltl_tree_to_string_formula(ast, formula);
        // DONT DELETE THIS
        tl_out = stdout;

        strcpy(uform, formula);
        hasuform = length(uform);
        final_never_claim[0] = 0;
        tl_parse();

        Input_buffer never_claim_process;
        never_claim_process.raw_buffer = (char *)&final_never_claim;
        never_claim_process.count = length(never_claim_process.raw_buffer);
        never_claim_process.reset();

        graph = create_graph(&never_claim_process);
        
        if (parameters.debug_info) {
            printf("\nreplacing formulas\n\nafter\n");
            print_ltl_tree(SYMBOL_TABLE, ast);
            printf("\nltl2ba formated formula: ");
            color_text(formula, 255, 255, 0);
            printf("\n\nBuchi automaton never claims\n");
            print_graph_debug_info(graph);
            printf("\n");
        }
    }

    if (parameters.debug_info) {
        print_header(HEADER, "\nheader of input file: ");
        printf("\n---------------------------------");
        color_text("START EVALUATION", 230, 230, 100);
        printf("---------------------------------\n");
    }

    bool symbols_referenced = false;
    u32 message_read_index = 0;

    while (!streaming.has_ended()) {
        get_csv_line(&streaming, line);

        if (streaming.current_char == '\n') {
            streaming.eat_char();
            streaming.get_char();
        }

        enqueue(T, line);

        if (!intervals) {
            if (!symbols_referenced) {
                put_line_into_symbol_table(HEADER, line, SYMBOL_TABLE);
                get_variable_references_from_symbol_table(ast, SYMBOL_TABLE);
                get_variable_references_from_symbol_table(HEADER, SYMBOL_TABLE);
                symbols_referenced = true;
            }
        }

        message_read_index++;
    }

    bool result;

    if (intervals) {
        result = execute_machine(state_machines, 0, length(T));
    } else {
        result = graph->evaluate_step(parameters.debug_info, T, steps_to_skip);
    }

    if (parameters.debug_info) {
        printf("---------------------------------");
        color_text("END EVALUATION", 230, 230, 100);
        printf("---------------------------------\n");
    }

    printf("\n    %s", ltl.raw_buffer);
    printf(": ");
    if (result) GREEN(to_string(result));
    else RED(to_string(result));
    printf("\n\n");

    u64 elapsed_us = get_time_microseconds() - program_time;
    f64 lps = (f64)message_read_index / (((f64)elapsed_us)/1000000.0);
    char *size_scale;
    char *speed_scale;

    if (parameters.debug_info || parameters.elapsed) {
        if (elapsed_us > 1000000) {
            printf("\nELAPSED: %.2fs, lines processed: %d, %.2f lines per second\n", ((f64)elapsed_us) / 1000000.0, message_read_index, lps);
        } else {
            printf("\nELAPSED: %.2fms, lines processed: %d, %.2f lines per second\n", ((f64)elapsed_us) / 1000.0, message_read_index, lps);
        }

        f64 size = (f64)streaming.count;
        f64 speed = (f64)streaming.count / (((f64)elapsed_us) / 1000000.0f);

        if (size < KILOBYTE) {
            size_scale = "B";
        } else if (size < MEGABYTE) {
            size_scale = "KB";
            size = size / KILOBYTE;
        } else if (size >= MEGABYTE) {
            size_scale = "MB";
            size = size / MEGABYTE;
        } else {
            size_scale = "GB";
            size = size / GIGABYTE;
        }

        if (speed < KILOBYTE) {
            speed_scale = "B";
        } else if (speed < MEGABYTE) {
            speed_scale = "KB";
            speed = speed / KILOBYTE;
        } else if (speed >= MEGABYTE) {
            speed_scale = "MB";
            speed = speed / MEGABYTE;
        } else {
            speed_scale = "GB";
            speed = speed / GIGABYTE;
        }
        
        printf("Size processed: %.2f%s; %.2f %s/s\n", 
            size, 
            size_scale,
            speed,
            speed_scale
        );
    }

    free(streaming.raw_buffer);
    free(mem.base);
    free_trace(T);

#if CONSOLE_COLORS
    restore_console();
#endif
    return 0;
}
