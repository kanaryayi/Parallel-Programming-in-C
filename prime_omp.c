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
#include <omp.h>

// ============================================================================
// Serial version of the prime number generator
// ============================================================================

void Primes(unsigned N);

// ============================================================================
// Parallel version of the prime number generator
// ============================================================================

void ParallelPrimes(unsigned N, unsigned P);

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
  double startime,processtime;
  int i;
  int prime;
  int div1, div2, rem;
  startime = gettime();
    count = 0;
    lastPrime = 0;

    for (i = 0; i < (N-1)/2; ++i) {    /* For every odd number */
      prime = 2*i + 3;

      /* Keep searching for divisor until rem == 0 (i.e. non prime),
         or we've reached the sqrt of prime (when#pragma omp parallel for num_threads(P) div1 > div2) */

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
    processtime = gettime() - startime;
    printf("%f\n", processtime);
}


// ============================================================================
// Parallel version of the prime number generator. You must use openmp to
// parallelize this function.
// ============================================================================

void ParallelPrimes(unsigned N, unsigned P) {
  double startime,processtime;
  int i=0;
  int prime;
  int div1, div2, rem;
  startime = gettime();

    count = 0;
    lastPrime = 0;

    #pragma omp parallel for num_threads(P) shared(i,count,lastPrime) private(div1, div2, rem,prime) schedule(dynamic,N/(200*P))
 		         		    				    	//creation of paralel threads
    for (i = 0; i < (N-1)/2; ++i) {    /* For every odd number */			// comparing with Pthread file i didnt
      prime = 2*i + 3;							// change the code a lot. Because of the  
      /* Keep searching for divisor until rem == 0 (i.e. non prime			some variables that i, count ,lastprime
         or we've reached the sqrt of prime (when div1 > div2) */			//these variables are in use by every Thread
         	        	      	        	    	       	   			//so i defined them as Shared values.
      div1 = 1;
      do {
        div1 += 2;            /* Divide by 3, 5, 7, ... */
        div2 = prime / div1;  /* Find the dividend */
        rem = prime % div1;   /* Find remainder */
      } while (rem != 0 && div1 <= div2);

      if (rem != 0 || div1 == prime) {
        /* prime is really a prime */
        flags[i] = 1;
    #   pragma omp atomic // here is a shared variable so that i made this omp atomic
        count++;

        if(prime>lastPrime){ // it s more cheap to use this "if" than omp critical 
          lastPrime = prime;
        }
      } else {
        /* prime is not a prime */
        flags[i] = 0;
      }
    }
    processtime = gettime() - startime;
    printf("%f %d\n", processtime,i);
}
