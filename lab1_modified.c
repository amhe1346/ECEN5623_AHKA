/****************************************************************************/
/*                                                                          */
/* Sam Siewert - 2005     
                                                  */

//modified to run on linux  -

// to get system log   
//sudo grep "lab1_rms" /var/log/syslog | tail -50



/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sched.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>

#define FIB_LIMIT_FOR_32_BIT 47
#define NSEC_PER_MSEC 1000000
#define NSEC_PER_SEC 1000000000

sem_t semF10, semF20;
int abortTest = 0 ;
unsigned int fib10Cnt = 0 , fib20Cnt = 0 ;
FILE *log_file = NULL;
struct timespec prog_start_time;
int fibtentime = 38000;
int fibtwentytime = 78000;


//FIBONACCI 
void FIB_TEST(unsigned int seqCnt, unsigned int iterCnt)   
   {
    unsigned int idx, jdx;
    unsigned int fib = 0 , fib0 =0 , fib1 =1; 

   for(idx=0; idx < iterCnt; idx++)    
   {        
      jdx= 1;
      fib0 =0 ;
      fib1 =1;                          
      fib = fib0 + fib1;               
      while(jdx < seqCnt)              
      {                                
         fib0 = fib1;                  
         fib1 = fib;                   
         fib = fib0 + fib1;            
         jdx++;                        
      }                                
   }                                   
  }

double get_elapsed_time(struct timespec *start, struct timespec *end)
{
    return (end->tv_sec - start->tv_sec) + 
           (end->tv_nsec - start->tv_nsec) / 1e9;
}

/* Iterations, 2nd arg must be tuned for any given target type
   using windview
   
   170000 <= 10 msecs on Pentium at home
   
   Be very careful of WCET overloading CPU during first period of
   LCM.
   
 */
void* fib10(void* arg)
{
  struct timespec start, end;
  double elapsed;

   while(!abortTest)
   {
    sem_wait(&semF10);
    if(abortTest) break ; 

    clock_gettime(CLOCK_MONOTONIC,&start);
    double start_time = get_elapsed_time(&prog_start_time, &start) * 1000;
    printf("[START] fib10 #%d\n", fib10Cnt + 1);
    syslog(LOG_INFO, "[%.3f ms] fib10 #%d START", start_time, fib10Cnt + 1);
    if(log_file) fprintf(log_file, "%.3f, fib10, START, %d\n", start_time, fib10Cnt + 1);

	  
	   FIB_TEST(FIB_LIMIT_FOR_32_BIT, fibtentime);
     clock_gettime(CLOCK_MONOTONIC,&end);

	   fib10Cnt++;
     elapsed = get_elapsed_time(&start,&end);
     double end_time = get_elapsed_time(&prog_start_time, &end) * 1000;
     printf("fib10 #%d completed, elapsed: %.3f ms\n",fib10Cnt,elapsed *1000);
     syslog(LOG_INFO, "[%.3f ms] fib10 #%d COMPLETE (exec: %.3f ms)", end_time, fib10Cnt, elapsed * 1000);
     if(log_file) fprintf(log_file, "%.3f, fib10, COMPLETE, %d\n", end_time, fib10Cnt);
     //debug section tocheck if the elapsed time isgreater than 10 ms bc it will need to be changed on rpi 
     // on pi 
     if(elapsed > .010){
      printf("Elapsed > 10 ms!\n");
     }

   }
   return NULL;
}

void* fib20(void* arg)
{
    struct timespec start, end;
    double elapsed ; 
    
    while(!abortTest)
    {
        sem_wait(&semF20);
        if(abortTest) break; 
        
        clock_gettime(CLOCK_MONOTONIC, &start);
        double start_time = get_elapsed_time(&prog_start_time, &start) * 1000;
        printf("[START] fib20 #%d\n", fib20Cnt + 1);
        syslog(LOG_INFO, "[%.3f ms] fib20 #%d START", start_time, fib20Cnt + 1);
        if(log_file) fprintf(log_file, "%.3f, fib20, START, %d\n", start_time, fib20Cnt + 1);
        
        FIB_TEST(FIB_LIMIT_FOR_32_BIT, fibtwentytime);  
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        fib20Cnt++;
        elapsed = get_elapsed_time(&start,&end);
        double end_time = get_elapsed_time(&prog_start_time, &end) * 1000;

        printf("fib20 #%d completed, elapsed: %.3f ms\n",fib20Cnt,elapsed *1000);
        syslog(LOG_INFO, "[%.3f ms] fib20 #%d COMPLETE (exec: %.3f ms)", end_time, fib20Cnt, elapsed * 1000);
        if(log_file) fprintf(log_file, "%.3f, fib20, COMPLETE, %d\n", end_time, fib20Cnt);
        //debug section tocheck if the elapsed time isgreater than 10 ms bc it will need to be changed on rpi 
        if(elapsed > .020){
          printf("Elapsed > 20 ms!\n");
     }


    }
    return NULL;
}

