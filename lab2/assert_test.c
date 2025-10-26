#include <stdio.h>
#include "my_assert.h" 
#include <stdlib.h>

void foo(int num) {
    my_assert((num >= 0) && (num <= 100));
    printf("foo: num = %d\n", num);
}

int main(int argc, char *argv[]) {
    int num;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s aNumber (0 <= aNumber <= 100)\n", argv[0]);
        exit(1);
    }

    num = atoi(argv[1]);
    foo(num);
    return 0;
}
