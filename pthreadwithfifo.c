//cd '/mnt/c/Users/aehee/OneDrive - UCB-O365/Real Time Embedded Systems/RTES-Labs/lab1/incdecthread' && make clean && make && ./pthread
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>


#define COUNT  1000

typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[2];
threadParams_t threadParams[2];
// code for fifo scheduling set up 
pthread_attr_t rt_Sched_attr[2];
struct sched_param rt_param[2];
int rt_max_prio, rt_min_prio;  // this will be used to set priorities 






// Unsafe global
int gsum=0;

void *incThread(void *threadp)
{
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<COUNT; i++)
    {
        gsum=gsum+i;
        printf("Increment thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
    }
}


void *decThread(void *threadp)
{
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<COUNT; i++)
    {
        gsum=gsum-i;
        printf("Decrement thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
    }
}




int main (int argc, char *argv[])
{
   int rc;
   int i=0;

   // get max and min for fifo
   rt_max_prio = sched_get_priority_max(SCHED_FIFO);
   rt_min_prio = sched_get_priority_min(SCHED_FIFO);

   //thread attributes for increase - set it as higher prio
   pthread_attr_init(&rt_Sched_attr[i]);
   pthread_attr_setinheritsched(&rt_Sched_attr[i],PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&rt_Sched_attr[i],SCHED_FIFO);
   rt_param[i].sched_priority = rt_max_prio; 
   pthread_attr_setschedparam(&rt_Sched_attr[i],&rt_param[i]);

   threadParams[i].threadIdx=i;
   //this is a POSIX API pthread_Create with FIFO schedule
   pthread_create(&threads[i],   // pointer to thread descriptor
                  &rt_Sched_attr[i],  // use FIFO
                  incThread, // thread function entry point
                  (void *)&(threadParams[i]) // parameters to pass in
                 );
   i++;

   // thread attributes for the decreasing thread 
   pthread_attr_init(&rt_Sched_attr[i]);
   pthread_attr_setinheritsched(&rt_Sched_attr[i],PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&rt_Sched_attr[i],SCHED_FIFO);
   rt_param[i].sched_priority = rt_max_prio-1; //this is how the decrease thread gets lower prio
   pthread_attr_setschedparam(&rt_Sched_attr[i],&rt_param[i]);

   


   threadParams[i].threadIdx=i;
   pthread_create(&threads[i], &rt_Sched_attr[i], decThread, (void *)&(threadParams[i])); // replace the void with the sched type 

   for(i=0; i<2; i++)
   //this is a POSIX API pthread_join 
     pthread_join(threads[i], NULL);

   printf("TEST COMPLETE\n");
}
