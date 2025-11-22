#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "commands.h" 

// cp 
void cmd_cp(char *src, char *dest) {
    int fd_in, fd_out;
    char buffer[4096];
    ssize_t bytes;

    if ((fd_in = open(src, O_RDONLY)) < 0) {
        perror("cp error");
        return;
    }
    if ((fd_out = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
        perror("cp dest error");
        close(fd_in);
        return;
    }
    while ((bytes = read(fd_in, buffer, sizeof(buffer))) > 0) {
        write(fd_out, buffer, bytes);
    }
    close(fd_in);
    close(fd_out);
}

// mv 
void cmd_mv(char *src, char *dest) {
    if (rename(src, dest) < 0) {
        perror("mv error");
    }
}

// ln 
void cmd_ln(char *src, char *dest) {
    if (link(src, dest) < 0) {
        perror("ln error");
    }
}

// grep 
void cmd_grep(char *pattern, char *filename) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("grep error");
        return;
    }
    while ((read = getline(&line, &len, fp)) != -1) {
        if (strstr(line, pattern) != NULL) {
            printf("%s", line);
        }
    }
    free(line);
    fclose(fp);
}