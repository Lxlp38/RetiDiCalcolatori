#include <signal.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>

# define TMIN 80
# define TMAX 120
# define T 500
# define N 10

#define NUM_TRAINS 800


pthread_mutex_t mutex_trains_wait;
int trains_wait = 0;

pthread_mutex_t mutex_trains_in;
sem_t semaphore_trains_in;
int trains_in[N];

pthread_mutex_t mutex_trains_done;
int trains_done = 0;


pthread_mutex_t mutex_id;
int id = 0;


int inToString(){
    int k = 0;
    for(int i = 0; i < N; i++){
        if(trains_in[i] != -1){
            k++;
        }
    }
    return k;
}

void printstatus(int id, char* extra){
    printf("id: %d\t\t wait: %d\t\t in: %d\t\t done: %d\t\t %s\n", id, trains_wait, inToString(), trains_done, extra);
    for (int i = 0; i < N; i++) {
        printf("%d\t", trains_in[i]);
    }
    printf("\n");
}

void train_handler(int pre, int post){
    pthread_mutex_lock(&mutex_trains_in);
    for(int i = 0; i < N; i++){
        if(trains_in[i] == pre){
            trains_in[i] = post;
            break;
        }
    }
    pthread_mutex_unlock(&mutex_trains_in);
}

void* train_task(void* arg){
    pthread_mutex_lock(&mutex_id);
    id++;
    int thread_id = id;
    pthread_mutex_unlock(&mutex_id);

    // Arrivo in stazione
    pthread_mutex_lock(&mutex_trains_wait);
    trains_wait++;
    printstatus(thread_id, "-->wait");
    pthread_mutex_unlock(&mutex_trains_wait);

    // Arrivo al binario
    sem_wait(&semaphore_trains_in);
    pthread_mutex_lock(&mutex_trains_wait);
    trains_wait--;
    pthread_mutex_unlock(&mutex_trains_wait);
    train_handler(0,thread_id);
    printstatus(thread_id, "-->in");

    // Busy al binario
    usleep(T*1000);

    // Partenza da binario
    train_handler(thread_id, 0);
    pthread_mutex_lock(&mutex_trains_done);
    trains_done++;
    pthread_mutex_unlock(&mutex_trains_done);
    sem_post(&semaphore_trains_in);
    printstatus(thread_id, "-->done");

    return NULL;
}

int main(int argc, char const *argv[])
{

    for(int i = 0; i < N; i++){
        trains_in[i] = 0;
    }

    pthread_t threads[NUM_TRAINS];
    
    pthread_mutex_init(&mutex_trains_done, NULL);
    pthread_mutex_init(&mutex_trains_in, NULL);
    pthread_mutex_init(&mutex_trains_wait, NULL);
    pthread_mutex_init(&mutex_id, NULL);

    sem_init(&semaphore_trains_in, 0, N);

    for (int i = 0; i < NUM_TRAINS; i++) {
        int random_time = (rand() % (TMAX - TMIN + 1)) + TMIN;
        usleep(random_time*1000);
        pthread_create(&threads[id], NULL, train_task, NULL);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex_trains_done);
    pthread_mutex_destroy(&mutex_trains_in);
    pthread_mutex_destroy(&mutex_trains_wait);
    pthread_mutex_destroy(&mutex_id);

    return 0;
}
