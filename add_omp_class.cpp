#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <omp.h>


using namespace std::chrono;
using namespace std;

#define NUM_THREADS 4

//Random Thread method
void randomVector(int vector[], int size)
{
    for (int i = 0; i < size; i++)
    {
        // randomly populate
        vector[i] = rand() % 100;
    }
}


// scheduling
int calculateSum(int* vector, int size)
{
    int totalSum = 0;

    #pragma omp for schedule(dynamic ,12)
    // loop variable 'i' is private by default
    for(int i = 0; i < size; i++)
    totalSum += vector[i];
    #pragma omp barrier

    return totalSum;
}

// atomic update
int calculateSum1(int* vector, int size)
{
    int totalSum = 0;

    #pragma omp parallel for
    // loop variable 'i' is private by default
    for(int i = 0; i < size; i++)

    #pragma omp atomic update
    totalSum += vector[i];
    return totalSum;
}

// reduction
int calculateSum2(int* vector, int size)
{
    int totalSum = 0;
    // specifies that one or more variables that are private to each thread
    // are the subject of a reduction operation at the end of the parallel 
    // region
    #pragma omp parallel for reduction(+: totalSum)
    for(int i = 0; i < size; i++)
    totalSum += vector[i];
    return totalSum;
}

// critical
int calculateSum3(int* vector, int size)
{
    // shared variable
    int sum_shared = 0;
    // specifies that one or more variables that are private to each thread
    // are the subject of a reduction operation at the end of the parallel 
    // region
    #pragma omp parallel 
    {
    // private variable, one for each thread
    int sum_local = 0;

    #pragma omp for nowait
    for(int i = 0; i < size; i++)
    sum_local += vector[i];
    #pragma omp critical
    {
        sum_shared += sum_local;
    }
    }

    return sum_shared;
}


int main(){

    unsigned long size = 1000000000L;
    double t1,t2, durat; 
    int aa, ba;
    int n_per_thread;                   // elements per thread
	int total_threads = NUM_THREADS; 
    int i;       // loop index
    int s1,s2,totalSum, s3, s4, totalSum_;

    srand(time(0));

    int *v1, *v2, *v3;

    // store the start time in a variable
    auto start = high_resolution_clock::now();
    // alternative clock
    t1 = omp_get_wtime();

    //get memory in the heap for 3 vectors
    v1 = (int *) malloc(size * sizeof(int *));
    v2 = (int *) malloc(size * sizeof(int *));
    v3 = (int *) malloc(size * sizeof(int *));


    randomVector(v1, size);
    randomVector(v2, size);



    // Additional work to set the number of threads.
	// We hard-code to 4 for illustration purposes only.
	omp_set_num_threads(total_threads);
	
	// determine how many elements each process will work on
	n_per_thread = size/total_threads;

/* 
//0 

	#pragma omp parallel for default(none) private (v1,v2,v3) shared(size)
        for(i=0; i < size; i++) 
		    v3[i] = v1[i]+v2[i];

// 1
    // Compute the vector addition
	// Here is where the 4 threads are specifically 'forked' to
	// execute in parallel. This is directed by the pragma and
	// thread forking is compiled into the resulting exacutable.
	// Here we use a 'static schedule' so each thread works on  
	// a  chunk of the original 100000000-element array.
   
	#pragma omp parallel for shared(v1, v2, v3) private(i) schedule(static, n_per_thread)
        for(i=0; i < size; i++) 
		    v3[i] = v1[i]+v2[i];

//2
    #pragma omp parallel num_threads(total_threads)
        for(i=0; i < size; i++) 
		    v3[i] = v1[i]+v2[i];

//3 
    // 4 threads available, 1 will be used
    #pragma omp parallel
    #pragma omp single
        for(i=0; i < size; i++) 
		    v3[i] = v1[i]+v2[i];

//4 
    // A single region is similar to a parallel for loop in the sense that there is 
    // waiting after it (but not before). You can use nowait to disable waiting:
    #pragma omp parallel
    #pragma omp single nowait
        for(i=0; i < size; i++) 
		    v3[i] = v1[i]+v2[i];

//5 -> atomic
    #pragma omp parallel
    {
        s1 = calculateSum1(v1,size);
        s2 = calculateSum1(v2,size);
        totalSum = s1+s2;
    }

//6 -> reduction
    #pragma omp parallel
    {
        s1 = calculateSum2(v1,size);
        s2 = calculateSum2(v2,size);
        totalSum = s1+s2;
    }

//6 -> critical

        s3 = calculateSum3(v1,size);
        s4 = calculateSum3(v2,size);
        totalSum_ = s3+s4;
*/    

//7 -> schedule
    #pragma omp parallel
    {
        s1 = calculateSum(v1,size);
        s2 = calculateSum(v2,size);
        totalSum = s1+s2;
    }

    //store the time (stop time) in a variable
    auto stop = high_resolution_clock::now();
    // alternative time - stop
    t2 = omp_get_wtime();
        
    //calculates the time difference (duration of execution)
    auto duration = duration_cast<microseconds>(stop - start);
    // alternative clock
    durat = t2-t1;
        
    cout << "\n *** Time taken by function: "
         << duration.count() << " microseconds" << endl;
    cout << "\n         *** omp_get_wtime: "
         << durat << " seconds\n" << endl;
    cout << "\n  sum: "
         << s1 << " + " << s2 << " = "<< totalSum << endl;

    // for critical
    //cout << "\n  sum: "
    //     << s3 << " + " << s4 << " = "<< totalSum_ << endl;
    return 0;
}