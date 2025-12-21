#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define MAX_CLNT 256

typedef struct {
    int sock;
    char name[20];
    int room;
} Client;

void *handle_clnt(void *arg);
void send_msg(Client *sender, char *msg, int len);
void error_handling(char *msg);

// mode: 0(Login), 1(Register)
// return: 0(Success), -1(Fail:PW/Exist), -2(Fail:NoID/Dup)
int process_auth(char *mode, char *id, char *pw);

int clnt_cnt = 0;
Client *clnt_list[MAX_CLNT];
pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    int clnt_adr_sz;
    pthread_t t_id;

    if (argc != 2) {
        printf("사용법 : %s <port>\n", argv[0]);
        exit(1);
    }

    pthread_mutex_init(&mutx, NULL);
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    
    int opt = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    printf(">>> 서버 시작됨 ...\n");

    while (1) {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);

        Client *new_clnt = (Client *)malloc(sizeof(Client));
        new_clnt->sock = clnt_sock;
        new_clnt->room = 0; 

        pthread_mutex_lock(&mutx);
        clnt_list[clnt_cnt++] = new_clnt;
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id, NULL, handle_clnt, (void *)new_clnt);
        pthread_detach(t_id);
        printf(">>> 새 접속자 IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
    }
    close(serv_sock);
    return 0;
}

int process_auth(char *mode, char *id, char *pw) {
    FILE *fp;
    char file_id[20], file_pw[20];
    int found = 0;
    
    pthread_mutex_lock(&mutx); 

    fp = fopen("users.txt", "r+");
    if (fp == NULL) fp = fopen("users.txt", "w+");

    while (fscanf(fp, "%s %s", file_id, file_pw) != EOF) {
        if (strcmp(file_id, id) == 0) {
            found = 1;
            break;
        }
    }

    if (strcmp(mode, "REGISTER") == 0) {
        if (found) {
            pthread_mutex_unlock(&mutx);
            fclose(fp);
            return -1; // 이미 존재하는 ID
        }
        fseek(fp, 0, SEEK_END);
        fprintf(fp, "%s %s\n", id, pw);
        printf(">>> [회원가입] 새 유저: %s\n", id);
        fclose(fp);
        pthread_mutex_unlock(&mutx);
        return 0; // 가입 성공
    } 
    else { // LOGIN
        fclose(fp);
        pthread_mutex_unlock(&mutx);
        
        if (!found) return -2; // ID 없음
        if (strcmp(file_pw, pw) == 0) return 0; // 로그인 성공
        else return -1; // 비번 틀림
    }
}

void *handle_clnt(void *arg)
{
    Client *clnt = (Client *)arg;
    int str_len = 0;
    char msg[BUF_SIZE];
    char auth_buf[BUF_SIZE]; 

    int read_len = read(clnt->sock, auth_buf, sizeof(auth_buf)-1);
    if (read_len <= 0) {
        // 접속 끊김
    } else {
        auth_buf[read_len] = 0;
        char mode[10], id[20], pw[20];
        sscanf(auth_buf, "%s %s %s", mode, id, pw);

        int already_logged_in = 0;
        if (strcmp(mode, "LOGIN") == 0) {
            pthread_mutex_lock(&mutx);
            for(int i=0; i<clnt_cnt; i++) {
                if(clnt_list[i]->sock != clnt->sock && strcmp(clnt_list[i]->name, id) == 0) {
                    already_logged_in = 1;
                    break;
                }
            }
            pthread_mutex_unlock(&mutx);
        }

        if (already_logged_in) {
            char *fail_msg = "실패: 이미 접속중인 아이디입니다.";
            write(clnt->sock, fail_msg, strlen(fail_msg));
            goto DISCONNECT;
        }

        int result = process_auth(mode, id, pw);

        if (result == 0) {
            write(clnt->sock, "OK", 2);
            if (strcmp(mode, "LOGIN") == 0) {
                strcpy(clnt->name, id);
                sprintf(msg, ">>> 서버: 환영합니다 %s님! (로비 입장)\n", clnt->name);
                write(clnt->sock, msg, strlen(msg));
            }
        } else {
            if (strcmp(mode, "REGISTER") == 0) {
                char *fail_msg = "실패: 이미 존재하는 아이디입니다.";
                write(clnt->sock, fail_msg, strlen(fail_msg));
            }
            else if (result == -2) {
                char *fail_msg = "실패: 존재하지 않는 아이디입니다.";
                write(clnt->sock, fail_msg, strlen(fail_msg));
            }
            else {
                char *fail_msg = "실패: 비밀번호가 틀렸습니다.";
                write(clnt->sock, fail_msg, strlen(fail_msg));
            }
            goto DISCONNECT;
        }
    }

    if (strlen(clnt->name) == 0) goto DISCONNECT;

    while ((str_len = read(clnt->sock, msg, sizeof(msg))) != 0) {
        msg[str_len] = 0;
        if (msg[0] == '/') {
            if (strncmp(msg, "/join", 5) == 0) {
                int new_room = atoi(msg + 6);
                clnt->room = new_room;
                sprintf(msg, ">>> 서버: %d번 방으로 이동했습니다.\n", new_room);
                write(clnt->sock, msg, strlen(msg));
            }
            continue;
        }
        send_msg(clnt, msg, str_len);
    }

DISCONNECT:
    pthread_mutex_lock(&mutx);
    for (int i = 0; i < clnt_cnt; i++) {
        if (clnt->sock == clnt_list[i]->sock) {
            while (i++ < clnt_cnt - 1)
                clnt_list[i] = clnt_list[i + 1];
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt->sock);
    free(clnt);
    return NULL;
}

void send_msg(Client *sender, char *msg, int len)
{
    pthread_mutex_lock(&mutx);
    for (int i = 0; i < clnt_cnt; i++) {
        if (strlen(clnt_list[i]->name) > 0 && clnt_list[i]->room == sender->room) {
            if (strncmp(msg, "[FILE]", 6) == 0) {
                 write(clnt_list[i]->sock, msg, len);
            } else {
                char final_msg[BUF_SIZE + 30];
                sprintf(final_msg, "[%s] %s", sender->name, msg);
                write(clnt_list[i]->sock, final_msg, strlen(final_msg));
            }
        }
    }
    pthread_mutex_unlock(&mutx);
}

void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}