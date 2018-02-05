// ============================================================================
// File: prime.c
// Description: A simple prime number generator
//
// Odev 2
//
// Amac:
//    The goal of this assignment is to parallelize the prime number
//    generator using OpenMP and Pthreads.
//    (This project is adopted from a course project in CMU)
// ============================================================================


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

// ============================================================================
// Serial version of the prime number generator
// ============================================================================

void Primes(unsigned N);

// ============================================================================
// Parallel version of the prime number generator
// ============================================================================

void ParallelPrimes(unsigned N, unsigned P);

// ============================================================================
// Function of parallel threads --added by me
// ============================================================================
void *threadfunc(void* arrayNumbers);

// ============================================================================
// Timer: returns time in seconds
// ============================================================================

double gettime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// ============================================================================
// Global variables
// ============================================================================

// Number of primes found
unsigned count;
// Last prime number found
unsigned lastPrime;
// Array of flags. flag[i] denotes if 2*i+3 is prime or not
char *flags;

// ============================================================================
// Usage function
// ============================================================================

void Usage(char *program) {
  printf("Usage: %s [options]\n", program);
  printf("-n <num>\tSize of input. Maximum number to test\n");
  printf("-p <num>\tNumber of processors to use\n");
  printf("-o <file>\tOutput file name\n");
  printf("-d\t\tDisplay output\n");
  printf("-h \t\tDisplay this help and exit\n");

  exit(0);
}


// ============================================================================
// Main function
// ============================================================================

int main(int argc, char **argv) {

  int optchar;
  unsigned N = 100, P = 1;
  char outputfile[100] = "";
  char displayoutput = 0;

  // Read the command line options and obtain the input size and number of
  // processors.
  while ((optchar = getopt(argc, argv, "n:p:o:dh")) != -1) {

    switch (optchar) {

      case 'n':
        N = atoi(optarg);
        break;

      case 'p':
        P = atoi(optarg);
        break;

      case 'o':
        strcpy(outputfile, optarg);
        break;

      case 'd':
        displayoutput = 1;
        break;

      case 'h':
        Usage(argv[0]);
        break;

       default:
        Usage(argv[0]);
    }
  }

  // Create the flag array
  flags = malloc(sizeof(char) * ((N - 1)/2));
  if (!flags) {
    printf("Not enough memory.\n");
    exit(1);
  }

  printf("Testing for primes till: %u\n", N);
  printf("Number of processors: %u\n", P);

  // Depending on the number of processors, call the appropriate prime generator
  // function.
  if (P == 1) {
    Primes(N);
  }
  else {
    ParallelPrimes(N, P);
  }

  if (N >= 2) {
    count ++;
  }

  printf("Number of primes found = %u\n", count);
  printf("Last prime found = %u\n", lastPrime);


  // If we need to display the output, then open the output file
  // Open the output file
  if (displayoutput) {
    FILE *output;
    unsigned i;

    if (strlen(outputfile) > 0) {
      output = fopen(outputfile, "w");
      if (output == NULL) {
        printf("Cannot open specified output file `%s'. Falling back to stdout\n",
            outputfile);
        output = stdout;
      }
    }
    else {
      output = stdout;
    }

    fprintf(output, "List of prime numbers:\n");
    fprintf(output, "2\n");
    for (i = 0; i < (N-1)/2; i ++) {
      if (flags[i])
        fprintf(output, "%u\n", i*2+3);
    }
  }

  free(flags);
  return 0;
}

// ============================================================================
// Implementation of the serial version of the prime number generator
// ============================================================================

void Primes(unsigned N) {
  int i;
  int prime;
  int div1, div2, rem;
  double startTime ,processTime;
  startTime = gettime();
    count = 0;
    lastPrime = 0;

    for (i = 0; i < (N-1)/2; ++i) {    /* For every odd number */
      prime = 2*i + 3;

      /* Keep searching for divisor until rem == 0 (i.e. non prime),
         or we've reached the sqrt of prime (when div1 > div2) */

      div1 = 1;
      do {
        div1 += 2;            /* Divide by 3, 5, 7, ... */
        div2 = prime / div1;  /* Find the dividend */
        rem = prime % div1;   /* Find remainder */
      } while (rem != 0 && div1 <= div2);

      if (rem != 0 || div1 == prime) {
        /* prime is really a prime */
        flags[i] = 1;
        count++;
        lastPrime = prime;
      } else {
        /* prime is not a prime */
        flags[i] = 0;
      }
    }
    processTime = gettime() - startTime ; //get total process time
    printf("process time = % f ms\n",processTime);
}


// ============================================================================
// Parallel version of the prime number generator. You must use openmp to
// parallelize this function.
// ============================================================================

void ParallelPrimes(unsigned N, unsigned P) {
    int thread,i;
    double startTime ,processTime;
    unsigned * numbers; 
     
    pthread_t * threads = malloc(P*sizeof(pthread_t)); //create thread array
    startTime = gettime();
    
    count = 0;
    lastPrime = 0;
    
    for(thread= 0 ; thread<P ;thread++){
    	numbers = malloc(3*sizeof(unsigned));
          numbers[0]= N; //boundry
    	numbers[1]= P; //total thread count
    	numbers[2]= (unsigned) thread; //the number of the thread
    	
    	pthread_create(&threads[thread],NULL,threadfunc,(void*) numbers); //create threads
    	
    }
    for(thread= 0 ; thread<P ;thread++){
          pthread_join(threads[thread],NULL); //wait for end of threads
    }
    
    free(threads);
    
    for(i = 0;i<(N-1)/2;i++){ // count the "1" values in flags array to get the total number of primes     
    	if(flags[i]==1){
    		count++;
          }
    }
    processTime = gettime() - startTime ; //get total process time
    printf("process time = % f ms\n",processTime);
    
  
}
void *threadfunc(void *arrayNumbers){
    int i;
    unsigned prime;
    int div1, div2, rem;  
    unsigned thread,totalthread,N;
    unsigned* numbers;
    
    numbers = (unsigned*) arrayNumbers;
    N = numbers[0];
    totalthread = numbers[1];
    thread = numbers[2];

    for (i = thread; i < (N-1)/2; i +=totalthread) {   
     
      prime = 2*i + 3;	/* EACH THREAD GETS VALUE CONSIDERING BY ITS NUMBER IT MAKES PROCESS MORE BALANCED */
      div1 = 1;
      do {
        div1 += 2;            /* Divide by 3, 5, 7, ... */
        div2 = prime / div1;  /* Find the dividend */
        rem = prime % div1;   /* Find remainder */
      } while (rem != 0 && div1 <= div2);

      if (rem != 0 || div1 == prime) {
        /* prime is really a prime */
        flags[i] = 1;
        
        //count++;    I decided to not use mutex because it cause more time. so after process im calculating flags .
        
        if(lastPrime<prime){
        	lastPrime = prime; // MUTEX expensiver than this process so i decided to not use mutex here as well.
        }
        
      } else {
        /* prime is not a prime */
        flags[i] = 0;
      }
    }
    free(arrayNumbers);
    return 0;	
}
