struct Token_position {
    u32 p0;
    u32 pf;
};

struct Token {
    LTL_TOKEN_TYPE type;
    Token_position position;
    char value[STRING_LENGTH];
};

struct Input_buffer {
    char *raw_buffer;
    u32 current_index;
    u64 count;
    char current_char;
    Token current_token;

    inline void get_char();
    inline void eat_char();
    inline void rewind_char();
    void skip_whitespaces(bool new_line_too);
    void skip_carrier_return();
    void reset(bool skip);
    void get_number_or_date();
    void get_next_token();
    void get_next_csv_token();
    bool is_date(bool show_error);
    bool has_ended();
    inline void print_error_message(char *message, bool highlight_input);
    inline void print_syntax_error_message(char *message, Token_position position);
    inline void print_error_message_unbalanced_parenthesis(char *message);
    void report_error_in_input(char *message, bool highlight_input);
    void report_lexical_error(char *message);
    void report_syntax_error(char *message, Token_position position);
    void report_syntax_error_unbalanced_parenthesis(char *message);
};

inline void Input_buffer::get_char() {
    if (raw_buffer) {
        current_char = raw_buffer[current_index];
    } else {
        error("in get_char, no buffer given");
    }
}

inline void Input_buffer::eat_char() {
    current_index++;
}

inline void Input_buffer::rewind_char() {
    assert_msg(current_index > 0, "trying to unwind a 0 cursor\n");
    current_index--;
}

void Input_buffer::skip_carrier_return() {
    while (current_char == '\r') {
        eat_char();
        get_char();
    }
}

void Input_buffer::skip_whitespaces(bool new_line_too = true) {
    if (new_line_too) {
        while (is_whitespace(current_char) || is_new_line(current_char) || current_char == '\r') {
            eat_char();
            get_char();
        }
    } else {
        while (is_whitespace(current_char) || current_char == '\r') {
            eat_char();
            get_char();
        }
    }
}

void Input_buffer::print_error_message(char *message, bool highlight_input=true) {
    printf(": %s\n", message);

    putchar('\n');
    putchar(' ');
    putchar(' ');
    putchar(' ');
    putchar(' ');
    putchar(' ');

    for (u32 i = 0; i < count; i++) {
        if (i == (current_index - 0) && highlight_input) {
            RED((char)raw_buffer[i]);
        } else {
            putchar(raw_buffer[i]);
        }
    }

    putchar('\n');

    if (highlight_input) {
        for (int i = 0; i < (current_index + 5); i++) {
            putchar('-');
        }

        putchar('^');
        putchar('\n');
        putchar('\n');
    }

    exit(-1);
}

void Input_buffer::print_syntax_error_message(char *message, Token_position position={}) {
    printf(": %s\n", message);

    putchar('\n');
    putchar(' ');
    putchar(' ');
    putchar(' ');
    putchar(' ');
    putchar(' ');

    for (u32 i = 0; i < count; i++) {
        if (i >= position.p0 && i <= position.pf) {
            RED(raw_buffer[i]);
        } else {
            putchar(raw_buffer[i]);
        }
    }

    putchar('\n');

    for (u32 i = 0; i < (5 + position.p0); i++) {
        putchar('-');
    }

    for (u32 i = position.p0; i <= position.pf; i++) {
        putchar('^');

    }

    putchar('\n');
    putchar('\n');

    exit(-1);
}

void Input_buffer::print_error_message_unbalanced_parenthesis(char *message) {
    printf(": %s\n", message);

    putchar('\n');
    putchar(' ');
    putchar(' ');
    putchar(' ');
    putchar(' ');
    putchar(' ');

    u32 unbalanced_index = 0;
    s32 balance = 0;

    for (s32 i = count - 1; i >= 0; i--) {
        if (raw_buffer[i] == ')') {
            balance--;
        }

        if (raw_buffer[i] == '(') {
            balance++;
            if (balance > 0) {
                unbalanced_index = i;
            }
        }
    }

    for (u32 i = 0; i < count; i++) {
        if (i == unbalanced_index) {
            RED((char)raw_buffer[i]);
        } else if (i == (current_index - 0)) {
            RED((char)raw_buffer[i]);
        } else {
            putchar(raw_buffer[i]);
        }
    }

    putchar('\n');

    for (int i = 0; i < (current_index + 5); i++) {
        if (i == (unbalanced_index + 5)) {
            putchar('^');
        } else {
            putchar('-');
        }
    }

    putchar('^');
    putchar('\n');
    putchar('\n');

    exit(-1);
}

void Input_buffer::report_error_in_input(char *message, bool highlight_input=true) {
    putchar('\n');
    RED("ERROR");
    print_error_message(message, highlight_input);
}

void Input_buffer::report_lexical_error(char *message) {
    putchar('\n');
    RED("LEXICAL ERROR");
    print_error_message(message);
}

void Input_buffer::report_syntax_error(char *message, Token_position position={}) {
    putchar('\n');
    RED("SYNTAX ERROR");
    print_syntax_error_message(message, position);
}

void Input_buffer::report_syntax_error_unbalanced_parenthesis(char *message) {
    putchar('\n');
    RED("SYNTAX ERROR");
    print_error_message_unbalanced_parenthesis(message);
}

void Input_buffer::reset(bool skip=true) {
    current_index = 0;
    get_char();
    if (skip) skip_whitespaces();
}

inline bool Input_buffer::has_ended() {
    return current_index >= count;
}

struct Interval {
    LTL_expression *lower_bound;
    LTL_expression *higher_bound;
};

struct LTL_expression {
    LTL_TOKEN_TYPE type;
    LTL_expression *left = NULL;
    LTL_expression *right = NULL;
    char formula[VARIABLE_NAME_LENGTH];
    Interval *interval;
    bool is_from_parenthesis;
    LTL_symbol *symbol;
};

struct Operation_value {
    LTL_TOKEN_TYPE type;
    union {
        s64 int_value;
        f64 float_value;
        char string_value[STRING_LENGTH];
    };
};

void print(Operation_value *value) {
    if (value->type == TOKEN_INTEGER) {
        printf("%lld", value->int_value);
    } else if (value->type == TOKEN_FLOAT) {
        printf("%.2f", value->float_value);
    } else if (value->type == TOKEN_STRING) {
        printf("%s", value->string_value);
    }
}

#define SECONDS_PER_YEAR 31557600
#define SECONDS_PER_DAY 86400

u64 date_to_timestamp(char *date) {
    // number of milliseconds between date and 1/1/1970.
    // Format accepted: YYYY-MM-DDThh:mm:ss.ms (eg 1997-07-16T19:20:30.450)
    // 0123456789012345678901
    // YYYY-MM-DDThh:mm:ss.ms
    // https://www.w3.org/TR/NOTE-datetime
    // https://en.wikipedia.org/wiki/Unix_time
    char msg[BUFFER_SIZE];
    u32 days_per_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    u64 result = 0;

    date[4] = 0;
    u32 YYYY = to_int(date);

    if (YYYY < 1970) {
        sprintf(msg, "in date, year was %d and must be higher or equal to 1970(its the year when we start counting the microseconds)", YYYY);
        error(msg);
    }

    date[7] = 0;
    u32 MM = to_int(date + 5);

    if (MM < 1 || MM > 12) {
        sprintf(msg, "in date, month was %d and must be between 01 and 12", MM);
        error(msg);
    }

    date[10] = 0;
    u32 DD = to_int(date + 8);

    if (LEAP_YEAR(YYYY)) {
        if (MM == 2) {
            if (DD < 1 || DD > (days_per_month[MM - 1] + 1)) {
                sprintf(msg, "in date, day was %d and must be between 01 and 29 for month 2", DD);
                error(msg);
            }
        } else {
            if (DD < 1 || DD > (days_per_month[MM - 1] + 1)) {
                sprintf(msg, "in date, day was %d and must be between 01 and %d for month %d", DD, days_per_month[MM - 1], MM);
                error(msg);
            }
        }
    } else {
        if (DD < 1 || DD > (days_per_month[MM - 1] + 1)) {
            sprintf(msg, "in date, day was %d and must be between 01 and %d for month %d", DD, days_per_month[MM - 1], MM);
            error(msg);
        }
    }

    date[13] = 0;
    u32 hh = to_int(date + 11);

    if (hh < 0 || hh > 23) {
        sprintf(msg, "in date, hour was %d and must be between 00 and 23", hh);
        error(msg);
    }

    date[16] = 0;
    u32 mm = to_int(date + 14);

    if (mm < 0 || mm > 59) {
        sprintf(msg, "in date, minutes was %d and must be between 00 and 59", mm);
        error(msg);
    }

    date[19] = 0;
    u32 ss = to_int(date + 17);

    if (ss < 0 || ss > 59) {
        sprintf(msg, "in date, seconds was %d and must be between 00 and 59", ss);
        error(msg);
    }

    u32 ms = to_int(date + 20);

    if (ms < 0 || ms > 999) {
        sprintf(msg, "in date, milliseconds was %d and must be between 00 and 999", ms);
        error(msg);
    }

    result = (YYYY - 1970) * SECONDS_PER_YEAR;

    for (u32 i = 1970; i < YYYY; i++) {
        if (LEAP_YEAR(i)) {
          result += SECONDS_PER_DAY;
        }
    }

    for (u32 j = 1; j <= MM ; j++) {
        if ((j == 2) && LEAP_YEAR(YYYY)) {
            result += SECONDS_PER_DAY * 29;
        } else {
            result += SECONDS_PER_DAY * days_per_month[j - 1];
        }
    }

    result += SECONDS_PER_DAY * (DD - 1);
    result += hh * 60 * 60 ;
    result += mm * 60;
    result += ss;

    result *= 1000;
    result += ms;

    return result;
}

