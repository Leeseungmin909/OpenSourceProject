#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "commands.h"

#define MAX_ARG_LEN 64

void signal_handler(int signo) {
    if (signo == SIGINT) {
        printf("\nMyShell> ");
        fflush(stdout);
    } else if (signo == SIGQUIT) {
        printf("\n종료하려면 exit를 입력하세요.\nMyShell> ");
        fflush(stdout);
    }
}

int parse_args(char *input, char **args) {
    int i = 0;
    int arg_count = 0;
    int in_word = 0;
    
    while (input[i] != '\0') {
        if (input[i] == ' ' || input[i] == '\t') {
            if (in_word) {
                input[i] = '\0';
                in_word = 0;
            }
        } else {
            if (!in_word) {
                args[arg_count++] = &input[i];
                in_word = 1;
            }
        }
        i++;
    }
    args[arg_count] = NULL;
    return arg_count;
}

int main() {
    char *input;
    char *args[MAX_ARG_LEN];
    struct sigaction act;

    act.sa_handler = signal_handler;
    sigfillset(&(act.sa_mask));
    act.sa_flags = 0;
    
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);

    while (1) {
        input = readline("\nMyShell> ");
        
        if (input == NULL) break;
        if (strlen(input) == 0) { free(input); continue; }

        add_history(input);

        if (strcmp(input, "exit") == 0) {
            printf("MyShell을 종료합니다.\n");
            free(input);
            break;
        }

        int is_bg = 0;
        int len = strlen(input);
        if (input[len - 1] == '&') {
            is_bg = 1;
            input[len - 1] = '\0';
            for (int k = len - 2; k >= 0; k--) {
                if (input[k] == ' ') input[k] = '\0';
                else break;
            }
        }

        char *pipe_pos = strchr(input, '|');
        if (pipe_pos != NULL) {
            *pipe_pos = '\0';
            char *cmd1_str = input;
            char *cmd2_str = pipe_pos + 1;
            
            int fd[2];
            if (pipe(fd) == -1) { perror("pipe error"); free(input); continue; }

            pid_t pid1 = fork();
            if (pid1 == 0) {
                close(fd[0]); dup2(fd[1], STDOUT_FILENO); close(fd[1]);
                char *p_args[MAX_ARG_LEN];
                parse_args(cmd1_str, p_args);
                execvp(p_args[0], p_args);
                exit(1);
            }

            pid_t pid2 = fork();
            if (pid2 == 0) {
                close(fd[1]); dup2(fd[0], STDIN_FILENO); close(fd[0]);
                char *p_args[MAX_ARG_LEN];
                parse_args(cmd2_str, p_args);
                execvp(p_args[0], p_args);
                exit(1);
            }
            close(fd[0]); close(fd[1]);
            wait(NULL); wait(NULL);
            free(input); continue;
        }

        char *input_copy = strdup(input);
        char *infile = NULL, *outfile = NULL;
        char *temp_args[MAX_ARG_LEN];
        
        parse_args(input_copy, temp_args);
        
        int i = 0;
        int arg_idx = 0;
        while (temp_args[i] != NULL) {
            if (strcmp(temp_args[i], "<") == 0) {
                if (temp_args[++i]) infile = temp_args[i];
            } else if (strcmp(temp_args[i], ">") == 0) {
                if (temp_args[++i]) outfile = temp_args[i];
            } else {
                args[arg_idx++] = temp_args[i];
            }
            i++;
        }
        args[arg_idx] = NULL;

        if (args[0] == NULL) { free(input); free(input_copy); continue; }

        if (strcmp(args[0], "cd") == 0) cmd_cd(arg_idx, args);
        else if (strcmp(args[0], "ls") == 0) cmd_ls(arg_idx, args);
        else if (strcmp(args[0], "pwd") == 0) cmd_pwd(arg_idx, args);
        else if (strcmp(args[0], "mkdir") == 0) cmd_mkdir(arg_idx, args);
        else if (strcmp(args[0], "rmdir") == 0) cmd_rmdir(arg_idx, args);
        else if (strcmp(args[0], "ln") == 0) cmd_ln(arg_idx, args);
        else if (strcmp(args[0], "cp") == 0) cmd_cp(arg_idx, args);
        else if (strcmp(args[0], "rm") == 0) cmd_rm(arg_idx, args);
        else if (strcmp(args[0], "mv") == 0) cmd_mv(arg_idx, args);
        else if (strcmp(args[0], "cat") == 0) cmd_cat(arg_idx, args);
        else if (strcmp(args[0], "grep") == 0) cmd_grep(arg_idx, args);
        else {
            pid_t pid = fork();
            if (pid == 0) {
                if (infile) {
                    int fd = open(infile, O_RDONLY);
                    dup2(fd, STDIN_FILENO); close(fd);
                }
                if (outfile) {
                    int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    dup2(fd, STDOUT_FILENO); close(fd);
                }
                execvp(args[0], args);
                perror("Command execution failed");
                exit(1);
            } else if (pid > 0) {
                if (!is_bg) waitpid(pid, NULL, 0);
                else printf("[Background PID: %d]\n", pid);
            } else {
                perror("fork failed");
            }
        }
        
        free(input);
        free(input_copy);
    }
    return 0;
}