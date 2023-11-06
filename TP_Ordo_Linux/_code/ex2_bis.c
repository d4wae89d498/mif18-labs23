#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <stdlib.h>
#include <pthread.h>
#include <syscall.h>

/*MIF18 : Lab 5 Linux Scheduling, ex2 version threads*/
/* Grégoire Pichon, 2022 */

#define NB_CALLS 100000
#define MS_DIFF(END, START) ((END.tv_sec - START.tv_sec) * 1000000 + (END.tv_nsec - START.tv_nsec) / 1000)
#define MAX_THREADS 256

void *work(void *useless)
{
    unsigned long   elapsed_time;
    struct timespec start, end;
    unsigned        y;

    while (1)
    {
        clock_gettime(CLOCK_MONOTONIC, &start);
        y = 0;
        while (++y <= NB_CALLS)
            sched_yield();
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed_time = MS_DIFF(end, start);
        printf("Calling %i sched_yield() took %8lu microseconds id %lu\n",
               NB_CALLS, elapsed_time, syscall(SYS_gettid));
    }
    return NULL;
}

int main(int ac, char **av)
{
    int i;
    int nb;
    pthread_t t[MAX_THREADS];

    if (ac != 2)
    {
        printf("Usage: %s [nb_threads]\n", av[0]);
        exit(EXIT_FAILURE);
    }
    nb = atoi(av[1]);
    if (nb > MAX_THREADS)
    {
        printf("Error: MAX_THREADS=%i, given nb_threads=%i\n", MAX_THREADS, nb);
        exit(EXIT_FAILURE);
    }
    i = 0;
    while (++i <= nb)
        pthread_create(&t[i], NULL, &work, NULL);
    i = 0;
    while (++i <= nb)
        pthread_join(t[i], NULL);
    return 0;
}
