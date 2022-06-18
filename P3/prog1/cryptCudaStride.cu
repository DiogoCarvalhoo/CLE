/**
 *   Tomás Oliveira e Silva, November 2017
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>
#include <unistd.h> 

#include "common.h"
#include <cuda_runtime.h>

/* allusion to internal functions */

static void calculate_determinant_cpu_kernel (double * matrix_pointer, double * determinant,
                                              unsigned int order_of_matrix);
__global__ static void calculate_determinant_cuda_kernel (double * __restrict__ mat, double * __restrict__ determinants,
                                                          unsigned int n_sectors, unsigned int sector_size);
static double get_delta_time(void);

/** \brief function responsible to present the program usage */
static void printUsage(char *cmdName);

/**
 *   main program
 */

int main (int argc, char **argv)
{
  printf("%s Starting2...\n", argv[0]);
  if (sizeof (unsigned int) != (size_t) 4)
     return 1;                                             // it fails with prejudice if an integer does not have 4 bytes


  /* process command line arguments */

  int opt;            /* selected option */
  char *fName;   /* file name (initialized to "no name" by default) */

  opterr = 0;
  do
  {
      switch ((opt = getopt(argc, argv, "f:h")))
      {
      case 'f': /* file name */
          if (optarg[0] == '-')
          {
              fprintf(stderr, "%s: file name is missing\n", basename(argv[0]));
              printUsage(basename(argv[0]));

              return EXIT_FAILURE;
          }
          fName = optarg;
          break;
      case 'h': /* help mode */
          printUsage(basename(argv[0]));

          return EXIT_SUCCESS;
      case '?': /* invalid option */
          fprintf(stderr, "%s: invalid option\n", basename(argv[0]));
          printUsage(basename(argv[0]));

          return EXIT_FAILURE;
      case -1:
          
          break;
      }
  } while (opt != -1);
  if (argc == 1) 
  {
    fprintf(stderr, "%s: invalid format\n", basename(argv[0]));
    return EXIT_FAILURE;
  }

  /* open text file */

  FILE * fpointer;
  fpointer = fopen(fName, "rb");

  if (fpointer == NULL) {
      fprintf(stderr, "It occoured an error while openning file. \n");
      exit(EXIT_FAILURE);
  }

  /* read file content */

  int number_of_matrix;
  int order_of_matrix;
  
  // Read Number of Matrix from File
  if(fread(&number_of_matrix, sizeof(int), 1, fpointer) != 1)
      strerror(1);
  printf("Number of matrices to be read = %i \n", number_of_matrix);   

  // Read Order of Matrix from File
  if(fread(&order_of_matrix, sizeof(int), 1, fpointer) != 1)
      strerror(1);
  printf("Matrices order = %i \n", order_of_matrix);

  /* set up the device */

  int dev = 0;

  cudaDeviceProp deviceProp;
  CHECK (cudaGetDeviceProperties (&deviceProp, dev));
  printf("Using Device %d: %s\n", dev, deviceProp.name);
  CHECK (cudaSetDevice (dev));

  /* create memory areas in host and device memory where the disk sectors data and sector numbers will be stored */

  int mat_size = order_of_matrix * order_of_matrix * sizeof(double);
  size_t mat_area_size = number_of_matrix * mat_size;
  double * host_mat, * host_determinants;
  double * device_mat, * device_determinants;

  if ((mat_area_size + number_of_matrix * sizeof(double)) > (size_t) 1.3e9)
     { fprintf (stderr,"The GeForce GTX 1660 Ti cannot handle more than 5GB of memory!\n");
       exit (1);
     }
  printf ("Total mat size: %d\n", (int) mat_area_size);
  
  host_mat = (double *) malloc (mat_area_size);
  host_determinants = (double *) malloc (number_of_matrix*sizeof(double));
  CHECK (cudaMalloc ((void **) &device_mat, mat_area_size));
  CHECK (cudaMalloc ((void **) &device_determinants, number_of_matrix*sizeof(double)));

  /* initialize the host data */

  (void) get_delta_time ();

  if(fread(host_mat, mat_area_size, 1, fpointer) != 1)
      strerror(1);
  
  for (int i = 0; i<number_of_matrix; i++) {
    host_determinants[i] = 1;
  }

  printf ("The initialization of host data took %.3e seconds\n",get_delta_time ());

  /* copy the host data to the device memory */

  (void) get_delta_time ();
  CHECK (cudaMemcpy (device_mat, host_mat, mat_area_size, cudaMemcpyHostToDevice));
  CHECK (cudaMemcpy (device_determinants, host_determinants, number_of_matrix * sizeof(double), cudaMemcpyHostToDevice));
  printf ("The transfer of %d bytes from the host to the device took %.3e seconds\n",
          (int) mat_area_size , get_delta_time ());

  /* run the computational kernel */

  unsigned int gridDimX,gridDimY,gridDimZ,blockDimX,blockDimY,blockDimZ;
  int n_sectors, sector_size;

  n_sectors = number_of_matrix * order_of_matrix;
  sector_size = order_of_matrix;
  blockDimX = order_of_matrix;
  blockDimY = 1 << 0;                                             // optimize!
  blockDimZ = 1 << 0;                                             // do not change!
  gridDimX = number_of_matrix;
  gridDimY = 1 << 0;                                              // optimize!
  gridDimZ = 1 << 0;                                              // do not change!

  dim3 grid (gridDimX, gridDimY, gridDimZ);
  dim3 block (blockDimX, blockDimY, blockDimZ);

  if ((gridDimX * gridDimY * gridDimZ * blockDimX * blockDimY * blockDimZ) != n_sectors)
     { printf ("Wrong configuration!\n");
       return 1;
     }
  (void) get_delta_time ();
  calculate_determinant_cuda_kernel <<<grid, block, sector_size-1>>> (device_mat, device_determinants, n_sectors, sector_size);
  CHECK (cudaDeviceSynchronize ());                            // wait for kernel to finish
  CHECK (cudaGetLastError ());                                 // check for kernel errors
  printf("The CUDA kernel <<<(%d,%d,%d), (%d,%d,%d)>>> took %.3e seconds to run\n",
         gridDimX, gridDimY, gridDimZ, blockDimX, blockDimY, blockDimZ, get_delta_time ());

  /* copy kernel result back to host side */

  double *modified_mat;
  double *determinants;

  //modified_device_sector_data = (unsigned int *) malloc (sector_data_size);
  modified_mat = (double *) malloc (mat_area_size);
  determinants = (double *) malloc (number_of_matrix*sizeof(double));
  CHECK (cudaMemcpy (modified_mat, device_mat, mat_area_size, cudaMemcpyDeviceToHost));
  CHECK (cudaMemcpy (determinants, device_determinants, number_of_matrix*sizeof(double), cudaMemcpyDeviceToHost));
  printf ("The transfer of %d bytes from the device to the host took %.3e seconds\n",
          (int) mat_area_size, get_delta_time ());

  for (int i = 0; i < number_of_matrix; i++) {
    printf("Processing matrix %d \n", i + 1);
    
    /*
    for (int k = 0; k < order_of_matrix; k++) {
      for (int l = 0; l < order_of_matrix; l++) {
        printf("%6.2f ", *(modified_mat + (k * order_of_matrix) + l + (i * order_of_matrix * order_of_matrix) ) );
      }
      printf("\n");
    }
    */
    
    printf("Determinant: %.3e \n\n", determinants[i]);
  }

  /* free device global memory */

  CHECK (cudaFree (device_mat));
  //CHECK (cudaFree (device_sector_number));

  /* reset the device */

  CHECK (cudaDeviceReset ());

  /* compute the determinants on the CPU */

  (void) get_delta_time ();
  double *cpu_determinants;
  cpu_determinants = (double *) malloc (number_of_matrix*sizeof(double));
  
  for (int i = 0; i < number_of_matrix; i++) {
    cpu_determinants[i] = 1;
    calculate_determinant_cpu_kernel (host_mat + (i*order_of_matrix*order_of_matrix), &cpu_determinants[i], order_of_matrix);
  }
  printf("The cpu kernel took %.3e seconds to run (single core)\n",get_delta_time ());

  /* compare results */
  
  for(int i = 0; i < number_of_matrix; i++) {
    if (fabs(determinants[i] - cpu_determinants[i]) > 0.0001f)
       { 
        printf ("Mismatch in matrix %d. GPU calculated %.4e CPU calculated %.4e \n", i, determinants[i], cpu_determinants[i]);
        exit(1);
       }
  }
  printf ("All is well!\n");

  /* free host memory */

  free (host_mat);
  free (modified_mat);

  return 0;
}

