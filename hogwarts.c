#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <errno.h>
#include <limits.h>
#define NUM_ELVES 5
#define HASH_BUFFER_LENGTH 32
unsigned int random_seed = 252;
int task_id_counter = 0;
typedef struct { int id; unsigned char *input; } task;
typedef struct node { task *task; struct node *next; } node;
node *list_head = NULL;
int total_tasks, active_tasks = 0, finished = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t available = PTHREAD_COND_INITIALIZER;
pthread_cond_t drained = PTHREAD_COND_INITIALIZER;
void post_tasks(int);
task *take_task(void);
void do_work(task *);

/* Producer fills one batch at a time. Every shared queue predicate is guarded. */
void *dobby(void *unused) {
    (void)unused;
    pthread_mutex_lock(&mutex);
    while (total_tasks > 0) {
        while (active_tasks > 0) pthread_cond_wait(&drained, &mutex);
        int batch = total_tasks < 10 ? total_tasks : 10;
        post_tasks(batch); active_tasks = batch; total_tasks -= batch;
        pthread_cond_broadcast(&available);
    }
    finished = 1;
    pthread_cond_broadcast(&available);
    pthread_mutex_unlock(&mutex);
    return NULL;
}
void *house_elf(void *unused) {
    (void)unused;
    for (;;) {
        pthread_mutex_lock(&mutex);
        while (active_tasks == 0 && !finished) pthread_cond_wait(&available, &mutex);
        if (active_tasks == 0 && finished) { pthread_mutex_unlock(&mutex); return NULL; }
        task *item = take_task();
        if (--active_tasks == 0) pthread_cond_signal(&drained);
        pthread_mutex_unlock(&mutex);
        do_work(item);
    }
}
int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "Usage: %s total_tasks\n", argv[0]); return 1; }
    char *end; errno = 0; long value = strtol(argv[1], &end, 10);
    if (errno || end == argv[1] || *end || value < 0 || value > INT_MAX) return 1;
    total_tasks = (int)value;
    printf("There are %d tasks to do today.\n", total_tasks);
    pthread_t workers[NUM_ELVES], producer;
    for (int i = 0; i < NUM_ELVES; i++) {
        if (pthread_create(&workers[i], NULL, house_elf, NULL)) {
            pthread_mutex_lock(&mutex); finished = 1; pthread_cond_broadcast(&available); pthread_mutex_unlock(&mutex);
            for (int j=0; j<i; j++) pthread_join(workers[j], NULL);
            return 1;
        }
    }
    if (pthread_create(&producer, NULL, dobby, NULL)) {
        pthread_mutex_lock(&mutex); finished = 1; pthread_cond_broadcast(&available); pthread_mutex_unlock(&mutex);
        for (int i=0; i<NUM_ELVES; i++) pthread_join(workers[i], NULL);
        return 1;
    }
    pthread_join(producer, NULL);
    for (int i=0; i<NUM_ELVES; i++) pthread_join(workers[i], NULL);
    pthread_mutex_destroy(&mutex); pthread_cond_destroy(&available); pthread_cond_destroy(&drained);
    return 0;
}

/* Generate random string code based off original by Ates Goral */
char* random_string(const int len, unsigned int * generator_seed) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    char* s = malloc( (len+1) * sizeof( char ));

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand_r(generator_seed) % (sizeof(alphanum) - 1)];
    }
    s[len] = 0;
    return s;
}

void post_tasks( int howmany ) {
    printf("Adding %d tasks to the list of active tasks.\n", howmany);
    for ( int i = 0; i < howmany; ++i ) {
        task* t = malloc( sizeof( task ));
        t->id = ++task_id_counter;
        t->input = (unsigned char *)random_string( HASH_BUFFER_LENGTH, &random_seed );
        node* n = malloc( sizeof( node ));
        n->task = t;
        n->next = list_head;
        list_head = n;
    }
}

void do_work( task* todo ) {
     unsigned char* output_buffer = calloc( HASH_BUFFER_LENGTH , sizeof ( unsigned char ) );
     SHA256( todo->input, HASH_BUFFER_LENGTH, output_buffer );
     printf("Task %d was completed!\n", todo->id);
     free( output_buffer );
     free( todo->input );
     free( todo );
}

task* take_task( ) {
    if (list_head == NULL) {
        /* Something has gone wrong */
        printf("Trying to take from an empty list!\n");
        exit( -1 );
    }
    node* head = list_head;
    task* t = head->task;
    list_head = list_head->next;
    free( head );
    return t;
}
