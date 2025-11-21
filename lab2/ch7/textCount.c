#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFSIZE 512        

int main()
{
    char buffer[BUFSIZE];
    int filedes;
    ssize_t nread;
    long total = 0;

    if ( (filedes = open ("anotherfile", O_RDONLY) ) == -1)
    {
        printf ("error in opening anotherfile\n");
        exit (1);
    }

    /* EOF 까지 반복하라. EOF 는 복귀값 0 에 의해 표시된다 */
    while ( (nread = read (filedes, buffer, BUFSIZE) ) > 0)
        total += nread;      

    printf ("total chars in anotherfile: %ld\n", total);
    exit (0);
}