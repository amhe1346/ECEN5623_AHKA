#include <math.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0
#define U32_T unsigned int

// Task arrays 
// U=0.7333
U32_T ex0_period[] = {2, 10, 15};
U32_T ex0_wcet[] = {1, 1, 2};

// U=0.9857
U32_T ex1_period[] = {2, 5, 7};
U32_T ex1_wcet[] = {1, 1, 2};

// U=0.9967
U32_T ex2_period[] = {2, 5, 7, 13};
U32_T ex2_wcet[] = {1, 1, 1, 2};

// U=0.93
U32_T ex3_period[] = {3, 5, 15};
U32_T ex3_wcet[] = {1, 2, 3};

// U=1.0
U32_T ex4_period[] = {2, 4, 16};
U32_T ex4_wcet[] = {1, 1, 4};

// U=1.0
U32_T ex5_period[] = {2,5,10};
U32_T ex5_wcet[] = {1,2,1};

// U=.996
U32_T ex6_period[] = {2,5,7,13};
U32_T ex6_wcet[] = {1,1,1,2};
U32_T ex6_deadline[] = {2,3,7,15};  // DM deadlines different from periods

// U=1.0
U32_T ex7_period[] = {3,5,15};
U32_T ex7_wcet[] = {1,2,4};

// U=.996
U32_T ex8_period[] = {2,5,7,13};
U32_T ex8_wcet[] = {1,1,1,2};

// harmonic  U=1
U32_T ex9_period[] = {6,8,12,24};
U32_T ex9_wcet[] = {1,2,4,6};

// creating a task set structure 
typedef struct {
    U32_T *period;
    U32_T *wcet; 
    U32_T numTasks;
    char *name;
    double utilization;
} TaskSet;

// Task set definitions (NOW AFTER the arrays are defined)
TaskSet examples[] = {
    {ex0_period, ex0_wcet, 3, "Ex-0", (1.0/2.0) + (1.0/10.0) + (2.0/15.0)},
    {ex1_period, ex1_wcet, 3, "Ex-1", (1.0/2.0) + (1.0/5.0) + (1.0/7.0)},
    {ex2_period, ex2_wcet, 4, "Ex-2", (1.0/2.0) + (1.0/5.0) + (1.0/7.0) + (2.0/13.0)},
    {ex3_period, ex3_wcet, 3, "Ex-3", (1.0/3.0) + (2.0/5.0) + (3.0/15.0)},
    {ex4_period, ex4_wcet, 3, "Ex-4", (1.0/2.0) + (1.0/4.0) + (4.0/16.0)},
    {ex5_period, ex5_wcet, 3, "Ex-5", (1.0/2.0) + (2.0/5.0) + (1.0/10.0)},
    {ex6_period, ex6_wcet, 4, "Ex-6", (1.0/2.0) + (1.0/5.0) + (1.0/7.0) + (2.0/13.0)},
    {ex7_period, ex7_wcet, 3, "Ex-7", (1.0/3.0) + (2.0/5.0) + (4.0/15.0)},
    {ex8_period, ex8_wcet, 4, "Ex-8", (1.0/2.0) + (1.0/5.0) + (1.0/7.0) + (2.0/13.0)},
    {ex9_period, ex9_wcet, 4, "Ex-9", (1.0/6.0) + (2.0/8.0) + (4.0/12.0) + (6.0/24.0)}
};

#define NUM_EXAMPLES (sizeof(examples)/sizeof(TaskSet))

typedef int (*FeasibilityTest)(U32_T, U32_T*, U32_T*, U32_T*);

//
int edf_util_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);
int llf_util_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);
int dm_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);


int completion_time_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);
int scheduling_point_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);


void run_all_tests(TaskSet *examples, int num_examples) {
    FeasibilityTest tests[] = {
        completion_time_feasibility,
        scheduling_point_feasibility,
        edf_util_feasibility,
        llf_util_feasibility
    };
    
    char *test_names[] = {
        "Completion Time",
        "Scheduling Point", 
        "EDF",
        "LLF"
    };
    
    int num_tests = sizeof(tests)/sizeof(FeasibilityTest);
    int t, i;
    
    for (t = 0; t < num_tests; t++) {
        printf("******** %s Feasibility\n", test_names[t]);
        for (i = 0; i < num_examples; i++) {
            // Skip Ex-6 for EDF/LLF per assignment
            if ((t >= 2) && (i == 6)) continue;
            
            printf("%s U=%.2f: ", examples[i].name, examples[i].utilization);
            if(tests[t](examples[i].numTasks,
                       examples[i].period, 
                       examples[i].wcet, 
                       examples[i].period) == TRUE)
                printf("FEASIBLE\n");
            else
                printf("INFEASIBLE\n");
        }
        printf("\n");
    }
}



//this function is response time analysis algorithim that tests if tasks emet deadline with 
// rate monotonic scheduling 
// calculated worse case response time for each task then check if it meets deadline 

int completion_time_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[])
{
  int i, j;
  U32_T an, anext;
  
  // assume feasible until we find otherwise
  int set_feasible=TRUE;
   
  //printf("numServices=%d\n", numServices);
  
  for (i=0; i < numServices; i++)
  {
       an=0; anext=0;


       // this is the response time = sum of execution times of tasks [i] and all the higher prio tasks
             // higher prio tasks are indec 0 to i-1 
       for (j=0; j <= i; j++)
       {
           an+=wcet[j]; // sum of execuition times 
       }
       
	   //printf("i=%d, an=%d\n", i, an);

       /// this while loop  nR_i = C_i + Σ(⌈R_i/T_j⌉ × C_j) for all j < i
       //ri is response time of task
       // ci is executoin itme
       // tj is period of higherpriotask j
       //ri/ti is number of timestask jprrempts task i  

       while(1)
       
       {
             anext=wcet[i];
             
	     
             for (j=0; j < i; j++)
                 anext += ceil(((double)an)/((double)period[j]))*wcet[j];
		 
             if (anext == an)
                break;
             else
                an=anext;
                

			 //printf("an=%d, anext=%d\n", an, anext);
       }
       
	   //printf("an=%d, deadline[%d]=%d\n", an, i, deadline[i]);
       // this if section is deadline check. 
    

       if (an > deadline[i])
       {
          set_feasible=FALSE;
       }
  }
  
  return set_feasible;
}


