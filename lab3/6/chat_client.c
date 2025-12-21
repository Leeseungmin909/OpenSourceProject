#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    int sock;
    char buf[BUF_SIZE];
    struct sockaddr_in serv_adr;
    fd_set reads, cpy_reads;
    int str_len, fd_max;

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    // 서버에 연결 요청
    if (connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        perror("connect() error"); exit(1);
    } else {
        printf("서버에 연결되었습니다.\n");
    }

    FD_ZERO(&reads);
    FD_SET(0, &reads);    // 표준 입력 감시 등록
    FD_SET(sock, &reads); // 서버 소켓 감시 등록
    fd_max = sock;

    while (1) {
        cpy_reads = reads;
        
        // 입력이나 수신이 발생할 때까지 대기 
        if (select(fd_max + 1, &cpy_reads, 0, 0, 0) == -1) break;

        // 1. 서버에서 데이터가 온 경우
        if (FD_ISSET(sock, &cpy_reads)) {
            str_len = read(sock, buf, BUF_SIZE - 1);
            if (str_len == 0) break; // 연결 종료
            
            buf[str_len] = 0;
            printf("%s", buf);
        }

        // 2. 키보드 입력이 있는 경우
        if (FD_ISSET(0, &cpy_reads)) {
            if (fgets(buf, BUF_SIZE, stdin) != NULL) {
                // Q 누르면 통신 종료
                if (strcmp(buf, "q\n") == 0 || strcmp(buf, "Q\n") == 0) break;

                printf("\033[1A\033[2K"); 
                printf("[Me] > %s", buf);

                write(sock, buf, strlen(buf)); // 서버 전송
            }
        }
    }
    close(sock);
    return 0;
}