static void calculate_determinant_cpu_kernel (double * matrix_pointer, double * determinant,
                                              unsigned int order_of_matrix)
{
  // Define matrix coeficients structure (2D Array)
  double matrix_coeficients[order_of_matrix][order_of_matrix];

  // Read Matrix Coefficients from buffer and save it to 2D array
  for (int l = 0; l < order_of_matrix; l++) {
      int line_offset = l * order_of_matrix;
      for (int column = 0; column < order_of_matrix; column++) {
          double coefficient = *(matrix_pointer + column + line_offset);
          matrix_coeficients[l][column] = coefficient;
      }
  }

  for (int l = 0; l < order_of_matrix-1; l++) {
      if (matrix_coeficients[l][l] != 0.0) {
          
          for(int k=l+1; k<order_of_matrix; k++){
              double  term=matrix_coeficients[k][l]/ matrix_coeficients[l][l];
              for(int j=0; j< order_of_matrix; j++){
                  matrix_coeficients[k][j]=matrix_coeficients[k][j]-term*matrix_coeficients[l][j];
              }
          }

      } else {
          // Find column != 0.0 to be changed
          int columnToChange = -1;
          for (int j=l+1; j<order_of_matrix; j++) {
              if (matrix_coeficients[l][j] != 0.0) {
                  columnToChange = j;
              }
          }

          if (columnToChange == -1) {
              *determinant = 0;
              break;
          } else {
              int temp;
              for (int i = 0; i < order_of_matrix; ++i) {
                  temp = matrix_coeficients[i][l - 1];
                  matrix_coeficients[i][l - 1] = matrix_coeficients[i][columnToChange - 1];
                  matrix_coeficients[i][columnToChange - 1] = temp;
              }
          }

      }

  }

  // Calculate Determinant from upper triangular matrix (Multiply Diagonal Values)
  for(int l = 0; l < order_of_matrix; l++){
      *determinant = *determinant*matrix_coeficients[l][l];
  }
}