LTL_expression TREE_ONE = {TOKEN_INTEGER, NULL, NULL, "1", NULL};

#define STACK_INPUT_BUFFER(name, buffer) Input_buffer name = {};\
        name.raw_buffer = buffer;\
        name.count = length(buffer);\
        name.reset();\
        name.get_next_token();\

struct Csv_header {
    char *name;
    LTL_symbol *symbol;
    Csv_header *next;
};

u32 length(Csv_header *header) {
    u32 result = 0;

    while (header) {
        result++;
        header = header->next;
    }

    return result;
}

void print_header(Csv_header *header, char *pre="") {
    printf("%s", pre);
    u32 index = 0;

    while (header) {
        Color_rgb color = csv_debug_colors[index];
        u32 r = color.r;
        u32 g = color.g;
        u32 b = color.b;
        color_text(header->name, r, g, b);
        putchar(',');
        putchar(' ');
        index++;
        header = header->next;
    }

    putchar('\n');
}

Csv_header *HEADER;
char header_string[BUFFER_SIZE] = {};

char *get_header(Csv_header *header, u32 index) {
    u32 index_to_print = index;
    Csv_header *to_print = header;
    u64 chars_count = 0;

    while (index > 0) {
        chars_count += length(header->name);
        --index;

        if (header->next == NULL) {
            // error("getting csv header column name, index queried out of bounds");
            putchar('\n');
            RED("    ERROR");
            printf(": missing header value for input (there are more value columns than headers, %d was the index queried)\n", index_to_print);
            putchar('\n');
            putchar(' ');
            putchar(' ');
            putchar(' ');
            putchar(' ');
            printf("header: ");
            print_header(to_print);
            char msg[BUFFER_SIZE];
            u32 i;

            for (i = 0; i < chars_count; i++) {
                msg[i] = ' ';
            }

            msg[i++] = ' ';
            msg[i++] = ' ';
            msg[i++] = ' ';
            msg[i++] = ' ';
            msg[i++] = ' ';
            msg[i++] = ' ';
            msg[i++] = ' ';
            msg[i++] = ' ';
            msg[i++] = ' ';
            msg[i++] = ' ';
            msg[i++] = ' ';
            msg[i++] = ' ';
            msg[i++] = ' ';
            msg[i++] = ' ';
            msg[i++] = ' ';
            msg[i++] = '^';
            msg[i++] = '^';
            msg[i++] = '^';
            msg[i++] = '^';
            msg[i++] = '^';
            msg[i++] = '\n';
            RED(msg);
            exit(-1);
        }

        header = header->next;
    }

    return header->name;
}

void get_csv_line(Input_buffer *streaming, char *line) {
    u32 i = 0;
    local_persist u32 line_number = 0;
    bool quoted = false;

    while (!streaming->has_ended() && streaming->current_char != '\n') {
        if (streaming->current_char == '"') quoted = true;
        
        while (quoted && !streaming->has_ended()) {
            line[i++] = streaming->current_char;
            streaming->eat_char();
            streaming->get_char();
            streaming->skip_carrier_return();

            if (i > STRING_LENGTH) {
                char msg[BUFFER_SIZE];
                sprintf(msg, "csv line was longer than supported (current limit %d Bytes) please to use the interpreter with longer line sizes change the flag STRING_SIZE to a bigger number in one of the build files and recompile the interpreter\nthe line was: %d\n\n%s", STRING_LENGTH, line_number, line);
                error(msg);
            }
            
            if (streaming->current_char == '"') quoted = false;
        }

        line[i++] = streaming->current_char;
        streaming->eat_char();
        streaming->get_char();
        streaming->skip_carrier_return();

        if (i > STRING_LENGTH) {
            char msg[BUFFER_SIZE];
            sprintf(msg, "csv line was longer than supported (current limit %d Bytes) please to use the interpreter with longer line sizes change the flag STRING_SIZE to a bigger number in one of the build files and recompile the interpreter\nthe line was: %d\n%s", STRING_LENGTH, line_number, line);
            error(msg);
        }
    }
    
    line[i] = 0;
    
    if (!streaming->has_ended()) {
        streaming->eat_char();
        streaming->get_char();
    }

    line_number++;
}

void line_to_header(char *line, Csv_header *header) {
    Csv_header *t = header;
    Input_buffer h;
    h.raw_buffer = line;
    h.count = length(line);
    h.reset();

    while (!h.has_ended()) {
        h.get_next_csv_token();

        t->name = GET_MEMORY_COUNT(char, 256);
        strcpy(t->name, h.current_token.value);
        h.get_next_csv_token();

        if (h.current_token.type != TOKEN_COMMA && !h.has_ended()) {
            error("in csv file parsing, header not separated with comma");
        }

        if (!h.has_ended()) {
            t->next = GET_MEMORY(Csv_header);
            t = t->next;
            t->next = NULL;
        }
    }
}

