#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *method = getenv("REQUEST_METHOD");
    char *query = NULL;
    char post_data[1024] = {0,};

    printf("Content-type: text/html\r\n\r\n");
    printf("<html><body>");
    printf("<h1>CGI Result</h1>");
    printf("<p>Method: %s</p>", method);

    // GET 방식 처리
    if (method && strcmp(method, "GET") == 0) {
        query = getenv("QUERY_STRING");
        if(query) printf("<p>GET Data: %s</p>", query);
    } 
    // POST 방식 처리
    else if (method && strcmp(method, "POST") == 0) {
        if (fgets(post_data, sizeof(post_data), stdin) != NULL) {
             printf("<p>POST Data: %s</p>", post_data);
             query = post_data; 
        }
    }

    // 간단한 파싱 및 계산 
    if (query && strstr(query, "a=") && strstr(query, "b=")) {
        int a = 0, b = 0;
        char *ptr_a = strstr(query, "a=");
        char *ptr_b = strstr(query, "b=");
        if(ptr_a) a = atoi(ptr_a + 2);
        if(ptr_b) b = atoi(ptr_b + 2);
        
        printf("<h2>Calculation: %d + %d = %d</h2>", a, b, a + b);
    }

    printf("</body></html>");
    return 0;
}