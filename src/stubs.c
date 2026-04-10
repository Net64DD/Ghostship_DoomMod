#include "mod.h"
#include "sm64.h"
#include "doom/doomtype.h"

#include <stdarg.h>
#include <stdio.h>

int printf(const char* fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    LUSLOG_INFO("%s", buffer);
    return len;
}

int fprintf(FILE* stream, const char* fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    LUSLOG_INFO("%s", buffer);
    return len;
}

int snprintf(char* buffer, size_t len, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(buffer, len, fmt, args);
    va_end(args);
    return written;
}

static inline unsigned char custom_tolower(unsigned char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A'); // Shifts the character by 32
    }
    return c;
}

int __strcasecmp(const char *s1, const char *s2) {
    const unsigned char *us1 = (const unsigned char *)s1;
    const unsigned char *us2 = (const unsigned char *)s2;

    while (custom_tolower(*us1) == custom_tolower(*us2)) {
        if (*us1 == '\0') {
            return 0;
        }
        us1++;
        us2++;
    }
    
    return custom_tolower(*us1) - custom_tolower(*us2);
}

int __strncasecmp(const char *s1, const char *s2, int n) {
    if (n == 0) {
        return 0;
    }

    const unsigned char *us1 = (const unsigned char *)s1;
    const unsigned char *us2 = (const unsigned char *)s2;

    while (n-- != 0) {
        if (custom_tolower(*us1) != custom_tolower(*us2)) {
            return custom_tolower(*us1) - custom_tolower(*us2);
        }
        if (*us1 == '\0') {
            return 0;
        }
        us1++;
        us2++;
    }
    
    return 0;
}