bool Input_buffer::is_date(bool show_error=true) {
    // YYYY-MM-DDThh:mm:ss.ms
    u32 last_index = current_index;

    if (current_char != '-') {
        current_index = last_index;
        get_char();
        return false;
    }

    eat_char();
    get_char();

    if (!is_number(current_char)) {
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();
    
    if (!is_number(current_char)) {
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();

    if (current_char != '-') {
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();
    
    if (!is_number(current_char)) {
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();
    
    if (!is_number(current_char)) {
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();

    if (current_char != 'T') {
        if (is_alpha(current_char) && show_error) report_lexical_error("missing 'T' for the time separator in date type");
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();
    
    if (!is_number(current_char)) {
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();
    
    if (!is_number(current_char)) {
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();

    if (current_char != ':') {
        if (show_error) report_lexical_error("missing ':' for the time separator in date type");
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();
    
    if (!is_number(current_char)) {
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();
    
    if (!is_number(current_char)) {
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();

    if (current_char != ':') {
        if (show_error) report_lexical_error("missing ':' for the time separator in date type");
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();
    
    if (!is_number(current_char)) {
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();
    
    if (!is_number(current_char)) {
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();

    if (current_char != '.') {
        if (show_error) report_lexical_error("missing '.' for the time separator in date type");
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();
    
    if (!is_number(current_char)) {
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();
    
    if (!is_number(current_char)) {
        current_index = last_index;
        get_char();
        return false;
    }
    
    eat_char();
    get_char();
    
    if (!is_number(current_char)) {
        current_index = last_index;
        get_char();
        return false;
    }

    current_index = last_index;
    get_char();
    return true;
}

void Input_buffer::get_number_or_date() {
    u32 result_index = 0;

    while (is_number(current_char)) {
        current_token.value[result_index++] = current_char;
        eat_char();
        get_char();
    }

    // if (current_char == '-') {
    if (is_date()) {
        current_token.value[result_index++] = current_char;
        eat_char();
        get_char();

        if (!is_number(current_char)) {
            // Is not a date the - char is for other token, for example '->'
            rewind_char();
            get_char();
            result_index--;
            current_token.type = TOKEN_INTEGER;
        } else {
            //     v
            // YYYY-MM-DDThh:mm:ss.ms
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            //        v
            // YYYY-MM-DDThh:mm:ss.ms
            if (current_char != '-') {
                eat_char();
                report_lexical_error("missing '-' for the day in date type");
            }
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            //           v
            // YYYY-MM-DDThh:mm:ss.ms
            if (current_char != 'T') {
                eat_char();
                report_lexical_error("missing 'T' for the time separator in date type");
            }
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            //              v
            // YYYY-MM-DDThh:mm:ss.ms
            if (current_char != ':') {
                eat_char();
                report_lexical_error("missing ':' for minutes in date type");
            }
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            //                 v
            // YYYY-MM-DDThh:mm:ss.ms
            if (current_char != ':') {
                eat_char();
                report_lexical_error("missing ':' for seconds in date type");
            }
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            //                    v
            // YYYY-MM-DDThh:mm:ss.ms
            if (current_char != '.') {
                eat_char();
                report_lexical_error("missing '.' for milliseconds in date type");
            }
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            
            if (!is_number(current_char)) {
                eat_char();
                report_lexical_error("number for milliseconds in date type must have 3 characters (for example: 543, 035, 002, 000)");
            }
            
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
            
            if (!is_number(current_char)) {
                eat_char();
                report_lexical_error("number for milliseconds in date type must have 3 characters (for example: 543, 035, 002, 000)");
            }

            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();

            if (is_number(current_char)) {
                eat_char();
                report_lexical_error("number for milliseconds in date type must have 3 characters (for example: 543, 035, 002, 000)");
            }

            current_token.type = TOKEN_DATE;
        }
    } else if (current_char == '.') {
        current_token.value[result_index++] = current_char;
        eat_char();
        get_char();
        current_token.type = TOKEN_FLOAT;
    } else {
        current_token.type = TOKEN_INTEGER;
    }

    while (is_number(current_char)) {
        current_token.value[result_index++] = current_char;
        eat_char();
        get_char();
    }

    current_token.value[result_index] = 0;
}

char *token_type_to_symbol(LTL_TOKEN_TYPE type) {
    if (type == TOKEN_LTL_UNTIL) return "U";
    if (type == TOKEN_LTL_RELEASE) return "V";
    if (type == TOKEN_LTL_NEXT) return "X";
    if (type == TOKEN_GREATER_THAN) return ">";
    if (type == TOKEN_LESS_THAN) return "<";
    if (type == TOKEN_GREATER_THAN_OR_EQUALS) return ">=";
    if (type == TOKEN_LESS_THAN_OR_EQUALS) return "<=";
    if (type == TOKEN_EQUALS_TO) return "==";
    if (type == TOKEN_ASSIGNMENT) return "=";
    if (type == TOKEN_NOT_EQUALS_TO) return "!=";
    if (type == TOKEN_ADD) return "+";
    if (type == TOKEN_MINUS) return "-";
    if (type == TOKEN_UNARY_MINUS) return "-";
    if (type == TOKEN_MULTIPLICATION) return "*";
    if (type == TOKEN_DIVISION) return "/";
    if (type == TOKEN_LEFT_PAREN) return "(";
    if (type == TOKEN_RIGHT_PAREN) return ")";
    if (type == TOKEN_LTL_OR) return "||";
    if (type == TOKEN_LTL_AND) return "&&";
    if (type == TOKEN_LTL_NOT) return "!";
    if (type == TOKEN_CONSEQUENCE) return "->";
    if (type == TOKEN_BICONDITIONAL) return "<->";
    if (type == TOKEN_LTL_EVENTUALLY) return "<>";
    if (type == TOKEN_LTL_ALWAYS) return "[]";
    return "ERROR";
}

char *to_string(LTL_TOKEN_TYPE type) {
    if (type == TOKEN_LTL_ERROR) return "TOKEN_LTL_ERROR";
    if (type == TOKEN_VARIABLE) return "TOKEN_VARIABLE";
    if (type == TOKEN_INTEGER) return "TOKEN_INTEGER";
    if (type == TOKEN_FLOAT) return "TOKEN_FLOAT";
    if (type == TOKEN_STRING) return "TOKEN_STRING";
    if (type == TOKEN_STRING_LITERAL) return "TOKEN_STRING_LITERAL";
    if (type == TOKEN_GREATER_THAN) return "TOKEN_GREATER_THAN";
    if (type == TOKEN_LESS_THAN) return "TOKEN_LESS_THAN";
    if (type == TOKEN_GREATER_THAN_OR_EQUALS) return "TOKEN_GREATER_THAN_OR_EQUALS";
    if (type == TOKEN_LESS_THAN_OR_EQUALS) return "TOKEN_LESS_THAN_OR_EQUALS";
    if (type == TOKEN_EQUALS_TO) return "TOKEN_EQUALS_TO";
    if (type == TOKEN_NOT_EQUALS_TO) return "TOKEN_NOT_EQUALS_TO";
    if (type == TOKEN_LEFT_PAREN) return "TOKEN_LEFT_PAREN";
    if (type == TOKEN_RIGHT_PAREN) return "TOKEN_RIGHT_PAREN";
    if (type == TOKEN_ADD) return "TOKEN_ADD";
    if (type == TOKEN_MINUS) return "TOKEN_MINUS";
    if (type == TOKEN_UNARY_MINUS) return "TOKEN_UNARY_MINUS";
    if (type == TOKEN_MULTIPLICATION) return "TOKEN_MULTIPLICATION";
    if (type == TOKEN_MODULUS) return "TOKEN_MODULUS";
    if (type == TOKEN_DIVISION) return "TOKEN_DIVISION";
    if (type == TOKEN_ASSIGNMENT) return "TOKEN_ASSIGNMENT";
    if (type == TOKEN_LTL_ALWAYS) return "TOKEN_LTL_ALWAYS";
    if (type == TOKEN_LTL_NEXT) return "TOKEN_LTL_NEXT";
    if (type == TOKEN_LTL_EVENTUALLY) return "TOKEN_LTL_EVENTUALLY";
    if (type == TOKEN_LTL_UNTIL) return "TOKEN_LTL_UNTIL";
    if (type == TOKEN_LTL_RELEASE) return "TOKEN_LTL_RELEASE";
    if (type == TOKEN_LTL_NOT) return "TOKEN_LTL_NOT";
    if (type == TOKEN_LTL_OR) return "TOKEN_LTL_OR";
    if (type == TOKEN_LTL_AND) return "TOKEN_LTL_AND";
    if (type == TOKEN_CONSEQUENCE) return "TOKEN_CONSEQUENCE";
    if (type == TOKEN_BICONDITIONAL) return "TOKEN_BICONDITIONAL";
    if (type == TOKEN_LTL_ATOM) return "TOKEN_LTL_ATOM";
    if (type == TOKEN_UNDERSCORE) return "TOKEN_UNDERSCORE";
    if (type == TOKEN_COMMA) return "TOKEN_COMMA";
    if (type == TOKEN_SEMICOLON) return "TOKEN_SEMICOLON";
    if (type == TOKEN_COLON) return "TOKEN_COLON";
    if (type == TOKEN_RIGHT_CURLY_BRACE) return "TOKEN_RIGHT_CURLY_BRACE";
    if (type == TOKEN_LEFT_CURLY_BRACE) return "TOKEN_LEFT_CURLY_BRACE";
    if (type == TOKEN_LTL_FUNCTION_CALL) return "TOKEN_LTL_FUNCTION_CALL";
    if (type == TOKEN_DATE) return "TOKEN_DATE";
    if (type == TOKEN_EOF) return "TOKEN_EOF";
    
    return "ERROR_LTL_TOKEN_TYPE_LTL_NOT_FOUND";
}

#define ONE_CHAR_TOKEN(token_type)\
    current_token.value[0] = current_char;\
    current_token.value[1] = 0;\
    current_token.type = token_type;\

void Input_buffer::get_next_csv_token() {
    // https://datatracker.ietf.org/doc/html/rfc4180
    /*
       1.  Each record is located on a separate line, delimited by a line
           break (CRLF).  For example:

           aaa,bbb,ccc CRLF
           zzz,yyy,xxx CRLF

       2.  The last record in the file may or may not have an ending line
           break.  For example:

           aaa,bbb,ccc CRLF
           zzz,yyy,xxx

       3.  There maybe an optional header line appearing as the first line
           of the file with the same format as normal record lines.  This
           header will contain names corresponding to the fields in the file
           and should contain the same number of fields as the records in
           the rest of the file (the presence or absence of the header line
           should be indicated via the optional "header" parameter of this
           MIME type).  For example:

           field_name,field_name,field_name CRLF
           aaa,bbb,ccc CRLF
           zzz,yyy,xxx CRLF

        4.  Within the header and each record, there may be one or more
           fields, separated by commas.  Each line should contain the same
           number of fields throughout the file.  Spaces are considered part
           of a field and should not be ignored.  The last field in the
           record must not be followed by a comma.  For example:

           aaa,bbb,ccc

       5.  Each field may or may not be enclosed in double quotes (however
           some programs, such as Microsoft Excel, do not use double quotes
           at all).  If fields are not enclosed with double quotes, then
           double quotes may not appear inside the fields.  For example:

           "aaa","bbb","ccc" CRLF
           zzz,yyy,xxx

       6.  Fields containing line breaks (CRLF), double quotes, and commas
           should be enclosed in double-quotes.  For example:

           "aaa","b CRLF
           bb","ccc" CRLF
           zzz,yyy,xxx

       7.  If double-quotes are used to enclose fields, then a double-quote
           appearing inside a field must be escaped by preceding it with
           another double quote.  For example:

           "aaa","b""bb","ccc"


        The ABNF grammar [2] appears as follows:

        file = [header CRLF] record *(CRLF record) [CRLF]
        header = name *(COMMA name)
        record = field *(COMMA field)
        name = field
        field = (escaped / non-escaped)
        escaped = DQUOTE (TEXTDATA / COMMA / CR / LF / 2DQUOTE)* DQUOTE
        non-escaped = TEXTDATA*
        COMMA = ','
        CR = '\r'
        DQUOTE =  '"'
        LF = '\n'
        CRLF = CR LF
        TEXTDATA =  ' ' - '!' / '#' - '+' / '\'' - '~'
    */

    if (current_char == '-') {
        ONE_CHAR_TOKEN(TOKEN_UNARY_MINUS);
        eat_char();
        get_char();

        if (!is_number(current_char) && is_csv_char(current_char) && current_char != ',') {
            u32 i = 1;

            while (current_char != ',' && current_char != '\n' && current_char != '\r' && !has_ended()) {
                current_token.value[i++] = current_char;
                eat_char();
                get_char();
            }

            current_token.value[i] = 0;
            current_token.type = TOKEN_STRING;
        } else if (current_char == ',' || current_char == '\n' || current_char == 0) {
            current_token.type = TOKEN_STRING;
        }
    } else if (current_char == ',') {
        ONE_CHAR_TOKEN(TOKEN_COMMA);
        eat_char();
        get_char();
    } else if (current_char == '"') {
        // escaped string
        eat_char();
        get_char();
        u32 i = 0;

        while (current_char != '"' && !has_ended()) {
            current_token.value[i++] = current_char;
            eat_char();
            get_char();

            if (current_char == '"') {
                eat_char();
                get_char();

                if (current_char == '"') {
                    // escaped double quote
                    current_token.value[i++] = '"';
                    eat_char();
                    get_char();
                } else {
                    rewind_char();
                    get_char();
                }
            }
        }

        if (current_char != '"' && current_char != 0) {
            report_error_in_input("expected double quote '\"' for unfinished quoted string in input file data");
        }

        eat_char();
        get_char();
        current_token.value[i] = 0;
        current_token.type = TOKEN_STRING;
    } else if (is_number(current_char)) {
        get_number_or_date();

        if (is_csv_char(current_char) && current_char != ',') {
            u32 i = length(current_token.value);

            while (is_csv_char(current_char) && current_char != ',') {
                current_token.value[i++] = current_char;
                eat_char();
                get_char();
            }

            current_token.value[i] = 0;
            current_token.type = TOKEN_STRING;
        }
    } else if (is_csv_char(current_char) && current_char != ',') {
        u32 i = 0;

        while (is_csv_char(current_char) && current_char != ',' && current_char != '\n' && !has_ended()) {
            current_token.value[i++] = current_char;
            eat_char();
            get_char();
        }

        current_token.value[i] = 0;
        current_token.type = TOKEN_STRING;
    } else if (current_char == 0) {
        current_token.value[0] = 0;
        current_token.type = TOKEN_EOF;
    } else {
        char msg[BUFFER_SIZE];
        sprintf(msg, "unrecognized character in input file data '%c'", current_char);
        report_error_in_input(msg);
    }
}


void Input_buffer::get_next_token() {
    current_token.position.p0 = current_index;

    if (current_char == 'U') {
        ONE_CHAR_TOKEN(TOKEN_LTL_UNTIL);
        eat_char();
        get_char();
    } else if (current_char == 'V') {
        ONE_CHAR_TOKEN(TOKEN_LTL_RELEASE);
        eat_char();
        get_char();
        current_token.position.pf = current_index;
    } else if (current_char == 'O') {
        ONE_CHAR_TOKEN(TOKEN_LTL_NEXT);
        eat_char();
        get_char();
    } else if (current_char == '%') {
        ONE_CHAR_TOKEN(TOKEN_MODULUS);
        eat_char();
        get_char();
    } else if (is_alpha(current_char)) {
        u32 result_index = 0;

        while ((is_alpha_numeric(current_char) || is_underscore(current_char)) && !is_ltl_operator_char(current_char)) {
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
        }

        current_token.value[result_index] = 0;

        if (current_char == '(') {
            current_token.type = TOKEN_LTL_FUNCTION_CALL;
        } else {
            current_token.type = TOKEN_VARIABLE;
        }
    } else if (is_number(current_char)) {
        get_number_or_date();
    } else if (current_char == '\'') {
        u32 str_index = 0;
        eat_char();
        get_char();

        while (current_char != '\'') {
            current_token.value[str_index++] = current_char;
            eat_char();
            get_char();

            if (current_char == 0) report_lexical_error("not finished string literal");
        }
        
        current_token.value[str_index] = 0;
        current_token.type = TOKEN_STRING_LITERAL;
        eat_char();
        get_char();
    } else if (current_char == '"') {
        u32 result_index = 0;
        eat_char();
        get_char();

        while (current_char != '"') {
            current_token.value[result_index++] = current_char;
            eat_char();
            get_char();
        }

        current_token.value[result_index] = 0;
        current_token.type = TOKEN_STRING;
        eat_char();
        get_char();
    } else if (current_char == '+') {
        ONE_CHAR_TOKEN(TOKEN_ADD);
        eat_char();
        get_char();
    } else if (current_char == '-') {
        ONE_CHAR_TOKEN(TOKEN_MINUS);
        eat_char();
        get_char();

        if (current_char == '>') {
            current_token.value[1] = current_char;
            current_token.value[2] = 0;
            current_token.type = TOKEN_CONSEQUENCE;
            eat_char();
            get_char();            
        }
    } else if (current_char == '*') {
        ONE_CHAR_TOKEN(TOKEN_MULTIPLICATION);
        eat_char();
        get_char();
    } else if (current_char == '/') {
        ONE_CHAR_TOKEN(TOKEN_DIVISION);
        eat_char();
        get_char();
    } else if (current_char == '<') {
        ONE_CHAR_TOKEN(TOKEN_LESS_THAN);
        eat_char();
        get_char();

        if (current_char == '>') {
            current_token.value[1] = current_char;
            current_token.value[2] = 0;
            current_token.type = TOKEN_LTL_EVENTUALLY;
            eat_char();
            get_char();
        } else if (current_char == '=') {
            current_token.value[1] = current_char;
            current_token.value[2] = 0;
            current_token.type = TOKEN_LESS_THAN_OR_EQUALS;
            eat_char();
            get_char();
        } else if (current_char == '-') {
            current_token.value[1] = current_char;
            eat_char();
            get_char();

            if (current_char != '>') {
                char msg[BUFFER_SIZE];
                sprintf(msg, "expected '>' for binary operator <->, got \'%c\'", current_char);
                report_lexical_error(msg);
            }

            current_token.value[2] = current_char;
            current_token.value[3] = 0;
            current_token.type = TOKEN_BICONDITIONAL;
            eat_char();
            get_char();
        }
    } else if (current_char == '>') {
        ONE_CHAR_TOKEN(TOKEN_GREATER_THAN);
        eat_char();
        get_char();

        if (current_char == '=') {
            current_token.value[1] = current_char;
            current_token.value[2] = 0;
            current_token.type = TOKEN_GREATER_THAN_OR_EQUALS;
            eat_char();
            get_char();
        }
    } else if (current_char == '=') {
        current_token.value[0] = current_char;
        eat_char();
        get_char();

        if (current_char != '=') {
            char msg[BUFFER_SIZE];
            sprintf(msg, "expected '=' for operator ==, got \'%c\'", current_char);
            report_lexical_error(msg);
        }

        current_token.value[1] = current_char;
        current_token.value[2] = 0;
        current_token.type = TOKEN_EQUALS_TO;
        eat_char();
        get_char();
    } else if (current_char == '(') {
        ONE_CHAR_TOKEN(TOKEN_LEFT_PAREN);
        eat_char();
        get_char();
    } else if (current_char == ')') {
        ONE_CHAR_TOKEN(TOKEN_RIGHT_PAREN);
        eat_char();
        get_char();
    } else if (current_char == '[') {
        current_token.value[0] = current_char;
        eat_char();
        get_char();
        
        if (current_char != ']') {
            char msg[BUFFER_SIZE];
            sprintf(msg, "expected ']' for binary operator [], got \'%c\'", current_char);
            report_lexical_error(msg);
        }

        current_token.value[1] = current_char;
        current_token.value[2] = 0;
        current_token.type = TOKEN_LTL_ALWAYS;
        eat_char();
        get_char();
    } else if (current_char == '|') {
        current_token.value[0] = current_char;
        eat_char();
        get_char();

        if (current_char != '|'){
            char msg[BUFFER_SIZE];
            sprintf(msg, "expected '|' for binary operator ||, got \'%c\'", current_char);
            report_lexical_error(msg);
        }

        current_token.value[1] = current_char;
        current_token.value[2] = 0;
        current_token.type = TOKEN_LTL_OR;
        eat_char();
        get_char();
    } else if (current_char == '&') {
        current_token.value[0] = current_char;
        eat_char();
        get_char();

        if (current_char != '&'){
            char msg[BUFFER_SIZE];
            sprintf(msg, "expected '&' for binary operator &&, got \'%c\'", current_char);
            report_lexical_error(msg);
        }

        current_token.value[1] = current_char;
        current_token.value[2] = 0;
        current_token.type = TOKEN_LTL_AND;
        eat_char();
        get_char();
    } else if (current_char == '!') {
        current_token.value[0] = current_char;
        current_token.value[1] = 0;
        current_token.type = TOKEN_LTL_NOT;
        eat_char();
        get_char();

        if (current_char == '=') {
            current_token.value[1] = current_char;
            current_token.value[2] = 0;
            current_token.type = TOKEN_NOT_EQUALS_TO;
            eat_char();
            get_char();            
        }
    } else if (current_char == '{') {
        ONE_CHAR_TOKEN(TOKEN_LEFT_CURLY_BRACE);
        eat_char();
        get_char();
    } else if (current_char == '}') {
        ONE_CHAR_TOKEN(TOKEN_RIGHT_CURLY_BRACE);
        eat_char();
        get_char();
    } else if (current_char == ':') {
        ONE_CHAR_TOKEN(TOKEN_COLON);
        eat_char();
        get_char();
    } else if (current_char == ';') {
        ONE_CHAR_TOKEN(TOKEN_SEMICOLON);
        eat_char();
        get_char();
    } else if (current_char == ',') {
        ONE_CHAR_TOKEN(TOKEN_COMMA);
        eat_char();
        get_char();
    } else if (current_char == '_') {
        ONE_CHAR_TOKEN(TOKEN_UNDERSCORE);
        eat_char();
        get_char();
    } else if (current_char == 0) {
        current_token.value[0] = 0;
        current_token.type = TOKEN_EOF;
    } else {
        report_lexical_error("character not recognized");
    }
    
    current_token.position.pf = current_index - 1;

    skip_whitespaces();
}

inline bool is_relop(LTL_TOKEN_TYPE type) {
    return type == TOKEN_GREATER_THAN || type == TOKEN_LESS_THAN || type == TOKEN_GREATER_THAN_OR_EQUALS || type == TOKEN_LESS_THAN_OR_EQUALS || type == TOKEN_EQUALS_TO || type == TOKEN_NOT_EQUALS_TO;
}

inline bool is_leaf(LTL_TOKEN_TYPE type) {
    return type == TOKEN_VARIABLE || type == TOKEN_INTEGER || type == TOKEN_FLOAT || type == TOKEN_STRING || type == TOKEN_STRING_LITERAL || type == TOKEN_DATE;
}

inline bool is_unary_logic_operator(LTL_TOKEN_TYPE type) {
    return type == TOKEN_LTL_NOT;
}

inline bool is_binary_logic_operator(LTL_TOKEN_TYPE type) {
    return type == TOKEN_LTL_OR || type == TOKEN_LTL_AND || type == TOKEN_CONSEQUENCE || type == TOKEN_BICONDITIONAL;
}

inline bool is_logic_operator(LTL_TOKEN_TYPE type) {
    return is_unary_logic_operator(type) || is_binary_logic_operator(type);
}

inline bool is_unary_ltl_operator(LTL_TOKEN_TYPE type) {
    return type == TOKEN_LTL_NEXT || type == TOKEN_LTL_EVENTUALLY || type == TOKEN_LTL_ALWAYS;
}

inline bool is_binary_ltl_operator(LTL_TOKEN_TYPE type) {
    return type == TOKEN_LTL_UNTIL || type == TOKEN_LTL_RELEASE;
}

inline bool is_binary_aritmetic_operator(LTL_TOKEN_TYPE type) {
    return type == TOKEN_MINUS || type == TOKEN_ADD || type == TOKEN_MULTIPLICATION || type == TOKEN_DIVISION || type == TOKEN_MODULUS;
}

inline bool is_addition_or_substraction(LTL_TOKEN_TYPE type) {
    return type == TOKEN_MINUS || type == TOKEN_ADD;
}

inline bool is_times_division_or_modulus(LTL_TOKEN_TYPE type) {
    return type == TOKEN_MULTIPLICATION || type == TOKEN_DIVISION || type == TOKEN_MODULUS;
}

inline bool is_basic_type(LTL_TOKEN_TYPE type) {
    return type == TOKEN_INTEGER || type == TOKEN_FLOAT || type == TOKEN_STRING || type == TOKEN_STRING_LITERAL || type == TOKEN_DATE;
}

inline bool is_ltl_operator(LTL_TOKEN_TYPE type) {
    return is_unary_ltl_operator(type) || is_binary_ltl_operator(type);
}

inline bool is_unary_operator(LTL_TOKEN_TYPE type) {
    return is_unary_ltl_operator(type) || is_unary_logic_operator(type) || type == TOKEN_UNARY_MINUS;
}

inline bool is_binary_operator(LTL_TOKEN_TYPE type) {
    return is_binary_ltl_operator(type) || is_binary_logic_operator(type);
}

inline bool apply_relop(Operation_value *left, Operation_value *right, LTL_TOKEN_TYPE relop, LTL_expression *left_tree, LTL_expression *right_tree) {
    if (left->type == TOKEN_INTEGER && right->type == TOKEN_INTEGER) {
        if (relop == TOKEN_GREATER_THAN) return left->int_value > right->int_value;
        if (relop == TOKEN_LESS_THAN) return left->int_value < right->int_value;
        if (relop == TOKEN_GREATER_THAN_OR_EQUALS) return left->int_value >= right->int_value;
        if (relop == TOKEN_LESS_THAN_OR_EQUALS) return left->int_value <= right->int_value;
        if (relop == TOKEN_EQUALS_TO) return left->int_value == right->int_value;
        if (relop == TOKEN_NOT_EQUALS_TO) return left->int_value != right->int_value;
    }

    if (left->type == TOKEN_INTEGER && right->type == TOKEN_FLOAT) {
        if (relop == TOKEN_GREATER_THAN) return (float)left->int_value > right->float_value;
        if (relop == TOKEN_LESS_THAN) return (float)left->int_value < right->float_value;
        if (relop == TOKEN_GREATER_THAN_OR_EQUALS) return (float)left->int_value >= right->float_value;
        if (relop == TOKEN_LESS_THAN_OR_EQUALS) return (float)left->int_value <= right->float_value;
        if (relop == TOKEN_EQUALS_TO) return (float)left->int_value == right->float_value;
        if (relop == TOKEN_NOT_EQUALS_TO) return (float)left->int_value != right->float_value;
    }

    if (left->type == TOKEN_FLOAT && right->type == TOKEN_INTEGER) {
        if (relop == TOKEN_GREATER_THAN) return left->float_value > (float)right->int_value;
        if (relop == TOKEN_LESS_THAN) return left->float_value < (float)right->int_value;
        if (relop == TOKEN_GREATER_THAN_OR_EQUALS) return left->float_value >= (float)right->int_value;
        if (relop == TOKEN_LESS_THAN_OR_EQUALS) return left->float_value <= (float)right->int_value;
        if (relop == TOKEN_EQUALS_TO) return left->float_value == (float)right->int_value;
        if (relop == TOKEN_NOT_EQUALS_TO) return left->float_value != (float)right->int_value;
    }

    if (left->type == TOKEN_FLOAT && right->type == TOKEN_FLOAT) {
        if (relop == TOKEN_GREATER_THAN) return left->float_value > right->float_value;
        if (relop == TOKEN_LESS_THAN) return left->float_value < right->float_value;
        if (relop == TOKEN_GREATER_THAN_OR_EQUALS) return left->float_value >= right->float_value;
        if (relop == TOKEN_LESS_THAN_OR_EQUALS) return left->float_value <= right->float_value;
        if (relop == TOKEN_EQUALS_TO) return left->float_value == right->float_value;
        if (relop == TOKEN_NOT_EQUALS_TO) return left->float_value != right->float_value;
    }

    if (left->type == TOKEN_STRING && right->type != TOKEN_STRING) {
        char msg[BUFFER_SIZE] = {};
        sprintf(msg, "uncompatible types for %s, '%s' <%s> and '%s' <%s>", to_string(relop), left_tree->formula, to_string(left->type), right_tree->formula, to_string(right->type));
        error(msg);
    }

    if (left->type != TOKEN_STRING && right->type == TOKEN_STRING) {
        char msg[BUFFER_SIZE] = {};
        sprintf(msg, "uncompatible types for %s, '%s' <%s> and '%s' <%s>", to_string(relop), left_tree->formula, to_string(left->type), right_tree->formula, to_string(right->type));
        error(msg);
    }

    if (left->type == TOKEN_STRING && right->type == TOKEN_STRING) {
        if (relop == TOKEN_GREATER_THAN) error("operation greater_than not allowed between strings");
        if (relop == TOKEN_LESS_THAN) error("operation less_than not allowed between strings");
        if (relop == TOKEN_GREATER_THAN_OR_EQUALS) error("operation greater_than_or_equals not allowed between strings");
        if (relop == TOKEN_LESS_THAN_OR_EQUALS) error("operation less_than_or_equals not allowed between strings");
        if (relop == TOKEN_EQUALS_TO) return equals(left->string_value, right->string_value);
        if (relop == TOKEN_NOT_EQUALS_TO) return !equals(left->string_value, right->string_value);
    }

    return false;
}

#define GET_EVALUATION(tree, result, dont_check_if_variable_in_table, empty_symbol_table)\
    if (tree->type == TOKEN_VARIABLE) {\
        LTL_symbol *s = tree->symbol;\
        if (s && empty_symbol_table) {\
            result.type = TOKEN_INTEGER;\
            result.int_value = 0;\
            return false;\
        }\
\
        if (!s && dont_check_if_variable_in_table) {\
            result.type = TOKEN_INTEGER;\
            result.int_value = 0;\
            return false;\
        }\
\
        if (!s) {\
            char msg[BUFFER_SIZE] = {};\
            sprintf(msg, "<%s> uninitialized variable, if a string literal was desired enclose it with single quotes like this '%s'", tree->formula, tree->formula);\
            error(msg);\
        }\
\
        if (s->type == TOKEN_INTEGER) {\
            result.type = TOKEN_INTEGER;\
            result.int_value = s->int_value;\
        } else if (s->type == TOKEN_FLOAT) {\
            result.type = TOKEN_FLOAT;\
            result.float_value = s->float_value;\
        } else if (s->type == TOKEN_STRING) {\
            result.type = TOKEN_STRING;\
            strcpy(result.string_value, s->string_value);\
        }\
    } else if (tree->type == TOKEN_INTEGER) {\
        result.type = TOKEN_INTEGER;\
        result.int_value = to_int(tree->formula);\
    } else if (tree->type == TOKEN_FLOAT) {\
        result.type = TOKEN_FLOAT;\
        result.float_value = to_float(tree->formula);\
    } else if (tree->type == TOKEN_STRING_LITERAL) {\
        result.type = TOKEN_STRING;\
        strcpy(result.string_value, tree->formula);\
    } else if (tree->type == TOKEN_STRING) {\
        result.type = TOKEN_STRING;\
        strcpy(result.string_value, tree->formula);\
    } else if (tree->type == TOKEN_DATE) {\
        result.int_value = date_to_timestamp(tree->formula);\
        result.type = TOKEN_INTEGER;\
    } else {\
        eval_tree(tree, &result, dont_check_if_variable_in_table);\
    }\

#define APPLY_OPERATOR(oper, token, left, right, left_tree, right_tree, result)\
    if (tree->type == token) {\
        if (left.type == TOKEN_STRING || right.type == TOKEN_STRING) {\
            char msg[BUFFER_SIZE] = {};\
            sprintf(msg, "operation '%s' not allowed with string type\n        '%s' <%s>; '%s' <%s>", #oper, left_tree->formula, to_string(left.type), right_tree->formula, to_string(right.type));\
            error(msg);\
        }\
\
        if (left.type == TOKEN_FLOAT && right.type == TOKEN_INTEGER) {\
            right.float_value = (f32)right.int_value;\
            right.type = TOKEN_FLOAT;\
            result->type = TOKEN_FLOAT;\
            result->float_value = left.float_value oper right.float_value;\
        } else if (left.type == TOKEN_INTEGER && right.type == TOKEN_FLOAT) {\
            left.float_value = (f32)left.int_value;\
            left.type = TOKEN_FLOAT;\
            result->type = TOKEN_FLOAT;\
            result->float_value = left.float_value oper right.float_value;\
        } else if (left.type == TOKEN_FLOAT && right.type == TOKEN_FLOAT) {\
            result->type = TOKEN_FLOAT;\
            result->float_value = left.float_value oper right.float_value;\
        } else {\
            result->type = TOKEN_INTEGER;\
            result->int_value = left.int_value oper right.int_value;\
        }\
\
        return result->int_value != 0;\
    }\

bool eval_tree(LTL_expression *tree, Operation_value *result, bool dont_check_if_variable_in_table=false, bool empty_symbol_table=false) {
    if (is_binary_aritmetic_operator(tree->type)) {
        Operation_value left;
        Operation_value right;

        GET_EVALUATION(tree->left, left, dont_check_if_variable_in_table, empty_symbol_table);
        GET_EVALUATION(tree->right, right, dont_check_if_variable_in_table, empty_symbol_table);
        APPLY_OPERATOR(+, TOKEN_ADD, left, right, tree->left, tree->right, result);
        APPLY_OPERATOR(-, TOKEN_MINUS, left, right, tree->left, tree->right, result);
        APPLY_OPERATOR(*, TOKEN_MULTIPLICATION, left, right, tree->left, tree->right, result);
        APPLY_OPERATOR(/, TOKEN_DIVISION, left, right, tree->left, tree->right, result);

        if (tree->type == TOKEN_MODULUS) {
            if (left.type != TOKEN_INTEGER || right.type != TOKEN_INTEGER) {
                error("operation '%' only allowed between integers");
            } else {
                result->int_value = left.int_value % right.int_value;
                result->type = TOKEN_INTEGER;
            }

            return result->int_value != 0;
        }
    } else if (tree->type == TOKEN_UNARY_MINUS) {
        Operation_value right;
        GET_EVALUATION(tree->right, right, dont_check_if_variable_in_table, empty_symbol_table);

        if (right.type == TOKEN_INTEGER) {
            result->type = TOKEN_INTEGER;
            result->int_value = -right.int_value;
        } else if (right.type == TOKEN_FLOAT) {
            result->type = TOKEN_FLOAT;
            result->float_value = -right.float_value;
        } else if (right.type == TOKEN_STRING){
            error("unary minus operation not valid for string");
        }

        return result->int_value != 0;
    } else if (is_relop(tree->type)) {
        Operation_value left;
        Operation_value right;

        GET_EVALUATION(tree->left, left, dont_check_if_variable_in_table, empty_symbol_table);
        GET_EVALUATION(tree->right, right, dont_check_if_variable_in_table, empty_symbol_table);
        return apply_relop(&left, &right, tree->type, tree->left, tree->right);
    } else if (is_binary_logic_operator(tree->type)) {
         bool p = eval_tree(tree->left, result, dont_check_if_variable_in_table, empty_symbol_table);
         bool q = eval_tree(tree->right, result, dont_check_if_variable_in_table, empty_symbol_table);

         if (tree->type == TOKEN_LTL_AND) {
            return p && q;
         } else if (tree->type == TOKEN_LTL_OR) {
            return p || q;
         } else if (tree->type == TOKEN_CONSEQUENCE) {
            return !p || q;
         } else if (tree->type == TOKEN_BICONDITIONAL) {
            return p == q;
         }

    } else if (is_unary_logic_operator(tree->type)) {
         bool p = eval_tree(tree->right, result, dont_check_if_variable_in_table, empty_symbol_table);
         return !p;
    } else if (tree->type == TOKEN_VARIABLE) {
        LTL_symbol *s = tree->symbol;

        if (s->type == TOKEN_INTEGER) {
            result->type = TOKEN_INTEGER;
            result->int_value = s->int_value;
        } else if (s->type == TOKEN_FLOAT) {
            result->type = TOKEN_FLOAT;
            result->float_value = s->float_value;
        } else if (s->type == TOKEN_STRING) {
            result->type = TOKEN_STRING;
            strcpy(result->string_value, s->string_value);
        } else if (s->type == TOKEN_LTL_ATOM) {
            return eval_tree(s->ltl_atom, result, dont_check_if_variable_in_table=false, empty_symbol_table);
        }
    } else if (tree->type == TOKEN_INTEGER) {
        result->type = TOKEN_INTEGER;
        result->int_value = to_int(tree->formula);
    } else if (tree->type == TOKEN_FLOAT) {
        result->type = TOKEN_FLOAT;
        result->float_value = to_float(tree->formula);
    } else if (tree->type == TOKEN_STRING) {
        result->type = TOKEN_STRING;
        strcpy(result->string_value, tree->formula);
    }

    return result->int_value != 0;
}

void eval_csv_factor(Input_buffer *streaming, Operation_value *result) {
    if (streaming->current_token.type == TOKEN_INTEGER) {
        result->int_value = to_int(streaming->current_token.value);
        result->type = TOKEN_INTEGER;
        streaming->get_next_csv_token();
    } else if (streaming->current_token.type == TOKEN_FLOAT) {
        result->float_value = to_float(streaming->current_token.value);
        result->type = TOKEN_FLOAT;
        streaming->get_next_csv_token();
    } else if (streaming->current_token.type == TOKEN_STRING) {
        strcpy(result->string_value, streaming->current_token.value);
        result->type = TOKEN_STRING;
        streaming->get_next_csv_token();
    } else if (streaming->current_token.type == TOKEN_STRING_LITERAL) {
        int str_index = 1;
        strcpy(result->string_value, streaming->current_token.value);

        while (result->string_value[str_index] != '\'') {
            result->string_value[str_index - 1] = result->string_value[str_index];
            ++str_index;
        }

        result->string_value[str_index - 1] = 0;
        result->type = TOKEN_STRING;
        streaming->get_next_csv_token();
    } else if (streaming->current_token.type == TOKEN_DATE) {
        result->int_value = date_to_timestamp(streaming->current_token.value);
        result->type = TOKEN_INTEGER;
        streaming->get_next_csv_token();
    }
}

LTL_expression *ltl_expression(Input_buffer *streaming, bool parenthesis_found=false);
LTL_expression *ltl_or_expression(Input_buffer *streaming, bool parenthesis_found=false);
LTL_expression *ltl_and_expression(Input_buffer *streaming, bool parenthesis_found=false);
LTL_expression *ltl_binary_expression(Input_buffer *streaming, bool parenthesis_found=false);
LTL_expression *ltl_term(Input_buffer *streaming, bool parenthesis_found=false);
LTL_expression *relop_expression(Input_buffer *streaming, bool parenthesis_found=false);
LTL_expression *expression(Input_buffer *streaming, bool parenthesis_found=false);
LTL_expression *term(Input_buffer *streaming, bool parenthesis_found=false);
LTL_expression *factor(Input_buffer *streaming, bool parenthesis_found=false);

LTL_expression *factor(Input_buffer *streaming, bool parenthesis_found) {
    /*
        factor: (expression) | var_name | function_call | basic_type
    */
    LTL_expression *tree;

    if (streaming->current_token.type == TOKEN_LEFT_PAREN) {
        streaming->get_next_token();
        tree = ltl_expression(streaming, true);

        if (streaming->current_token.type != TOKEN_RIGHT_PAREN) {
            char msg[BUFFER_SIZE];
            sprintf(msg, "expected right parenthesis, found: \'%s\'", streaming->current_token.value);
            // SYNTAX ERROR
            streaming->report_syntax_error_unbalanced_parenthesis(msg);
        }
    } else if (streaming->current_token.type == TOKEN_VARIABLE || streaming->current_token.type == TOKEN_INTEGER || streaming->current_token.type == TOKEN_FLOAT || streaming->current_token.type == TOKEN_STRING || streaming->current_token.type == TOKEN_DATE) {
        tree =  GET_MEMORY(LTL_expression);
        tree->interval = NULL;
        tree->is_from_parenthesis = false;
        tree->left = NULL;
        tree->right = NULL;
        tree->type = streaming->current_token.type;
        strcpy(tree->formula, streaming->current_token.value);
    } else if (streaming->current_token.type == TOKEN_STRING_LITERAL) {
        tree =  GET_MEMORY(LTL_expression);
        tree->interval = NULL;
        tree->is_from_parenthesis = false;
        tree->left = NULL;
        tree->right = NULL;
        tree->type = TOKEN_STRING;
        strcpy(tree->formula, streaming->current_token.value);
    } else if (streaming->current_token.type == TOKEN_LTL_FUNCTION_CALL) {
        tree =  GET_MEMORY(LTL_expression);
        tree->interval = NULL;
        tree->is_from_parenthesis = false;
        tree->left = NULL;
        tree->right = NULL;
        tree->type = TOKEN_LTL_FUNCTION_CALL;
        streaming->get_next_token();
        tree->right = expression(streaming);
        return tree;
    } else if (streaming->current_token.type == TOKEN_UNDERSCORE) {
        // SYNTAX ERROR
        char msg[BUFFER_SIZE] = {};
        sprintf(msg, "underscore for intervals not allowed here, found <%s> '%s'", to_string(streaming->current_token.type), streaming->current_token.value);
        streaming->report_syntax_error(msg, streaming->current_token.position);
    } else if (streaming->current_token.type == TOKEN_EOF) {
        char msg[BUFFER_SIZE] = {};
        sprintf(msg, "buffer terminated without being able to finish parsing properly\n    expecting an <expression>, variable or literal value, found <%s> '%s'", to_string(streaming->current_token.type), streaming->current_token.value);
        streaming->report_syntax_error(msg, streaming->current_token.position);
    } else {
        // SYNTAX ERROR
        char msg[BUFFER_SIZE] = {};
        sprintf(msg, "expecting an <expression>, variable or literal value, found <%s> '%s'", to_string(streaming->current_token.type), streaming->current_token.value);
        streaming->report_syntax_error(msg, streaming->current_token.position);
    }

    streaming->get_next_token();
    return tree;
}

LTL_expression *unary_expression(Input_buffer *streaming, bool parenthesis_found) {
    // unary_expression: [+-]* factor
    LTL_expression *tree;
    int sign = 1;

    while (streaming->current_token.type == TOKEN_ADD || streaming->current_token.type == TOKEN_MINUS) {
        if (streaming->current_token.type == TOKEN_MINUS) {
            sign *= -1;
        }

        streaming->get_next_token();
    }

    if (sign < 0) {
        tree = GET_MEMORY(LTL_expression);
        tree->interval = NULL;
        tree->type = TOKEN_UNARY_MINUS;
        tree->right = factor(streaming, parenthesis_found);
    } else {
        tree = factor(streaming, parenthesis_found);
    }

    return tree;
}

LTL_expression *term(Input_buffer *streaming, bool parenthesis_found) {
    // term: unary_expression ([/*%] unary_expression)*
    LTL_expression *tree = unary_expression(streaming, parenthesis_found);

    while (streaming->current_token.type == TOKEN_DIVISION || streaming->current_token.type == TOKEN_MULTIPLICATION || streaming->current_token.type == TOKEN_MODULUS) {
        LTL_expression *operator_tree = GET_MEMORY(LTL_expression);
        operator_tree->interval = NULL;
        operator_tree->is_from_parenthesis = parenthesis_found;
        operator_tree->type = streaming->current_token.type;
        streaming->get_next_token();
        
        operator_tree->left = tree;
        operator_tree->right = unary_expression(streaming, false);
        tree = operator_tree;
    }

    return tree;
}

LTL_expression *expression(Input_buffer *streaming, bool parenthesis_found) {
    // expression: term ([+-] term)*
    LTL_expression *tree = term(streaming, parenthesis_found);

    while (streaming->current_token.type == TOKEN_ADD || streaming->current_token.type == TOKEN_MINUS) {
        LTL_expression *operator_tree = GET_MEMORY(LTL_expression);
        operator_tree->interval = NULL;
        operator_tree->is_from_parenthesis = parenthesis_found;
        operator_tree->type = streaming->current_token.type;
        streaming->get_next_token();

        operator_tree->left = tree;
        operator_tree->right = term(streaming, false);
        tree = operator_tree;
    }

    return tree;
}

void get_interval(Interval *interval, Input_buffer *streaming) {
    streaming->get_next_token();
    
    if (streaming->current_token.type != TOKEN_LEFT_CURLY_BRACE) {
        char msg[BUFFER_SIZE];
        sprintf(msg, "parsing error, \'{\' expected for interval delimitation, found \'%s\'", streaming->current_token.value);
        streaming->report_syntax_error(msg, streaming->current_token.position);
    }

    streaming->get_next_token();
    interval->lower_bound = relop_expression(streaming);

    if (streaming->current_token.type == TOKEN_RIGHT_CURLY_BRACE) {
        streaming->get_next_token();
        // interval->higher_bound = interval->lower_bound;
        STACK_INPUT_BUFFER(eval, "1");
        interval->higher_bound = relop_expression(&eval);
        return;
    }

    if (streaming->current_token.type != TOKEN_COMMA) {
        char msg[BUFFER_SIZE];
        sprintf(msg, "parsing error, \',\' expected to separate interval lower and higher bounds, found \'%s\'", streaming->current_token.value);
        streaming->report_syntax_error(msg, streaming->current_token.position);
    }
    
    streaming->get_next_token();
    
    interval->higher_bound = relop_expression(streaming);

    if (streaming->current_token.type != TOKEN_RIGHT_CURLY_BRACE) {
        char msg[BUFFER_SIZE];
        sprintf(msg, "parsing error, \'}\' expected to finish interval definition, found \'%s\'", streaming->current_token.value);
        streaming->report_syntax_error(msg, streaming->current_token.position);
    }
    
    streaming->get_next_token();
}

LTL_expression *relop_expression(Input_buffer *streaming, bool parenthesis_found) {
    // expression (relop expression)*
    LTL_expression *tree = expression(streaming, parenthesis_found);

    while (is_relop(streaming->current_token.type)) {
        LTL_expression *operator_tree = GET_MEMORY(LTL_expression);
        operator_tree->interval = NULL;
        operator_tree->is_from_parenthesis = parenthesis_found;
        operator_tree->type = streaming->current_token.type;
        streaming->get_next_token();

        operator_tree->left = tree;
        operator_tree->right = expression(streaming, false);
        tree = operator_tree;
    }

    return tree;
}

LTL_expression *ltl_term(Input_buffer *streaming, bool parenthesis_found) {
    LTL_expression *tree;

    if (is_unary_operator(streaming->current_token.type)) {
        tree = GET_MEMORY(LTL_expression);
        tree->interval = NULL;
        tree->type = streaming->current_token.type;
        streaming->get_next_token();

        if (is_unary_ltl_operator(tree->type) && streaming->current_token.type == TOKEN_UNDERSCORE) {
            tree->interval = GET_MEMORY(Interval);
            get_interval(tree->interval, streaming);
        }
        
        tree->right = ltl_term(streaming, parenthesis_found);
    } else {
        tree = relop_expression(streaming, parenthesis_found);
    }

    return tree;
}

LTL_expression *ltl_binary_expression(Input_buffer *streaming, bool parenthesis_found) {
    LTL_expression *tree = ltl_term(streaming, parenthesis_found);
    
    while (is_binary_ltl_operator(streaming->current_token.type)) {
        LTL_expression *operator_tree = GET_MEMORY(LTL_expression);
        operator_tree->interval = NULL;
        operator_tree->is_from_parenthesis = parenthesis_found;
        operator_tree->type = streaming->current_token.type;
        streaming->get_next_token();
    
        if (streaming->current_token.type == TOKEN_UNDERSCORE) {
            operator_tree->interval = GET_MEMORY(Interval);
            get_interval(operator_tree->interval, streaming);
        }
    
        operator_tree->left = tree;
        operator_tree->right = ltl_term(streaming, false);
        tree = operator_tree;
    }

    return tree;
}

LTL_expression *ltl_and_expression(Input_buffer *streaming, bool parenthesis_found) {
    LTL_expression *tree = ltl_binary_expression(streaming, parenthesis_found);
    
    while (streaming->current_token.type == TOKEN_LTL_AND) {
        LTL_expression *operator_tree = GET_MEMORY(LTL_expression);
        operator_tree->interval = NULL;
        operator_tree->is_from_parenthesis = parenthesis_found;
        operator_tree->type = streaming->current_token.type;
        streaming->get_next_token();

        operator_tree->left = tree;
        operator_tree->right = ltl_binary_expression(streaming, false);
        tree = operator_tree;
    }

    return tree;
}

LTL_expression *ltl_or_expression(Input_buffer *streaming, bool parenthesis_found) {
    LTL_expression *tree = ltl_and_expression(streaming, parenthesis_found);
    
    while (streaming->current_token.type == TOKEN_LTL_OR) {
        LTL_expression *operator_tree = GET_MEMORY(LTL_expression);
        operator_tree->interval = NULL;
        operator_tree->is_from_parenthesis = parenthesis_found;
        operator_tree->type = streaming->current_token.type;
        streaming->get_next_token();

        operator_tree->left = tree;
        operator_tree->right = ltl_and_expression(streaming, false);
        tree = operator_tree;
    }

    return tree;
}

LTL_expression *ltl_expression(Input_buffer *streaming, bool parenthesis_found) {
    LTL_expression *tree = ltl_or_expression(streaming, parenthesis_found);
    
    while (streaming->current_token.type == TOKEN_CONSEQUENCE || streaming->current_token.type == TOKEN_BICONDITIONAL) {
        LTL_expression *operator_tree = GET_MEMORY(LTL_expression);
        operator_tree->interval = NULL;
        operator_tree->is_from_parenthesis = parenthesis_found;
        operator_tree->type = streaming->current_token.type;
        streaming->get_next_token();

        operator_tree->left = tree;
        operator_tree->right = ltl_or_expression(streaming, false);
        tree = operator_tree;
    }

    return tree;
}

inline void relop_expression_to_string(LTL_expression *relop_expression, char *result) {
    if (is_relop(relop_expression->type)) {
        relop_expression_to_string(relop_expression->left, result);
        strcat(result, token_type_to_symbol(relop_expression->type));
        relop_expression_to_string(relop_expression->right, result);
    } else if (is_leaf(relop_expression->type)) {
        strcat(result, relop_expression->formula);
    }
}

void print_ltl_tree(Symbol_table *st, LTL_expression *tree, u32 level) {
    char node_tab[BUFFER_SIZE];
    char leaf_tab[BUFFER_SIZE];
    char interval_lower[BUFFER_SIZE] = {};
    char interval_higher[BUFFER_SIZE] = {};
    u32 i;

    for (i = 0; i < (level); ++i) {
        node_tab[i] = '-';
    }

    node_tab[level] = '>';
    node_tab[i + 1] = 0;

    for (i = 0; i < (level); ++i) {
        leaf_tab[i] = '-';
    }

    leaf_tab[level] = '>';
    leaf_tab[i + 1] = 0;

    Color_rgb color = tree_debug_colors[level];
    u32 r = color.r;
    u32 g = color.g;
    u32 b = color.b;

    if ((is_binary_operator(tree->type) || is_binary_aritmetic_operator(tree->type) || is_relop(tree->type)) && tree->left != NULL) {
        printf("  %s", node_tab);
        color_text(to_string(tree->type), r, g, b);

        if (tree->interval) {
            relop_expression_to_string(tree->interval->lower_bound, interval_lower);

            if (tree->interval->higher_bound) {
                relop_expression_to_string(tree->interval->higher_bound, interval_higher);
                printf(" {%s, %s}\n", interval_lower, interval_higher);
            } else {
                printf(" {%s}\n", interval_lower);
            }
        } else {
            printf(" parenthesis %s", to_string(tree->is_from_parenthesis));
            putchar('\n');
        }

        print_ltl_tree(st, tree->left, level + 1);
        print_ltl_tree(st, tree->right, level + 1);
    } else if (is_unary_operator(tree->type)) {
        printf("  %s", node_tab);
        color_text(to_string(tree->type), r, g, b);

        if (tree->interval){
            relop_expression_to_string(tree->interval->lower_bound, interval_lower);
            
            if (tree->interval->higher_bound) {
                relop_expression_to_string(tree->interval->higher_bound, interval_higher);
                printf(" {%s, %s}\n", interval_lower, interval_higher);
            } else {
                printf(" {%s}\n", interval_lower);
            }
        } else {
            putchar('\n');
        }
        
        print_ltl_tree(st, tree->right, level + 1);
    } else if (tree->type == TOKEN_LTL_FUNCTION_CALL) {
        printf("  %savg(%s)\n", node_tab, tree->right->formula);
    } else if (tree->type == TOKEN_LTL_ATOM) {
        printf("  %s", node_tab);
        color_text(tree->formula, r, g, b);
    } else if (tree->type == TOKEN_STRING) {
        printf("  %s'", node_tab);
        color_text(tree->formula, r, g, b);
        printf("'<%s>\n", to_string(tree->type));
    } else {
        printf("  %s", node_tab, to_string(tree->type));
        color_text(tree->formula, r, g, b);
        printf("<%s>\n", to_string(tree->type));
    }
}

void print_ltl_tree(Symbol_table *st, LTL_expression *tree) {
    printf("ast tree:\n");
#if CONSOLE_COLORS
    printf("the colors indicates the depth level in the tree\n");
#endif
    print_ltl_tree(st, tree, 0);
}

void ltl_tree_to_string_formula(LTL_expression *tree, char *formula) {
    if (is_binary_operator(tree->type) || is_binary_aritmetic_operator(tree->type) || is_relop(tree->type)) {
        char l[BUFFER_SIZE] = {};
        char r[BUFFER_SIZE] = {};
        ltl_tree_to_string_formula(tree->left, l);
        ltl_tree_to_string_formula(tree->right, r);
        sprintf(formula, "(%s) %s (%s)", l, token_type_to_symbol(tree->type), r);
    } else if (is_unary_operator(tree->type)) {
        char r[BUFFER_SIZE] = {};
        ltl_tree_to_string_formula(tree->right, r);
        sprintf(formula, "%s (%s)", token_type_to_symbol(tree->type), r);
    } else if (tree->type == TOKEN_LTL_FUNCTION_CALL) {
        char r[BUFFER_SIZE] = {};
        ltl_tree_to_string_formula(tree->right, r);
        sprintf(formula, "%savg(%s)", formula, r);
    } else {
        sprintf(formula, "%s%s", formula, tree->formula);
    }    
}

void get_variable_references_from_symbol_table(LTL_expression *tree, Symbol_table *table, bool empty_symbol_table=false) {
    if (is_relop(tree->type) || is_binary_operator(tree->type) || is_binary_aritmetic_operator(tree->type)) {
        get_variable_references_from_symbol_table(tree->left, table, empty_symbol_table);
        get_variable_references_from_symbol_table(tree->right, table, empty_symbol_table);
        tree->symbol = 0;
    } else if (is_unary_operator(tree->type)) {
        get_variable_references_from_symbol_table(tree->right, table, empty_symbol_table);
        tree->symbol = 0;
    } else if (tree->type == TOKEN_VARIABLE) {
        tree->symbol = get_symbol_reference(tree->formula, table);

        if (!tree->symbol && empty_symbol_table) return;

        if (!tree->symbol) {
            char msg[BUFFER_SIZE] = {};
            sprintf(msg, "<%s> uninitialized variable, if a string literal was desired enclose it with single quotes like this '%s'", tree->formula, tree->formula);
            error(msg);
        }
        
        if (tree->symbol->type == TOKEN_LTL_ATOM) {
            get_variable_references_from_symbol_table(tree->symbol->ltl_atom, table, empty_symbol_table);
        }
    } else {
        tree->symbol = 0;
    }
}

void get_variable_references_from_symbol_table(Csv_header *header, Symbol_table *table) {
    while (header) {
        header->symbol = get_symbol_reference(header->name, table);
        header = header->next;
    }
}
