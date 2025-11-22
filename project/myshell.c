#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "commands.h" 

#define MAX_CMD_LEN 1024
#define MAX_ARG_LEN 64

int main() {
    char input[MAX_CMD_LEN];
    char *args[MAX_ARG_LEN];

    while (1) {
        printf("MyShell> ");
        if (fgets(input, MAX_CMD_LEN, stdin) == NULL) break; 
        
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;

        if (strcmp(input, "exit") == 0) break;

        int is_bg = 0;
        if (input[strlen(input) - 1] == '&') {
            is_bg = 1;
            input[strlen(input) - 1] = 0; 
        }

        char *pipe_pos = strchr(input, '|');
        if (pipe_pos != NULL) {
            *pipe_pos = 0; 
            char *cmd1_str = input;
            char *cmd2_str = pipe_pos + 1;
            int fd[2];
            
            if (pipe(fd) == -1) { perror("파이프 생성 실패"); continue; }

            if (fork() == 0) { 
                close(fd[0]); dup2(fd[1], STDOUT_FILENO); close(fd[1]);
                
                char *p_args[MAX_ARG_LEN];
                int i=0; p_args[i] = strtok(cmd1_str, " \t");
                while(p_args[i] != NULL) p_args[++i] = strtok(NULL, " \t");
                execvp(p_args[0], p_args);
                perror("첫 번째 명령어 실행 실패"); 
                exit(1);
            }

            if (fork() == 0) { 
                close(fd[1]); dup2(fd[0], STDIN_FILENO); close(fd[0]);

                char *p_args[MAX_ARG_LEN];
                int i=0; p_args[i] = strtok(cmd2_str, " \t");
                while(p_args[i] != NULL) p_args[++i] = strtok(NULL, " \t");
                execvp(p_args[0], p_args);
                perror("두 번째 명령어 실행 실패"); 
                exit(1);
            }

            close(fd[0]); close(fd[1]);
            wait(NULL); wait(NULL);
            continue; 
        }

        int i = 0;
        char *token = strtok(input, " \t");
        char *infile = NULL, *outfile = NULL;

        while (token != NULL) {
            if (strcmp(token, "<") == 0) {
                infile = strtok(NULL, " \t");
            } else if (strcmp(token, ">") == 0) {
                outfile = strtok(NULL, " \t");
            } else {
                args[i++] = token;
            }
            token = strtok(NULL, " \t");
        }
        args[i] = NULL; 

        if (args[0] == NULL) continue;

        if (strcmp(args[0], "cp") == 0) {
            if (args[1] && args[2]) cmd_cp(args[1], args[2]);
            else printf("사용법: cp <원본파일> <대상파일>\n");
            continue;
        }
        if (strcmp(args[0], "mv") == 0) {
            if (args[1] && args[2]) cmd_mv(args[1], args[2]);
            else printf("사용법: mv <원본파일> <대상파일>\n");
            continue;
        }
        if (strcmp(args[0], "ln") == 0) {
            if (args[1] && args[2]) cmd_ln(args[1], args[2]);
            else printf("사용법: ln <원본파일> <링크이름>\n");
            continue;
        }
        if (strcmp(args[0], "grep") == 0) {
            if (args[1] && args[2]) cmd_grep(args[1], args[2]);
            else printf("사용법: grep <검색어> <파일명>\n");
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("프로세스 생성(fork) 실패");
        } else if (pid == 0) { 
            if (infile) {
                int fd = open(infile, O_RDONLY);
                if(fd < 0) { perror("입력 파일 열기 실패"); exit(1); }
                dup2(fd, STDIN_FILENO); close(fd);
            }
            if (outfile) {
                int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if(fd < 0) { perror("출력 파일 생성 실패"); exit(1); }
                dup2(fd, STDOUT_FILENO); close(fd);
            }
            
            execvp(args[0], args);
            perror("명령어를 찾을 수 없거나 실행 실패"); 
            exit(1);
        } else { 
            if (!is_bg) { 
                waitpid(pid, NULL, 0);
            } else { 
                printf("[백그라운드 실행 PID: %d]\n", pid);
            }
        }
    }
    return 0;
}