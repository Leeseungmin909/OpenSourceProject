#include <stdio.h>
#include <string.h>
#include <time.h>

int main() {
    const char *text = "The magic thing is that you can change it.";
    char input[1024];
    int errcnt = 0;

    printf("다음 문장을 입력하고 엔터(Enter)를 누르세요:\n\n%s\n\n", text);

    time_t start_time = time(NULL);

    if (fgets(input, sizeof(input), stdin) == NULL) {
        return 1; 
    }

    time_t end_time = time(NULL);

    input[strcspn(input, "\n")] = '\0';

    int text_len = strlen(text);
    int input_len = strlen(input);
    
    int max_len = (text_len > input_len) ? text_len : input_len;

    for (int i = 0; i < max_len; i++) {
        if (i >= input_len || i >= text_len || text[i] != input[i]) {
            errcnt++;
        }
    }

    printf("\n--- 결과 ---\n");
    
    double duration = difftime(end_time, start_time);
    int cpm = 0;
    if (duration > 0) {
        cpm = (int)((input_len / duration) * 60);
    }

    printf("총 걸린 시간: %.0f초\n", duration);
    printf("타이핑 오류: %d개\n", errcnt);
    printf("평균 분당 타자수: %d타\n", cpm);

    return 0;
}