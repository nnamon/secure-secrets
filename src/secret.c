#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#include <termios.h>

#include "trim.h"

void usage(char* name) {
    printf("Usage: %s read|add|delete|list [--help]\n", name);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        usage(argv[0]);
        exit(1);
    }
    else if (strcasecmp(argv[1], "help") == 0) {
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
    int homelen = strlen(home);

    //
    // Read and check password
    //
    const char* pwname = "/.secretpw";
    const size_t pwlen = homelen + strlen(pwname);
    char pwpath[pwlen + 1];
    strncpy(pwpath, home, homelen);
    strncpy(pwpath + homelen, pwname, strlen(pwname));

    FILE* pwfile = NULL;
    if (access(pwpath, F_OK) == -1) {
        fprintf(stderr, "Error reading password file '%s': %s\n", pwpath, strerror(errno));
        exit(5);
    } else {
        pwfile = fopen(pwpath, "r");
    }

    size_t len = 0;
    char* line = NULL;
    getline(&line, &len, pwfile);
    fclose(pwfile);

    // Sanitise password
    char* temp = triml(trimr(line));
    len = strlen(temp);
    char attempt[64];
    char password[64];
    memset(attempt, 0, 64);
    memset(password, 0, len + 1);
    strncpy(&password, temp, len);
    free(line);

    /* Turn echoing off and fail if we can't. */
    struct termios old, new;
    if (tcgetattr (fileno(stdout), &old) != 0)
        return -1;
    new = old;
    new.c_lflag &= ~ECHO;
    if (tcsetattr (fileno(stdout), TCSAFLUSH, &new) != 0)
        return -1;
    /* ---------- */

    for(int i = 0; i < 3; i++) {
        // Read user input
        fprintf(stderr, "Enter secrets password: ");
        gets(attempt);
        fprintf(stderr, "\n");

        if (strncmp(password, attempt, strlen(attempt)) != 0) {
            sleep(2);
            fprintf(stderr, "Incorrect password, please try again (%d/3)\n", (i + 1));
        } else {
            (void) tcsetattr (fileno(stdout), TCSAFLUSH, &old);
            goto correct;
        }
    }
    exit(10);
correct:
    fprintf(stderr, "\n");

    //
    // Open secrets file
    //
    len = keyring == NULL ? strlen(home) + 10 : strlen(keyring);
    FILE* keyfile = NULL;
    char buf[len];

    if (keyring == NULL) {
        // ~/.secrets
        strncpy(buf, home, homelen);
        strncpy(buf + homelen, "/.secrets", 10);
    } else {
        strncpy(buf, keyring, len);
    }
    keyring = buf;

    if (access(keyring, F_OK) == -1) {
        fprintf(stderr, "Error reading secrets file '%s': %s\n", keyring, strerror(errno));
        exit(6);
    } else {
        keyfile = fopen(keyring, "r+");
    }

    //
    // Perform requested operation
    //
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
                memset(name, 0, len);
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
