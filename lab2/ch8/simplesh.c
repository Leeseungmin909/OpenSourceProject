#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

int getargs(char *cmd, char **argv) {
    int narg = 0;
    while (*cmd) {
        if (*cmd == ' ' || *cmd == '\t') {
            *cmd++ = '\0';
        } else {
            argv[narg++] = cmd++;
            while (*cmd != '\0' && *cmd != ' ' && *cmd != '\t')
                cmd++;
        }
    }
    argv[narg] = NULL;
    return narg;
}

int main() {
    char buf[256];
    char *argv[50];
    int narg;
    pid_t pid;
    int status; 

    while (1) {
        printf("shell> ");
        if (fgets(buf, sizeof(buf), stdin) == NULL) break; 

        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        narg = getargs(buf, argv);
        if (narg == 0) continue; 
        pid = fork();
        
        if (pid == 0) {
            execvp(argv[0], argv);
            perror("execvp failed");
            exit(1);
        } else if (pid > 0) {
            while (wait(&status) != pid); 
        } else {
            perror("fork failed");
        }
    }
    
    return 0;
}