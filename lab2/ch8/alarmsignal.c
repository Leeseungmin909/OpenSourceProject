#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int alarm_flag = 0;

void alarm_handler(int sig) {
    printf("Received a alarm signal.\n");
    alarm_flag = 1;
}

int main() {
    struct sigaction act;
    
    act.sa_handler = alarm_handler;
    sigemptyset(&(act.sa_mask)); 
    act.sa_flags = 0; 
    sigaction(SIGALRM, &act, NULL);
    
    alarm(5); 

    pause(); 

    if (alarm_flag) {
        printf("Passed a 5 secs.\n");
    }

    return 0;
}