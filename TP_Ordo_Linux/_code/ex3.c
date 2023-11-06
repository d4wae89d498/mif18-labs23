#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>
#include <stdlib.h>
#include <signal.h>

/*MIF18 : Lab 5 Linux Scheduling, ex3*/
/* Grégoire Pichon, 2022 */

#define MS_DIFF(END, START) ((END.tv_sec - START.tv_sec) * 1000000\
	+ (END.tv_nsec - START.tv_nsec) / 1000)
#define DEBUG 1

// une bonne grosse variable globale pour être acsessible au handler du signal
FILE* mylogfile;

void ferme_proprement(int sig)
{
    fclose(mylogfile);
    printf("closed with success\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv)
{
    unsigned long       waiting, elapsed;
    struct timespec     tv[2], begin;
    int                 cur_pos;
    struct sigaction    sa;
    int                 res;

    cur_pos = 0;
    if (argc == 1)
        mylogfile = fopen("ex3_logfile.csv","w");
    else
        mylogfile = fopen(argv[1],"w");
    if (mylogfile == NULL)
    {
        printf("Cannot open file\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("log file successfully opened\n");
        fprintf(mylogfile,"waiting,elapsed\n");
    }

    // Maintenant un petit gestionnaire du signal SIGINT
    // pour terminer proprement.
    // Ce signal sera envoyé via pkill afin de terminer le programme

    sigaction(SIGINT, NULL, &sa);
    sa.sa_handler = ferme_proprement;
    res = sigaction(SIGINT, &sa, NULL);
    if (res == -1)
    {
        printf("installation du gestionnaire pour SIGKILL error");
        exit(EXIT_FAILURE);
    }
    clock_gettime(CLOCK_MONOTONIC, &tv[1]);
    clock_gettime(CLOCK_MONOTONIC, &begin);
    while (1)
    {
        clock_gettime(CLOCK_MONOTONIC, &tv[cur_pos]);
        waiting = MS_DIFF(tv[cur_pos], tv[!cur_pos]);
        if (waiting > 500)
        {
            elapsed = MS_DIFF(tv[cur_pos], begin);
            if (DEBUG)
                printf("I did not run during %8lu microseconds, I was elapsed\
                       during %lu microseconds\n", waiting, elapsed);
            fprintf(mylogfile,"%lu,%lu\n", waiting, elapsed);
            begin = tv[cur_pos];
        }
        cur_pos = !cur_pos;
    }
    if (DEBUG)
        printf("numbers are stored in the ex3...csv file\n");
    return (0);
}
