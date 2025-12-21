#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 20
#define NUM_PRODUCERS 2     
#define NUM_CONSUMERS 2     

typedef struct {
    int item[BUFFER_SIZE];
    int totalitems;
    int in, out;
    pthread_mutex_t mutex;
    pthread_cond_t full;
    pthread_cond_t empty;
} buffer_t;

buffer_t bb = { {0}, 0, 0, 0, 
                PTHREAD_MUTEX_INITIALIZER, 
                PTHREAD_COND_INITIALIZER, 
                PTHREAD_COND_INITIALIZER 
};

int produce_item(int id) {
    int item = (int)(100.0 * rand() / (RAND_MAX + 1.0));
    sleep((unsigned long) (5.0 * rand() / (RAND_MAX + 1.0))); 
    printf("producer_item [%d]: item= %d\n", id, item);
    return item;
}

int insert_item(int id, int item) {
    int status;
    status = pthread_mutex_lock(&bb.mutex);
    if(status != 0) return status;

    while (bb.totalitems >= BUFFER_SIZE && status == 0) status = pthread_cond_wait(&bb.empty, &bb.mutex);
    if(status != 0){
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }

    bb.item[bb.in] = item;
    bb.in = (bb.in + 1) % BUFFER_SIZE;
    bb.totalitems++;

    if (status = pthread_cond_signal(&bb.full)){
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }

    return pthread_mutex_unlock(&bb.mutex);
}

void consume_item(int id, int item) {
    sleep((unsigned long) (5.0 * rand() / (RAND_MAX + 1.0))); 
    printf("\t\tconsumer_item [%d]: item= %d\n", id, item);
}

int remove_item(int id, int *item) {
    int status;
    status = pthread_mutex_lock(&bb.mutex);
    if(status != 0) return status;

    while (bb.totalitems <= 0 && status == 0) status = pthread_cond_wait(&bb.full, &bb.mutex);
    if(status != 0){
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }

    *item = bb.item[bb.out];
    bb.out = (bb.out + 1) % BUFFER_SIZE;
    bb.totalitems--;

    if(status = pthread_cond_signal(&bb.empty)){
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }

    return pthread_mutex_unlock(&bb.mutex);
}

void *producer(void *arg) {
    int id = *(int *)arg;
    int item;
    while (1) { 
        item = produce_item(id);
        insert_item(id, item);
    }
}

void *consumer(void *arg) {
    int id = *(int *)arg;
    int item;
    while (1) { 
        remove_item(id, &item);
        consume_item(id, item);
    }
}

int main() {
    pthread_t pro_tids[NUM_PRODUCERS];
    pthread_t con_tids[NUM_CONSUMERS];
    int pro_ids[NUM_PRODUCERS];
    int con_ids[NUM_CONSUMERS];
    int status;


    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pro_ids[i] = i + 1;
        status = pthread_create(&pro_tids[i], NULL, producer, (void *)&pro_ids[i]);
        if (status != 0) {
            perror("Create producer thread");
            exit(1);
        }
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        con_ids[i] = i + 1;
        status = pthread_create(&con_tids[i], NULL, consumer, (void *)&con_ids[i]);
        if (status != 0) {
            perror("Create consumer thread");
            exit(1);
        }
    }

    for (int i = 0; i < NUM_PRODUCERS; i++) pthread_join(pro_tids[i], NULL);
    
    for (int i = 0; i < NUM_CONSUMERS; i++) pthread_join(con_tids[i], NULL);
    
    pthread_mutex_destroy(&bb.mutex);
    pthread_cond_destroy(&bb.empty);
    pthread_cond_destroy(&bb.full);
}