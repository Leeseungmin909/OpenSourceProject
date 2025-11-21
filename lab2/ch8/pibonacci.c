#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>
#include <stdlib.h>

void start(int signo);
sigjmp_buf jbuf; 
int main() {
    struct sigaction act;
    int cur_i = 1, past_i = 0, tmp_i;

    if (sigsetjmp(jbuf, 1) == 0) {
        act.sa_handler = start;
        sigaction(SIGINT, &act, NULL); 
    }
    
    while (1) {
        printf("%d\n", cur_i);
        tmp_i = cur_i;
        cur_i += past_i;
        past_i = tmp_i;
        sleep(1);
    }
}

void start(int signo) {
    fprintf(stderr, "Interrupted\n");
    siglongjmp(jbuf, 1); 
}