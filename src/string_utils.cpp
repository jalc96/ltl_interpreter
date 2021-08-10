/*
requirements
#include<stdlib.h>
#include<cstring>
*/

u64 length(char *str) {
    char *s;
    for (s = str; *s; ++s);
    return (s - str);
}

inline char lower_case(char c) {
    // a: 97
    // z: 122
    // A: 65
    // Z: 90
    int conversion = 'a' - 'A';

    if (c >= 'A' && c <= 'Z') {
        return c + conversion;
    } else {
        return c;
    }

    return -1;
}

inline bool is_whitespace(char c) {
    return (c == ' ') || (c == '\t');
}

inline bool is_new_line(char c) {
    return c == '\n';
}

inline bool is_alpha(char c) {
    return (lower_case(c) >= 'a') && (lower_case(c) <= 'z');
}

inline bool is_number(char c) {
    return (c >= '0') && (c <= '9');
}

inline bool is_csv_char(char c) {
    // TEXTDATA =  ' ' - '!' / '#' - '+' / '\'' - '~'    
    return (c >= ' ') && (c <= '!') || (c >= '#') && (c <= '+') || (c >= '\'') && (c <= '~');
}

inline bool is_underscore(char c) {
    return c == '_';
}

inline bool is_alpha_numeric(char c) {
    return is_alpha(c) || is_number(c);
}

inline bool is_operator(char c) {
    return (c == '+') || (c == '-') || (c == '*') || (c == '/') || (c == '%') || (c == '<') || (c == '>') || (c == '=') || (c == '!') /*|| (c == '&') || (c == '|') || (c == '[') || (c == ']')*/;
}

inline bool is_relational_operator(char c) {
    return (c == '<') || (c == '>') || (c == '=') || (c == '!');
}

inline bool is_ltl_operator_char(char c) {
    return c == 'U' || c == 'V' || c == 'O';
}

int to_int(char c) {
    int result = c - '0';
    return result;
}

inline u64 to_int(char *str) {
    // return atoll(str);
    u64 n = to_int(*str);
    u64 d;

    while ((d = to_int(*++str)) <= 9) {
       n = n * 10 + d;
    }
    return n;
}

inline float to_float(char *str) {
    return atof(str);
}

char *to_string(bool p) {
    if (p) return "true";
    return "false";
}

inline bool equals(char *string_1, char *string_2) {
    return strcmp(string_1, string_2) == 0;
}

f32 similarity(char *string_1, char *string_2) {
    u32 hit = 0;
    u32 total = 1;
    u32 i = 0;

    while (string_1[i] != 0 && string_2[i] != 0) {
        if (string_1[i] == string_2[i]) hit++;
        total++;
        i++;
    }

    return ((f32)hit/(f32)total);
}
