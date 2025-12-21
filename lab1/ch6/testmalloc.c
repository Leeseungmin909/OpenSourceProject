#include <stdio.h>
#include <stdlib.h>
int main()
{
    int max, i;
    int *ptr;

    printf("배열의 원소 개수는? ");
    scanf("%d",&max);

    ptr = (int *)malloc(max * sizeof(int));

    if (ptr == NULL){
        printf("메모리를 할당하지 못했습니다.");
        exit(-1);
    }

    for( i = 0; i < max; i++ )
        scanf("%d", &ptr[i]);

    printf("입력 숫자 리스트:");
    for( i = 0; i < max; i++ )
        printf("%d ", *(ptr+i));
    printf("\n");
    free((int *)ptr);
}
