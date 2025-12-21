#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define PORT 9090
#define BUF_SIZE 1024

int main() {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    struct timeval timeout;
    fd_set reads, cpy_reads;
    
    socklen_t adr_sz;
    int fd_max, str_len, fd_num, i;
    char buf[BUF_SIZE];
    char msg_with_id[BUF_SIZE + 30]; 

    // 1. 서버 소켓 생성
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(PORT);

    if (bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1) {
        perror("bind() error"); exit(1);
    }
    if (listen(serv_sock, 5) == -1) {
        perror("listen() error"); exit(1);
    }

    // 2. select 설정 초기화
    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fd_max = serv_sock;

    printf("채팅 서버가 시작되었습니다 (Port: %d)...\n", PORT);

    while (1) {
        cpy_reads = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        if ((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1)
            break;
        if (fd_num == 0) continue;

        for (i = 0; i < fd_max + 1; i++) {
            if (FD_ISSET(i, &cpy_reads)) {
                
                // 새로운 연결 요청
                if (i == serv_sock) {
                    adr_sz = sizeof(clnt_adr);
                    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
                    
                    FD_SET(clnt_sock, &reads);
                    if (fd_max < clnt_sock) fd_max = clnt_sock;
                    
                    printf("새로운 클라이언트 연결됨: ID %d\n", clnt_sock);
                }
                // 데이터 수신
                else {
                    str_len = read(i, buf, BUF_SIZE);
                    
                    if (str_len == 0) { // 연결 종료
                        FD_CLR(i, &reads);
                        close(i);
                        printf("클라이언트 연결 종료: ID %d\n", i);
                    } else { 
                        buf[str_len] = 0; // 문자열 끝 처리

                        sprintf(msg_with_id, "[Client %d]: %s", i, buf);

                        printf("%s", msg_with_id);

                        for(int j = 0; j <= fd_max; j++) {
                            if (FD_ISSET(j, &reads) && j != serv_sock && j != i) {
                                write(j, msg_with_id, strlen(msg_with_id));
                            }
                        }
                    }
                }
            }
        }
    }
    close(serv_sock);
    return 0;
}