/*
 *   Sieve of Eratosthenes
 *
 *   Programmed by Stephen J Plautz
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int main(int argc, char *argv[])
{
    long count = 0;          /* Local prime count */
    double elapsed_time = 0; /* Parallel execution time */
    long first = 0;          /* Index of first multiple */
    long global_count = 0;   /* Global prime count */
    long high_value = 0;     /* Highest value on this proc */
    long i = 0;              /* Counter value */
    int id = 0;          /* Process ID number */
    long index = 0;      /* Index of current prime */
    long low_value = 0;  /* Lowest value on this proc */
    char *marked;    /* Portion of 2,...,'n' */
    long n = 0;          /* Sieving from 2, ..., 'n' */
    int p = 0;          /* Number of processes */
    long proc0_size = 0; /* Size of proc 0's subarray */
    long prime = 0;      /* Current prime */
    long size = 0;       /* Elements in 'marked' */

    MPI_Init(&argc, &argv);
    void print_array(char*, long);

    /* Start the timer */
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();

    //error handling to avoid case where user does not specify a number range on command line program call
    if (argc != 2)
    {
        if (!id)
            printf("Command line: %s <m>\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }

    n = atol(argv[1]);

    /* Figure out this process's share of the array, as
      well as the integers represented by the first and
      last array elements */
    low_value = 2 + id * (n - 1) / p;
    high_value = 1 + (id + 1) * (n - 1) / p;
    size = high_value - low_value + 1;

    /* Bail out if all the primes used for sieving are
      not all held by process 0 */
    proc0_size = (n - 1) / p;
    if ((2 + proc0_size) < (long)sqrt((long double)n))
    {
        if (!id)
            printf("Too many processes\n");
        MPI_Finalize();
        exit(1);
    }

    /* Allocate this process's share of the array. */
    marked = (char *)malloc(size);
    if (marked == NULL)
    {
        printf("Cannot allocate enough memory\n");
        MPI_Finalize();
        exit(1);
    }

    //initializing the array to hold marked status of integer values in range of process
    for (i = 0; i < size; i++)
        marked[i] = 0;

    //if process id == 0 index gets 0
    if (!id)
        index = 0;
    
    //start off with an initial prime value of 2
    //this step can be optimized to eliminate all multiples of 2
    prime = 2;

    //main loop for seiving primes used by all processes
    do
    {
        //if current prime squared is in the range of the process's subarray, determine index of first multiple of prime in subarray
        if (prime * prime > low_value)
            first = prime * prime - low_value;

        //else the value of the current prime squared is not in the range of the process's subarray, so determine correct index
        else
        {
            //if lowest value of subarray evenly divides current prime, first index is 0th index of process's subarray
            if (!(low_value % prime))
                first = 0;
            //else first index used is prime minus remainder index of process's subarray
            else
                first = prime - (low_value % prime);
        }

        //marking all multiples of the current prime within range assigned to this process
        for (i = first; i < size; i += prime)
            marked[i] = 1;

        //process 0 gets the next prime to sieve
        if (!id)
        {
            while (marked[++index]);
            prime = index + 2;
        }

        //process 0 broadcast next prime to the comm channel
        if (p > 1)
            MPI_Bcast(&prime, 1, MPI_INT, 0, MPI_COMM_WORLD);

    } while (prime * prime <= n);
    
    //counting the total number of primes determined within process range
    count = 0;
    
    for (i = 0; i < size; i++){
        if (!marked[i])
        {
            count++;
        }
    }

    //if being done in parallel, reduce the total of all processes prime counts into one global prime count
    if (p > 1){
        MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    }
        
    /* Stop the timer */
    elapsed_time += MPI_Wtime();


    /* Print the results */
    if (!id)
    {
        printf("There are %lu primes less than or equal to %lu\n",
               global_count, n);
        printf("SIEVE (%d) %10.6f\n", p, elapsed_time);
    }

    MPI_Finalize();
    return 0;
}

void print_array(char* array, long size){
    for(int i = 0; i < size; i++){
        if(!array[i])
            printf("[%d] ", i+2);
    }
    printf("\n");
}