__global__ static void calculate_determinant_cuda_kernel (double * __restrict__ mat, double * __restrict__ determinants,
                                                          unsigned int n_sectors, unsigned int sector_size)
{
  /* compute the thread number */

  int bkx = blockIdx.x + gridDim.x * blockIdx.y + gridDim.x * gridDim.y * blockIdx.z;         // block identifier
  int idx = threadIdx.x + blockDim.x * threadIdx.y + blockDim.x * blockDim.y * threadIdx.z;   // thread identifier

  if (idx >= n_sectors)
     return;                                             // safety precaution

  /* array to store the terms to update the coefficients in each iteration */
  extern __shared__ double term[];

  /* adjust pointers */

  mat += bkx * sector_size * sector_size;
  mat += idx;

  /* start the iteration cycle */

  for (int i = 0; i<sector_size-1; i++) {

    // If it is the first column of the iteration
    if (i == idx) {
      // Calculate the terms for each line and save them in the shared array
      for (int k = i+1; k < sector_size; k++) {
        term[k-1] = *(mat + (k*sector_size)) / *(mat + (i * sector_size));
      }
    }
    
    __syncthreads(); // Synchronizing all threads to get the current terms

    // Update the values of all the coefficients in the column
    for (int k = i+1; k<sector_size; k++) {
      *(mat + (k * sector_size)) = *(mat + (k * sector_size)) - term[k-1] * (*(mat + (i * sector_size)));
    }
    
  }

  // Thread 0 is responsible to calculate the determinant of this matrix
  if (idx == 0) {
    for(int l = 0; l < sector_size; l++){
        double coef = mat[ (l*sector_size) + l ];
        determinants[bkx] = determinants[bkx] * coef;
    }
  }

}

static double get_delta_time(void)
{
  static struct timespec t0,t1;

  t0 = t1;
  if(clock_gettime(CLOCK_MONOTONIC,&t1) != 0)
  {
    perror("clock_gettime");
    exit(1);
  }
  return (double)(t1.tv_sec - t0.tv_sec) + 1.0e-9 * (double)(t1.tv_nsec - t0.tv_nsec);
}

/**
 *  \brief print usage.
 */
static void printUsage(char *cmdName)
{
    fprintf(stderr, "\nSynopsis: %s OPTIONS [filename]\n"
                    "  OPTIONS:\n"
                    "  -h      --- print this help\n"
                    "  -f      --- filename\n",
            cmdName);
}