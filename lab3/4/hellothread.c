#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// 부모 0, 자식 1
int turn = 1;

void *child_fun(void *arg){
    while(1){
        pthread_mutex_lock(&mutex);

        while(turn != 1) pthread_cond_wait(&cond, &mutex);
        printf("Chile thread: hello parent\n");
        sleep(1);

        turn = 0;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

// 메인 스레드가 부모 스레드이다.
int main(){
    pthread_t child_tid;
    int status;

    status = pthread_create(&child_tid, NULL, child_fun, NULL);
    if(status != 0){
        perror("Create thread failed");
        return status;
    }

    while(1){
        pthread_mutex_lock(&mutex);

        while(turn != 0) pthread_cond_wait(&cond, &mutex);
        printf("Parent thread: hello child\n");
        sleep(1);

        turn = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
    pthread_join(child_tid, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}