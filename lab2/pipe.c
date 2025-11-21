#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

#define MSGSIZE 1024

void chat_loop(int read_pipe, int write_pipe, char *name) {
    char buf[MSGSIZE];
    fd_set set, master;
    int nread;

    FD_ZERO(&master);
    FD_SET(0, &master);         
    FD_SET(read_pipe, &master);  

    int max_fd = (read_pipe > 0) ? read_pipe : 0;
    printf("%s님과 채팅 시작\n",name);
    
    while (1) {
        set = master;

        if (select(max_fd + 1, &set, NULL, NULL, NULL) == -1) {
            perror("select error");
            exit(1);
        }

        if (FD_ISSET(0, &set)) {
            nread = read(0, buf, MSGSIZE);
            if (nread > 0) {
                buf[nread] = '\0'; 
                write(write_pipe, buf, nread);
            } else {
                break; 
            }
        }

        if (FD_ISSET(read_pipe, &set)) {
            nread = read(read_pipe, buf, MSGSIZE);
            if (nread > 0) {
                buf[nread] = '\0';
                printf("[%s]: %s", name, buf); 
            } else {
                printf("연결이 종료 되었습니다.\n");
                break; 
            }
        }
    }
}

int main() {
    int p1[2]; // 부모 -> 자식
    int p2[2]; // 자식 -> 부모
    int pid;

    if (pipe(p1) == -1 || pipe(p2) == -1) {
        perror("pipe failed");
        exit(1);
    }

    pid = fork();

    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid > 0) { // 부모 프로세스
        close(p1[0]); 
        close(p2[1]); 

        chat_loop(p2[0], p1[1], "Parent");

        close(p1[1]);
        close(p2[0]);
        wait(NULL); 
    } else if (pid == 0) { // 자식 프로세스
        close(p1[1]); 
        close(p2[0]); 

        chat_loop(p1[0], p2[1], "Child");

        close(p1[0]);
        close(p2[1]);
    }

    return 0;
}