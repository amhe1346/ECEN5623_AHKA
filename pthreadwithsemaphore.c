//cd '/mnt/c/Users/aehee/OneDrive - UCB-O365/Real Time Embedded Systems/RTES-Labs/lab1/incdecthread' && make clean && make && ./pthread
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <semaphore.h>

#define COUNT  1000

typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[2];
threadParams_t threadParams[2];

//semaphores
sem_t semIncrease;  // for the increasing thread 
sem_t semDecrease;   // for thedecreasing thread 


// Unsafe global
int gsum=0;

void *incThread(void *threadp)
{
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<COUNT; i++)
    {
        sem_wait(&semIncrease); // this is the sempahore command to wait 
        gsum=gsum+i;
        printf("Increment thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
        sem_post(&semDecrease); // this is the semaphore signal for the decrease  thread to start 
    }
}


void *decThread(void *threadp)
{
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<COUNT; i++)
    {
        sem_wait(&semDecrease);  // signal for decrease thread to wait 
        gsum=gsum-i;
        printf("Decrement thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
        sem_post(&semIncrease);  // signal for increase thread to run 
    }
}




int main (int argc, char *argv[])
{
   int rc;
   int i=0;

   //semaphore 
   sem_init(&semIncrease, 0,1); //start with 1 
   sem_init(&semDecrease,0,0); // start with 0 so decrease waits

   threadParams[i].threadIdx=i;
   //this is a POSIX API pthread_Create
   pthread_create(&threads[i],   // pointer to thread descriptor
                  (void *)0,     // use default attributes
                  incThread, // thread function entry point
                  (void *)&(threadParams[i]) // parameters to pass in
                 );
   i++;

   threadParams[i].threadIdx=i;
   pthread_create(&threads[i], (void *)0, decThread, (void *)&(threadParams[i]));

   for(i=0; i<2; i++)
   //this is a POSIX API pthread_join 
     pthread_join(threads[i], NULL);

    //remove semaphores 
    sem_destroy(&semIncrease);
    sem_destroy(&semDecrease);

   printf("TEST COMPLETE\n");
}
