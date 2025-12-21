#include <stdio.h>
#include <search.h>
#define TABLESIZE 10

int table[TABLESIZE] = {1,2,3,4,5};
size_t n=5;
int compare(const void *ap, const void *bp){
	return ( *(int *)ap - *(int *)bp);	
}

int main(){
    int item, *ptr;

    item = 6;
    ptr = (int *)lsearch(&item, table, &n, sizeof(int), compare);

    if (ptr == &table[n-1] && n > 5) {
        printf("%d is not in the table, but added. (current size: %zu)\n", item, n);
    } else {
        printf("%d is in the table. (current size: %zu)\n", *ptr, n);
    }

    item = 7;
    ptr = (int *)lfind(&item, table, &n, sizeof(int), compare);

    if (ptr == NULL){
        printf("%d is not in the table. (current size: %zu)\n", item, n);
    } else {
        printf("%d is in the table. (current size: %zu)\n", *ptr, n);
    }
    
    return 0;
}
