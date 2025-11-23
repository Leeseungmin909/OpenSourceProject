#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "commands.h"

#define MAX_ARG_LEN 64

int parse_args(char *input, char **args) {
    int i = 0;
    int arg_count = 0;
    int in_word = 0;
    
    while (input[i] != '\0') {
        if (input[i] == ' ') {
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

    while (1) {
        input = readline("MyShell> ");
        
        if (input == NULL) break;
        
        if (strlen(input) == 0) {
            free(input);
            continue;
        }

        add_history(input);

        if (strcmp(input, "exit") == 0) {
            printf("MyShell을 종료합니다.\n");
            free(input);
            break;
        }

        int is_bg = 0;
        if (input[strlen(input) - 1] == '&') {
            is_bg = 1;
            input[strlen(input) - 1] = 0;
            while (strlen(input) > 0 && input[strlen(input)-1] == ' ')
                input[strlen(input)-1] = 0;
        }

        char *pipe_pos = strchr(input, '|');
        if (pipe_pos != NULL) {
            *pipe_pos = 0;
            char *cmd1_str = input;
            char *cmd2_str = pipe_pos + 1;
            
            while (*cmd1_str == ' ') cmd1_str++;
            while (*cmd2_str == ' ') cmd2_str++;
            
            int fd[2];
            
            if (pipe(fd) == -1) {
                perror("파이프 생성 실패");
                free(input);
                continue;
            }

            pid_t pid1, pid2;
            
            if ((pid1 = fork()) == 0) {
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                
                char *p_args[MAX_ARG_LEN];
                parse_args(cmd1_str, p_args);
                
                execvp(p_args[0], p_args);
                perror("첫 번째 명령어 실행 실패");
                exit(1);
            }

            if ((pid2 = fork()) == 0) {
                close(fd[1]);
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);

                char *p_args[MAX_ARG_LEN];
                parse_args(cmd2_str, p_args);
                
                execvp(p_args[0], p_args);
                perror("두 번째 명령어 실행 실패");
                exit(1);
            }

            close(fd[0]);
            close(fd[1]);
            wait(NULL);
            wait(NULL);
            free(input);
            continue;
        }

        char *input_copy = strdup(input);
        char *infile = NULL, *outfile = NULL;
        char *temp_args[MAX_ARG_LEN];
        
        parse_args(input_copy, temp_args);
        
        int i = 0;
        int arg_idx = 0;
        while (temp_args[i] != NULL) {
            if (strcmp(temp_args[i], "<") == 0) {
                i++;
                if (temp_args[i] != NULL) {
                    infile = temp_args[i];
                }
            } else if (strcmp(temp_args[i], ">") == 0) {
                i++;
                if (temp_args[i] != NULL) {
                    outfile = temp_args[i];
                }
            } else {
                args[arg_idx++] = temp_args[i];
            }
            i++;
        }
        args[arg_idx] = NULL;

        if (args[0] == NULL) {
            free(input);
            free(input_copy);
            continue;
        }

        if (strcmp(args[0], "grep") == 0) {
            if (args[1] && args[2]) {
                cmd_grep(args[1], args[2]);
            } else {
                printf("사용법: grep <검색어> <파일명>\n");
            }
            free(input);
            free(input_copy);
            continue;
        }
        
        if (strcmp(args[0], "cp") == 0) {
            if (args[1] && args[2]) {
                cmd_cp(args[1], args[2]);
            } else {
                printf("사용법: cp <원본파일> <대상파일>\n");
            }
            free(input);
            free(input_copy);
            continue;
        }
        
        if (strcmp(args[0], "mv") == 0) {
            if (args[1] && args[2]) {
                cmd_mv(args[1], args[2]);
            } else {
                printf("사용법: mv <원본파일> <대상파일>\n");
            }
            free(input);
            free(input_copy);
            continue;
        }
        
        if (strcmp(args[0], "ln") == 0) {
            if (args[1] && args[2]) {
                cmd_ln(args[1], args[2]);
            } else {
                printf("사용법: ln <원본파일> <링크이름>\n");
            }
            free(input);
            free(input_copy);
            continue;
        }

        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork 실패");
        } else if (pid == 0) {
            if (infile) {
                int fd = open(infile, O_RDONLY);
                if (fd < 0) {
                    perror("입력 파일 열기 실패");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            
            if (outfile) {
                int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("출력 파일 생성 실패");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            
            execvp(args[0], args);
            perror("명령어 실행 실패");
            exit(1);
            
        } else {
            if (!is_bg) {
                waitpid(pid, NULL, 0);
            } else {
                printf("[백그라운드 프로세스 PID: %d]\n", pid);
            }
        }
        
        free(input);
        free(input_copy);
    }
    
    return 0;
}