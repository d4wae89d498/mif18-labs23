#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>

/*MIF18 : Lab 5 Linux Scheduling, ex2*/
/* Grégoire Pichon, 2022 */

#define NB_CALLS 100000
#define NB_LOOPS 10
#define US_DIFF(END, START) ((END.tv_sec - START.tv_sec) * 1000000\
	+ (END.tv_nsec - START.tv_nsec) / 1000)

int main(void)
{
    unsigned long   elapsed_time;
    struct timespec start, end;
    unsigned        i;
    unsigned        y;
    unsigned long   total;

    total = 0;
    i = 0;
    while (++i <= NB_LOOPS)
    {
        clock_gettime(CLOCK_MONOTONIC, &start);
        y = 0;
        while (++y <= NB_CALLS)
            sched_yield();
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed_time = US_DIFF(end, start);
        printf("Calling %i sched_yield() took %lu microseconds\n", NB_CALLS,
               elapsed_time);
        // warning: total may overflow. Make sure NB_LOOPS is not too large.
        // Also, using floating point may impact precision.
        total += elapsed_time;
    }
    printf("average=%lu\n", total / NB_LOOPS);
    return (0);
}
