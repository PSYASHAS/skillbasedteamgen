#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

void trim_newline(char* str) {
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

const char* get_experience_str(int level) {
    switch(level) {
        case 0: return "Beginner";
        case 1: return "Intermediate";
        case 2: return "Advanced";
        case 3: return "Expert";
        default: return "Unknown";
    }
}

int case_insensitive_match(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) {
            return 0; // not match
        }
        s1++;
        s2++;
    }
    return (*s1 == *s2);
}