int scheduling_point_feasibility(U32_T numServices, U32_T period[], 
								 U32_T wcet[], U32_T deadline[])
{
   int rc = TRUE, i, j, k, l, status, temp;

   for (i=0; i < numServices; i++) // iterate from highest to lowest priority
   {
      status=0;

      for (k=0; k<=i; k++) 
      {
          for (l=1; l <= (floor((double)period[i]/(double)period[k])); l++)
          {
               temp=0;

               for (j=0; j<=i; j++) temp += wcet[j] * ceil((double)l*(double)period[k]/(double)period[j]);

               if (temp <= (l*period[k]))
			   {
				   status=1;
				   break;
			   }
           }
           if (status) break;
      }
      if (!status) rc=FALSE;
   }
   return rc;
}

//earliest deadline first 
//take task params return true or false

int edf_util_feasibility(U32_T numServices, U32_T period[],U32_T wcet[], U32_T deadline[])
{
    double total_utilization = 0.0;
    int i;
    //necessary and sufficient condition is U <= 1
    // calculate total cpu for all tasks i 
    // calculate each taks utilization 
    // edf is schedulableonly if U<= 1

    for (i = 0; i < numServices; i++)
    {
        total_utilization +=(double)wcet[i] /(double)period[i];
    }
    return(total_utilization <= 1.0) ? TRUE:FALSE;
}

int llf_util_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[])
{//when deadline = period , its the same as edf 
    //llf is run task with least laxity deadline - current time - remaiing execution 
    double total_utilization = 0.0;
    int i;
    
    // For LLF with deadline <= period, necessary and sufficient condition is U <= 1
    // LLF has same utilization bound as EDF for deadline = period case
    for (i = 0; i < numServices; i++) 
    {
        total_utilization += (double)wcet[i] / (double)period[i];
    }
    
    return (total_utilization <= 1.0) ? TRUE : FALSE;
}

int dm_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[])
{
    int i, j, k;
    U32_T an, anext;
    int set_feasible = TRUE;
    
    // Create arrays to store deadline-sorted task parameters
    U32_T sorted_period[10], sorted_wcet[10], sorted_deadline[10];
    int task_indices[10];
    
    // Initialize indices for original task order
    for (i = 0; i < numServices; i++) {
        task_indices[i] = i;
    }
    
    // Sort tasks by deadline (DM priority assignment)
    // Bubble sort based on deadlines - shortest deadline = highest priority
    for (i = 0; i < numServices - 1; i++) {
        for (j = 0; j < numServices - i - 1; j++) {
            if (deadline[task_indices[j]] > deadline[task_indices[j + 1]]) {
                // Swap indices
                int temp = task_indices[j];
                task_indices[j] = task_indices[j + 1];
                task_indices[j + 1] = temp;
            }
        }
    }
    
    // Create sorted arrays based on deadline priority order
    for (i = 0; i < numServices; i++) {
        sorted_period[i] = period[task_indices[i]];
        sorted_wcet[i] = wcet[task_indices[i]];
        sorted_deadline[i] = deadline[task_indices[i]];
    }
    
    // Apply response time analysis with DM priority ordering
    for (i = 0; i < numServices; i++) {
        an = 0; anext = 0;
        
        // Initial response time estimate
        for (j = 0; j <= i; j++) {
            an += sorted_wcet[j];
        }
        
        // Iterative response time calculation: R_i = C_i + Σ(⌈R_i/T_j⌉ × C_j) for all j < i
        while (1) {
            anext = sorted_wcet[i];
            
            // Add interference from higher priority tasks (j < i in DM order)
            for (j = 0; j < i; j++) {
                anext += ceil(((double)an)/((double)sorted_period[j])) * sorted_wcet[j];
            }
            
            if (anext == an)
                break;
            else
                an = anext;
        }
        
        // Check if response time meets deadline
        if (an > sorted_deadline[i]) {
            set_feasible = FALSE;
        }
    }
    
    return set_feasible;
}

int main(void)
{
    printf("Running Feasibility Tests for Examples 0-9\n");
    printf("==========================================\n\n");
    
    // Run all tests using the array-based approach
    run_all_tests(examples, NUM_EXAMPLES);
    
    // Special handling for Example 6: RM and DM comparison
    printf("******** Example 6: RM and DM Comparison\n");
    printf("RM Priority Order (by period): T1(2) > T2(5) > T3(7) > T4(13)\n");
    printf("DM Priority Order (by deadline): T1(2) > T2(3) > T3(7) > T4(15)\n\n");
    
    printf("Ex-6 RM U=%.2f: ", examples[6].utilization);
    if(completion_time_feasibility(examples[6].numTasks,
                                 examples[6].period, 
                                 examples[6].wcet, 
                                 examples[6].period) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");
        
    printf("Ex-6 DM U=%.2f: ", examples[6].utilization);
    if(dm_feasibility(examples[6].numTasks,
                     examples[6].period, 
                     examples[6].wcet, 
                     ex6_deadline) == TRUE)  // Use actual DM deadlines
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

    return 0;
}