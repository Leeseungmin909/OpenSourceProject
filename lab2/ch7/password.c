#include <stdio.h>
#include <termios.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define PASSWORDSIZE 12

int main(void)
{
    int fd;
    int nread;
    char buf[PASSWORDSIZE], passwd[]="secret";
    struct termios init_attr, new_attr;

    fd = open(ttyname(fileno(stdin)), O_RDWR);
    tcgetattr(fd, &init_attr);
    new_attr = init_attr;
    new_attr.c_lflag &= ~ECHO; /* ~(ECHO | ECHOE | ECHOK | ECHONL) */

    if (tcsetattr(fd, TCSANOW, &new_attr) != 0) {
        fprintf(stderr, "터미널 속성을 설정할 수 없음.\n");
    }

    write(fd, "Password: ", 10);
    while ((nread=read(fd, buf, PASSWORDSIZE)) > 0) {
        buf[nread-1] = '\0';
        printf("\n");
        if (strcmp(buf, passwd) == 0)
            break;
        printf("패스워드가 틀렸습니다.\n");
        write(fd, "Password: ", 11);
    }
    printf("패스워드 입력이 성공했습니다.\n");
    tcsetattr(fd, TCSANOW, &init_attr);
    close(fd);
}