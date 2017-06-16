#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>

#include "trim.h"

void usage(char* name) {
    printf("Usage: %s read|add|delete|list [--help]\n", name);
}

int main(int argc, char** argv) {
    if (argc < 1) {
        usage(argv[0]);
        exit(1);
    }
    if (strcasecmp(argv[1], "help") == 0) {
        usage(argv[0]);
        exit(0);
    }

    char *home = NULL,
         *keyring = getenv("KEYRING");
    if (keyring == NULL) {
        if (argc > 2) {
            keyring = argv[2];
        } else {
            // Use HOME environment var
            home = getenv("HOME");
            if (home == NULL) {
                // Get home directory from /etc/passwd
                home = getpwuid(getuid())->pw_dir;
            }
        }
    }

    FILE* keyfile = NULL;
    int len = keyring == NULL ? strlen(home) + 10 : strlen(keyring);
    char buf[len];

    if (keyring == NULL) {
        // ~/.secrets
        int homelen = strlen(home);
        strncpy(buf, home, homelen);
        strncpy(buf + homelen, "/.secrets", 10);
    } else {
        strncpy(buf, keyring, len);
    }
    keyring = buf;

    if (access(keyring, F_OK) == -1) {
        fprintf(stderr, "The keyring file '%s' doesn't exist\n", keyring);
    } else {
        keyfile = fopen(keyring, "r+");
    }


    if (strcasecmp(argv[1], "read") == 0) {
        /* Open file and list secrets out */
        printf("Reading passwords is not implemented yet...\n");
    } else if (strcasecmp(argv[1], "add") == 0) {
        /* Append new secret to end of file */
        printf("Adding passwords is not implemented yet...\n");
    } else if (strcasecmp(argv[1], "delete") == 0) {
        /* Remove secret, name and empty line */
        printf("Deleting passwords is not implemented yet...\n");
    } else if (strcasecmp(argv[1], "list") == 0 && keyfile != NULL) {
        /* Dump each name/secret to stdout */

        const size_t ALLOC_SIZE = 128;

        ssize_t nread     = 0;
        size_t  len       = 0;
        size_t  contsz    = 0;
        char    *line     = NULL,
                *name     = NULL,
                *content  = NULL;

        while ((nread = getline(&line, &len, keyfile)) != -1) {

            if (line[0] == '>') {
                // Finished reading one section.
                // Print it and start again
                if (name != NULL) {
                    printf("=> %s\n%s\n\n", name, content);
                    free(name);
                    free(content);
                    name = NULL;
                    content = NULL;
                    contsz = 0;
                }

                // Remove trailing newline
                line[nread - 1] = 0;

                // Trim whitespace left and right
                char* trimmed = triml(trimr(line + 1));
                int len = strlen(trimmed);
                name = malloc(len);
                // Save trimmed line as 'name'
                strncpy(name, trimmed, len);
            }
            else {
                // Trim and concatenate content
                char* trimmed = triml(trimr(line));
                int len = strlen(trimmed);

                size_t contlen = (content == NULL) ? 0 : strlen(content);
                while (content == NULL || (contlen + len) >= contsz) {
                    contsz += ALLOC_SIZE;
                    content = realloc(content, contsz);
                    memset(content, 0, contsz);
                }
                strncat(content, trimmed, len);
            }
        }
        if (errno != 0) {
            printf("An error occurred whilst listing the passwords\n%s\n", strerror(errno));
            exit(2);
        }

        // Found EOF so printing and stopping
        if (name != NULL) {
            printf("=> %s\n%s\n\n", name, content);
            free(name);
            free(content);
            name = NULL;
            content = NULL;
            contsz = 0;
        }

        free(line);

    } else {
        printf("Command '%s' not found.\n", argv[1]);
        usage(argv[0]);
        exit(1);
    }

    if (keyfile != NULL) {
        fclose(keyfile);
    }
}
