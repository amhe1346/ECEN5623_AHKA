#include <bits/time.h>
#include <sched.h>
#include <stdio.h>
#include <syslog.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

struct sched_param idle_param, fib10_param, fib20_param;
pthread_t idle_thread, fib10_thread, fib20_thread;
pthread_attr_t idle_attr, fib10_attr, fib20_attr;
sem_t fib10_sem, fib20_sem;

//maximum value for n of the fibonacci sequence determined by https://stackoverflow.com/a/15065283
#define FIB_MAX_SIGNED_INT 47

//the following define statements are attributed to the provided Exercise 1 RT-Clock code
#define NSEC_PER_SEC 1000000000
#define NSEC_PER_MSEC 1000000
#define NSEC_PER_USEC 1000

//the following iterations needs to be obtained from manual testing
#define FIB10_ITERATIONS 14150
#define FIB20_ITERATIONS 27350

int fibonacci (int n)
{
    int result = 0;
    int last_num1 = 0;
    int last_num2 = 0;

    for (int i = 0; i < n; i++)
    {
        result = last_num1 + last_num2;
        last_num2 = last_num1;
        last_num1 = result;

        if (last_num1 == 0)
        {
            last_num1 = 1;
        }
    }

    return result;
}

//the following function is attributed to the provided Exercise 1 RT-Clock code
double d_ftime(struct timespec *fstart, struct timespec *fstop)
{
  double dfstart = ((double)(fstart->tv_sec) + ((double)(fstart->tv_nsec) / 1000000000.0));
  double dfstop = ((double)(fstop->tv_sec) + ((double)(fstop->tv_nsec) / 1000000000.0));

  return(dfstop - dfstart); 
}

void* fib10 (void* thread_id)
{
    static struct timespec rtclk_start_time = {0, 0};
    static struct timespec rtclk_stop_time = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &rtclk_start_time);
    sem_wait(&fib10_sem);

    //choose between predefined estimated number of iterations or actively timed number of iterations
#ifdef FIB10_ITERATIONS
    for (int j = 0; j < FIB10_ITERATIONS; j++)
    {
        fibonacci(FIB_MAX_SIGNED_INT);
    }

#else
    static struct timespec rtclk_curr_time = {0, 0};
    static struct timespec rtclk_goal_time = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &rtclk_curr_time);

    rtclk_goal_time.tv_nsec = rtclk_start_time.tv_nsec + (10 * NSEC_PER_MSEC);
    rtclk_goal_time.tv_sec = rtclk_start_time.tv_sec;

    while (d_ftime(&rtclk_curr_time, &rtclk_goal_time) > 0)
    {
        fibonacci(FIB_MAX_SIGNED_INT);
        syslog(LOG_KERN |LOG_CRIT, "fib10 current time elapsed: %ld seconds, %ld nanoseconds\r\n", rtclk_curr_time.tv_sec, rtclk_curr_time.tv_nsec); //may delete later
        clock_gettime(CLOCK_MONOTONIC, &rtclk_curr_time); //since every nanosecond counts, may introduce significant delay!
    }

#endif
    clock_gettime(CLOCK_MONOTONIC, &rtclk_stop_time);
    double elapsed = d_ftime(&rtclk_start_time, &rtclk_stop_time);
    printf("fib10 service completed, elapsed: %6.9lf seconds\r\n", elapsed);
}

void* fib20 (void* thread_id)
{
    static struct timespec rtclk_start_time = {0, 0};
    static struct timespec rtclk_stop_time = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &rtclk_start_time);
    sem_wait(&fib20_sem);

    //choose between predefined estimated number of iterations or actively timed number of iterations
#ifdef FIB20_ITERATIONS
    for (int j = 0; j < FIB20_ITERATIONS; j++)
    {
        fibonacci(FIB_MAX_SIGNED_INT);
    }

#else
    static struct timespec rtclk_curr_time = {0, 0};
    static struct timespec rtclk_goal_time = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &rtclk_curr_time);

    rtclk_goal_time.tv_nsec = rtclk_start_time.tv_nsec + (20 * NSEC_PER_MSEC);
    rtclk_goal_time.tv_sec = rtclk_start_time.tv_sec;

    while (d_ftime(&rtclk_curr_time, &rtclk_goal_time) > 0)
    {
        fibonacci(FIB_MAX_SIGNED_INT);
        syslog(LOG_KERN |LOG_CRIT, "fib20 current time elapsed: %ld seconds, %ld nanoseconds\r\n", rtclk_curr_time.tv_sec, rtclk_curr_time.tv_nsec); //may delete later
        clock_gettime(CLOCK_MONOTONIC, &rtclk_curr_time); //since every nanosecond counts, may introduce significant delay!
    }

