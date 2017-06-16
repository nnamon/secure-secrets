#include <stddef.h>
#include <string.h>
#include <ctype.h>

char* triml(char* str) {
    if (str == NULL)
        return NULL;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    return str;
}

char* trimr(char* str) {
    if (str == NULL)
        return NULL;

    // Trim trailing space
    char* end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator
    *(end+1) = 0;

    return str;
}
