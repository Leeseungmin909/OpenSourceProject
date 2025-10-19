#include <stdio.h>

double divide(int a, int b){
	if(b == 0){
		printf("b에는 0이 들어 올 수 없습니다.");
		return 0.0;
	}
	return (double)a/b;
}
