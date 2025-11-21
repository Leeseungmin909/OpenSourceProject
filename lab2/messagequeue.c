#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>           
#include <sys/stat.h>        
#include <mqueue.h>         
#include <signal.h>

#define QNAME1 "/chat_q1"
#define QNAME2 "/chat_q2"
#define MSG_SIZE 8192

mqd_t my_mq, peer_mq;
char *my_q_name;

void cleanup(int sig) {
    mq_close(my_mq);
    mq_close(peer_mq);
    mq_unlink(my_q_name);
    printf("\nChat ended.\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "인자값 숫자 '1' 또는 '2'를 입력해주세요. \n");
        exit(1);
    }

    int is_user1 = (atoi(argv[1]) == 1);
    char *peer_q_name;
    
    if (is_user1) {
        my_q_name = QNAME1;   
        peer_q_name = QNAME2; 
    } else {
        my_q_name = QNAME2;  
        peer_q_name = QNAME1;
    }

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;      // 최대 메시지 개수
    attr.mq_msgsize = MSG_SIZE; // 메시지 최대 크기
    attr.mq_curmsgs = 0;

    my_mq = mq_open(my_q_name, O_RDONLY | O_CREAT, 0644, &attr);
    if (my_mq == (mqd_t)-1) {
        perror("mq_open (my)");
        exit(1);
    }

    printf("상대방 연결 대기 중...\n");
    while (1) {
        peer_mq = mq_open(peer_q_name, O_WRONLY);
        if (peer_mq != (mqd_t)-1) break;
        usleep(500000); 
    }
    printf("채팅 시작\n");

    signal(SIGINT, cleanup);

    pid_t pid = fork();

    if (pid == 0) {
        char buf[MSG_SIZE];
        unsigned int prio;
        ssize_t n;

        while (1) {
            n = mq_receive(my_mq, buf, MSG_SIZE, &prio);
            if (n > 0) {
                buf[n] = '\0';
                printf("\r[상대방]: %s", buf); 
                printf("\n나: "); 
                fflush(stdout);
            } else if (n == -1) {
                perror("mq_receive");
                exit(1);
            }
        }
    } 
    else {
        char buf[MSG_SIZE];
        printf("나: ");
        fflush(stdout);

        while (fgets(buf, MSG_SIZE, stdin) != NULL) {
            buf[strcspn(buf, "\n")] = '\0';

            if (mq_send(peer_mq, buf, strlen(buf), 1) == -1) {
                perror("mq_send");
            }
            printf("나: ");
            fflush(stdout);
        }
        kill(pid, SIGINT); 
        cleanup(0);
    }
    return 0;
}