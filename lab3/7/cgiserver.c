#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define PORT 9190
#define BUF_SIZE 2048

void read_childproc(int sig) {
    int status;
    waitpid(-1, &status, WNOHANG);
}

void handle_request(int clnt_sock);

int main() {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t adr_sz;
    struct sigaction act;

    // 1. 좀비 프로세스 처리 설정
    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, 0);

    // 2. 소켓 생성 및 설정
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(PORT);

    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        perror("bind() error"); exit(1);
    }
    if (listen(serv_sock, 5) == -1) {
        perror("listen() error"); exit(1);
    }

    printf("Simple Web Server started on port %d...\n", PORT);

    // 3. 연결 대기 루프
    while (1) {
        adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
        if (clnt_sock == -1) continue;

        int pid = fork();
        if (pid == 0) { 
            close(serv_sock); 
            handle_request(clnt_sock);
            close(clnt_sock);
            exit(0);
        } else {
            close(clnt_sock); 
        }
    }
    close(serv_sock);
    return 0;
}

void handle_request(int clnt_sock) {
    char buf[BUF_SIZE] = {0,};
    char method[10];
    char file_name[100]; // 파일명 그릇
    char *body = NULL;
    
    // 요청 읽기
    int str_len = read(clnt_sock, buf, BUF_SIZE - 1);
    if (str_len <= 0) return;
    
    // 헤더 파싱
    sscanf(buf, "%s %s", method, file_name);
    
    // Body 찾기
    char *body_ptr = strstr(buf, "\r\n\r\n");
    if (body_ptr) body = body_ptr + 4;

    if (strcmp(file_name, "/") == 0) strcpy(file_name, "/index.html");

    // CGI 실행
    if (strstr(file_name, ".cgi") != NULL) {
        write(clnt_sock, "HTTP/1.1 200 OK\r\n", 17);

        int input_pipe[2];
        pipe(input_pipe);

        if (fork() == 0) {
            dup2(clnt_sock, STDOUT_FILENO);
            dup2(input_pipe[0], STDIN_FILENO);
            close(input_pipe[1]);

            setenv("REQUEST_METHOD", method, 1);
            if(body) {
                char len[32]; // [수정] 10 -> 32로 넉넉하게 늘림
                sprintf(len, "%ld", strlen(body));
                setenv("CONTENT_LENGTH", len, 1);
            }
            
            char *query = strchr(file_name, '?');
            if(query) { *query = 0; setenv("QUERY_STRING", query+1, 1); }

            execl(file_name + 1, file_name + 1, NULL);
            exit(1);
        } else {
            close(input_pipe[0]);
            if (body && strcmp(method, "POST") == 0) 
                write(input_pipe[1], body, strlen(body));
            close(input_pipe[1]);
            wait(NULL);
        }
    } 
    else { 
        char path[200]; // [수정] 100 -> 200으로 넉넉하게 늘림
        sprintf(path, ".%s", file_name);
        int fd = open(path, O_RDONLY);
        
        if (fd != -1) {
            write(clnt_sock, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n", 44);
            int len;
            while ((len = read(fd, buf, BUF_SIZE)) > 0) write(clnt_sock, buf, len);
            close(fd);
        } else {
            write(clnt_sock, "HTTP/1.1 404 Not Found\r\n\r\n404 Error", 36);
        }
    }
}