int main(int argc, char *argv[])
{
  pthread_t thread_fib10, thread_fib20;
  pthread_attr_t attr_fib10, attr_fib20;
  struct sched_param param_fib10,param_fib20, main_param;
  struct timespec delay;
  int max_priority;
  struct timespec prog_start, current_time;
  double time_from_start; 


  

  printf( "Starting Rate Monotonic Sched Test \n");
  printf("S1 (fib10: C=10ms, T = 20ms \n)");
  // service one worstcase is 10 mseconds the deadline is every 20 ms 
  printf("S2 (fib20): C = 20 ms , T= 50 ms \n");
  // service 2 worstcase is 20 ms the deadline is every 50 ms 

  printf("LCM = 100 ms \n");
  // the lcm is 100 .
  
  // Open syslog
  openlog("lab1_rms", LOG_PID | LOG_CONS, LOG_USER);
  syslog(LOG_INFO, "=== Rate Monotonic Schedule Test Started ==="); 

  // main needs max priority 
  max_priority = sched_get_priority_max(SCHED_FIFO);
  main_param.sched_priority = max_priority;

  // Set main process scheduler
  if(sched_setscheduler(0, SCHED_FIFO, &main_param) != 0) {
    perror("Failed to set scheduler - run with sudo");
    return 1;
  }

  //Semaphores :)
  sem_init(&semF10,0,0);
  sem_init(&semF20,0,0);

  //max FIFO

 
  // configure thread for fib10  
  // set the priority to max-1 
  // set the scheddule to fifo 
  //
  pthread_attr_init(&attr_fib10);
  pthread_attr_setinheritsched(&attr_fib10, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&attr_fib10, SCHED_FIFO);
  param_fib10.sched_priority = max_priority - 1;
  pthread_attr_setschedparam(&attr_fib10, &param_fib10);
  
  // Configure fib20 thread to max -2  and sched FIFO 
  pthread_attr_init(&attr_fib20);
  pthread_attr_setinheritsched(&attr_fib20, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&attr_fib20, SCHED_FIFO);
  param_fib20.sched_priority = max_priority - 2;
  pthread_attr_setschedparam(&attr_fib20, &param_fib20);

  if(pthread_create(&thread_fib10, &attr_fib10,fib10,NULL)!=0)
  {
    printf("didnot make the fib 10 thread \n");
    return 1;
  }
  printf("fib10 was made \n");

    if(pthread_create(&thread_fib20, &attr_fib20,fib20,NULL)!=0)
  {
    printf("did not make the fib 20 thread \n");
    return 1;
  }

  printf("fib20 thread was made \n ");

  // Start program timer
  clock_gettime(CLOCK_MONOTONIC, &prog_start);

  // RELEASE AT SAME TIME 
  clock_gettime(CLOCK_MONOTONIC, &current_time);
  time_from_start = get_elapsed_time(&prog_start, &current_time) * 1000;
  printf("\n==RELEASE TIME (t=%.1f ms)=== \n", time_from_start);
  printf("[RELEASE] fib10 #1 and fib20 #1\n");
  sem_post(&semF10);
  sem_post(&semF20);

  int cycles = 1;
  for(int i=0 ; i< cycles; i++)
  {
    printf("\n ==LCM cycle %d== \n, ", i+1);

    // t= 20 ms release fib 10 
    // it has to be schedueld to rease at time intervals to check that the preemption 
    // to check that the realtimebehavior is correct 
    // i wanted tohave the semaphores passed back and forth to each other but 
    // that is wrong bcitneeds tonotbe coop multitasking but hard deadlines 

    // t=20ms: Release fib10
    delay.tv_sec = 0;
    delay.tv_nsec = 20 * NSEC_PER_MSEC;
    nanosleep(&delay, NULL);
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    time_from_start = get_elapsed_time(&prog_start, &current_time) * 1000;
    printf("[RELEASE] fib10 at t=%.1f ms\n", time_from_start);
    sem_post(&semF10);


    // t=40ms: Release fib10
   
    delay.tv_nsec = 20 * NSEC_PER_MSEC;
    nanosleep(&delay, NULL);
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    time_from_start = get_elapsed_time(&prog_start, &current_time) * 1000;
    printf("[RELEASE] fib10 at t=%.1f ms\n", time_from_start);
    sem_post(&semF10);

    delay.tv_nsec = 20 * NSEC_PER_MSEC;
    nanosleep(&delay, NULL);
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    time_from_start = get_elapsed_time(&prog_start, &current_time) * 1000;
    printf("[RELEASE] fib10 at t=%.1f ms\n", time_from_start);
    sem_post(&semF10);

    delay.tv_nsec = 20 * NSEC_PER_MSEC;
    nanosleep(&delay, NULL);
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    time_from_start = get_elapsed_time(&prog_start, &current_time) * 1000;
    printf("[RELEASE] fib10 at t=%.1f ms\n", time_from_start);
    sem_post(&semF10);

    delay.tv_nsec = 20 * NSEC_PER_MSEC;
    nanosleep(&delay, NULL);
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    time_from_start = get_elapsed_time(&prog_start, &current_time) * 1000;
    printf("[RELEASE] fib10 and fib20 at t=%.1f ms (Critical Instant)\n", time_from_start);
    sem_post(&semF10);  
    sem_post(&semF20);
    


  }

  // Wait for tasks to complete before cleanup
  printf("\nWaiting for all tasks to complete...\n");
  delay.tv_sec = 0;
  delay.tv_nsec = 50 * NSEC_PER_MSEC;
  nanosleep(&delay, NULL);

  printf("finish, clean up \n ");
    abortTest = 1;
    sem_post(&semF10);
    sem_post(&semF20);
    
    pthread_join(thread_fib10, NULL);
    pthread_join(thread_fib20, NULL);
    
    sem_destroy(&semF10);
    sem_destroy(&semF20);


    printf("\nTest completed:\n");
    printf("fib10 executions: %d\n", fib10Cnt);
    printf("fib20 executions: %d\n", fib20Cnt);
    
    syslog(LOG_INFO, "=== Test Completed: fib10=%d, fib20=%d ===", fib10Cnt, fib20Cnt);
    closelog();

    return 0;
}