#include <stdio.h>
#include <time.h>
int main()
{
    time_t curtime;
    struct tm *tp, *gtp;

    time(&curtime);
    tp = localtime(&curtime);
    printf("ctime: %s", ctime(&curtime));
    printf("localtime: %d:%d:%d\n", tp->tm_hour, tp->tm_min, tp->tm_sec);

    gtp = gmtime(&curtime);
    printf("gmtime: %d:%d:%d\n", gtp->tm_hour, gtp->tm_min, gtp->tm_sec);

    printf("asctime: %s", asctime(tp));
    printf("mktime: time is %ld\n", mktime(tp));

    return 0;
}