#endif
    clock_gettime(CLOCK_MONOTONIC, &rtclk_stop_time);
    double elapsed = d_ftime(&rtclk_start_time, &rtclk_stop_time);
    printf("fib20 service completed, elapsed: %6.9lf seconds\r\n", elapsed);
}

int main (int argc, char *argv[])
{
    //step 1: initialization
    static struct timespec sleep_time_10 = {0, 10 * NSEC_PER_MSEC};
    static struct timespec remaining_time_10 = {0, 10 * NSEC_PER_MSEC};
    static struct timespec sleep_time_20 = {0, 20 * NSEC_PER_MSEC};
    static struct timespec remaining_time_20 = {0, 20 * NSEC_PER_MSEC};

    static struct timespec rtclk_total_start_time = {0, 0};
    static struct timespec rtclk_total_stop_time = {0, 0};

    printf("Beginning synthetic load generation with ScheduleFIFO scheduling policy...\r\n");

    sem_init(&fib10_sem, 0, 1);
	sem_init(&fib20_sem, 0, 1);

    idle_param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    fib10_param.sched_priority = idle_param.sched_priority - 1;
    fib20_param.sched_priority = fib10_param.sched_priority - 1;

    pthread_attr_init(&idle_attr);
    pthread_attr_setinheritsched(&idle_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&idle_attr, SCHED_FIFO);

    pthread_attr_init(&fib10_attr);
    pthread_attr_setinheritsched(&fib10_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&fib10_attr, SCHED_FIFO);

    pthread_attr_init(&fib20_attr);
    pthread_attr_setinheritsched(&fib20_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&fib20_attr, SCHED_FIFO);

    if (sched_setscheduler(getpid(), SCHED_FIFO, &idle_param) 
        || sched_setscheduler(getpid(), SCHED_FIFO, &fib10_param)
        || sched_setscheduler(getpid(), SCHED_FIFO, &fib20_param)) //getpid unavailable? use getpt instead
    {
        printf("Could not set scheduler on at least one process! Exiting program...\r\n");
        return 1;
    }

    pthread_attr_setschedparam(&idle_attr, &idle_param);
    pthread_attr_setschedparam(&fib10_attr, &fib10_param);
    pthread_attr_setschedparam(&fib20_attr, &fib20_param);

    //step 2: fire off threads
    clock_gettime(CLOCK_MONOTONIC, &rtclk_total_start_time);
    if (pthread_create(&fib10_thread, &fib10_attr, fib10, NULL)
        || pthread_create(&fib20_thread, &fib20_attr, fib20, NULL))
    {
        printf("At least one thread failed to be created! Exiting program...\r\n");
        return 1;
    }

    //step 3: fire off semaphores in respect of the schedule
    nanosleep(&sleep_time_20, &remaining_time_20);
    remaining_time_20.tv_nsec = 20 * NSEC_PER_MSEC;
    sem_post(&fib10_sem);
    nanosleep(&sleep_time_20, &remaining_time_20);
    remaining_time_20.tv_nsec = 20 * NSEC_PER_MSEC;
    sem_post(&fib10_sem);
    nanosleep(&sleep_time_10, &remaining_time_10);
    remaining_time_10.tv_nsec = 10 * NSEC_PER_MSEC;
    sem_post(&fib20_sem);
    nanosleep(&sleep_time_10, &remaining_time_10);
    remaining_time_10.tv_nsec = 10 * NSEC_PER_MSEC;
    sem_post(&fib10_sem);
    nanosleep(&sleep_time_20, &remaining_time_20);
    remaining_time_20.tv_nsec = 20 * NSEC_PER_MSEC;
    sem_post(&fib10_sem);
    nanosleep(&sleep_time_20, &remaining_time_20);
    remaining_time_20.tv_nsec = 20 * NSEC_PER_MSEC;

    //step 4: conclude test and clean up
    clock_gettime(CLOCK_MONOTONIC, &rtclk_total_stop_time);
    double elapsed = d_ftime(&rtclk_total_start_time, &rtclk_total_stop_time);
    printf("Program completed, elapsed: %6.9lf seconds\r\n", elapsed);

    pthread_join(fib10_thread, NULL);
    pthread_join(fib20_thread, NULL);
    sem_destroy(&fib10_sem);
    sem_destroy(&fib20_sem);

    return 0;
}
