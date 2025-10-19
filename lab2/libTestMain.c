#include <stdio.h>
#include <testlib.h>

int main(){
	int a = 100;
       	int b = 10;
	
	printf("add(%d,%d) = %d\n", a,b,add(a,b));
	printf("subtract(%d,%d) = %d\n", a,b,subtract(a,b));
	printf("multiple(%d,%d) = %d\n", a,b,multiply(a,b));
	printf("divide(%d,%d) = %lf\n", a,b,divide(a,b));

}
