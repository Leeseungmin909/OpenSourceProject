#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define MSGSIZE 16

char *parent_name = "parent";
char *child_name = "child";
char *parent_msg = "Hello, child!";
char *child_msg = "Hello, parent!";

void nonblock_rw(char *name, int read_pipe, int write_pipe, char *message);

int main()
{
    int pp[2][2], i;
    int pid;

    for (i = 0; i < 2; i++) {
        if (pipe(pp[i]) == -1) {
            perror("pipe call failed");
            exit(1);
        }
    }

    pid = fork();

    if (pid == 0) { 
        close(pp[0][1]); 
        close(pp[1][0]);
        
        nonblock_rw(child_name, pp[0][0], pp[1][1], child_msg);
    }
    else if (pid > 0) { 
        close(pp[0][0]);
        close(pp[1][1]);
        
        nonblock_rw(parent_name, pp[1][0], pp[0][1], parent_msg);
    }
    else {
        perror("fork failed");
        exit(1);
    }
    
    return 0;
}

void nonblock_rw(char *name, int read_pipe, int write_pipe, char *message)
{
    char buf[MSGSIZE];
    int nread;

    if (fcntl(read_pipe, F_SETFL, O_NONBLOCK) == -1) {
        perror("read pipe call");
        exit(1);
    }
    if (fcntl(write_pipe, F_SETFL, O_NONBLOCK) == -1) {
        perror("write pipe call");
        exit(1);
    }

    for (;;) {
        switch (nread = read(read_pipe, buf, MSGSIZE)) {
        case -1:
            /* 데이터가 아직 준비되지 않음 (EAGAIN) */
            if (errno == EAGAIN) {
                printf("%s: pipe empty!\n", name);
                sleep(1);
                break;
            }
            else {
                perror("read call");
                exit(1);
            }
        case 0:
            /* 파이프가 닫힘 */
            printf("%s: read pipe closed\n", name);
            exit(1);
        default:
            /* 데이터 읽기 성공 */
            printf("%s: MSG=%s\n", name, buf);
        }

        /* 메시지 전송 */
        write(write_pipe, message, MSGSIZE);
        sleep(1);
    }
}