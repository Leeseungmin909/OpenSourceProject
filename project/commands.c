#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "commands.h"

void cmd_cp(char *src, char *dest) {
    int src_fd, dst_fd;
    char buf[4096];
    ssize_t rcnt, wcnt;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    if ((src_fd = open(src, O_RDONLY)) == -1) {
        perror("cp: 원본 파일 열기 실패");
        return;
    }

    if ((dst_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, mode)) == -1) {
        perror("cp: 대상 파일 생성 실패");
        close(src_fd);
        return;
    }

    while ((rcnt = read(src_fd, buf, sizeof(buf))) > 0) {
        wcnt = write(dst_fd, buf, rcnt);
        if (wcnt != rcnt) {
            perror("cp: 쓰기 오류");
            break;
        }
    }

    if (rcnt < 0) {
        perror("cp: 읽기 오류");
    }

    close(src_fd);
    close(dst_fd);
    
    printf("cp: '%s' -> '%s' 복사 완료\n", src, dest);
}

void cmd_mv(char *src, char *dest) {
    struct stat buf;
    char *target;
    char *src_file_name_only;

    if (access(src, F_OK) < 0) {
        fprintf(stderr, "mv: '%s' 파일이 존재하지 않음\n", src);
        return;
    }

    char *slash = strrchr(src, '/');
    src_file_name_only = src;
    if (slash != NULL) {
        src_file_name_only = slash + 1;
    }

    target = (char *)malloc(strlen(dest) + 1);
    strcpy(target, dest);

    if (access(dest, F_OK) == 0) {
        if (lstat(dest, &buf) < 0) {
            perror("mv: stat 실패");
            free(target);
            return;
        }
        if (S_ISDIR(buf.st_mode)) {
            free(target);
            target = (char *)malloc(strlen(src) + strlen(dest) + 2);
            strcpy(target, dest);
            strcat(target, "/");
            strcat(target, src_file_name_only);
        }
    }

    if (rename(src, target) < 0) {
        perror("mv: 이름 변경 실패");
        free(target);
        return;
    }

    printf("mv: '%s' -> '%s' 이동 완료\n", src, target);
    free(target);
}

void cmd_ln(char *src, char *dest) {
    if (access(src, F_OK) < 0) {
        fprintf(stderr, "ln: '%s' 파일이 존재하지 않음\n", src);
        return;
    }

    if (link(src, dest) < 0) {
        perror("ln: 링크 생성 실패");
        return;
    }
    
    printf("ln: '%s' -> '%s' 하드 링크 생성 완료\n", src, dest);
}

void cmd_grep(char *pattern, char *filename) {
    FILE *fp;
    char line[1024];
    int line_num = 0;
    int found = 0;

    if ((fp = fopen(filename, "r")) == NULL) {
        perror("grep: 파일 열기 실패");
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        line_num++;
        if (strstr(line, pattern) != NULL) {
            printf("%s", line);
            found = 1;
        }
    }

    if (!found) {
        printf("grep: '%s'에서 '%s' 패턴을 찾을 수 없음\n", filename, pattern);
    }

    fclose(fp);
}