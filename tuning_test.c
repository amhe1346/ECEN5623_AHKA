/****************************************************************************/
/* Tuning Test - Find correct iteration counts for 10ms and 20ms loads     */
/****************************************************************************/

#include <stdio.h>
#include <time.h>

#define FIB_LIMIT_FOR_32_BIT 47

void FIB_TEST(unsigned int seqCnt, unsigned int iterCnt)   
{
    unsigned int idx, jdx;
    unsigned int fib = 0, fib0 = 0, fib1 = 1; 

    for(idx = 0; idx < iterCnt; idx++)    
    {        
        jdx = 1;
        fib0 = 0;
        fib1 = 1;                          
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

void test_iterations(unsigned int iterations)
{
    struct timespec start, end;
    double elapsed;
    
    // Warm up
    FIB_TEST(FIB_LIMIT_FOR_32_BIT, 1000);
    
    // Actual test
    clock_gettime(CLOCK_MONOTONIC, &start);
    FIB_TEST(FIB_LIMIT_FOR_32_BIT, iterations);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    elapsed = get_elapsed_time(&start, &end);
    printf("Iterations: %10u -> Time: %8.3f ms\n", iterations, elapsed * 1000);
}

int main(int argc, char *argv[])
{
    printf("=== Fibonacci Iteration Tuning Test ===\n");
    printf("Goal: Find iterations for ~10ms and ~20ms execution\n\n");
    
    // Test various iteration counts
    unsigned int test_values[] = {
        10000,      // Original value
        50000,
        100000,
        200000,
        500000,
        1000000,
        1500000,
        2000000,
        2500000,
        3000000,
        3500000,
        4000000,
        5000000
    };
    
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    printf("Testing different iteration counts...\n\n");
    
    for(int i = 0; i < num_tests; i++)
    {
        test_iterations(test_values[i]);
    }
    
    printf("\n=== Instructions ===\n");
    printf("1. Find the iteration count closest to 10ms\n");
    printf("2. Use that value for fib10\n");
    printf("3. Use 2x that value for fib20 (should be ~20ms)\n");
    printf("\nExample:\n");
    printf("  If 2000000 iterations = 10.5ms\n");
    printf("  Then: fib10 = 2000000, fib20 = 4000000\n");
    
    return 0;
}
