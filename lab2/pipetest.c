#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MSGSIZE 16

int main() {
    char buf[MSGSIZE];
    int p[2], i;
    int pid;

    /* open pipe */
    /* p[0]: 읽기용, p[1]: 쓰기용 */
    if (pipe(p) == -1) {
        perror("pipe call failed");
        exit(1);
    }

    pid = fork();

    if (pid == 0) { 
        /* child process */
        /* 자식은 파이프에 쓰기를 수행하므로, 읽기 디스크립터(p[0])를 닫음 */
        close(p[0]); 

        /* write to pipe */
        for (i = 0; i < 2; i++) {
            sprintf(buf, "Hello, world #%d", i + 1);
            write(p[1], buf, MSGSIZE);
        }
    }
    else if (pid > 0) {
        /* parent process */
        /* 부모는 파이프에서 읽기를 수행하므로, 쓰기 디스크립터(p[1])를 닫음 */
        close(p[1]);

        /* read from pipe */
        for (i = 0; i < 2; i++) {
            read(p[0], buf, MSGSIZE);
            printf("%s\n", buf);
        }
    }
    else {
        perror("fork failed");
        exit(1);
    }

    return 0;
}