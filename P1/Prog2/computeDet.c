/**
 *  \file computeDet.c (implementation file)
 *
 *  \brief Problem name: Compute Matrix Determinant.
 *
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Generator thread of the intervening entities.
 *
 *  \author Diogo Filipe Amaral Carvalho - 92969 - April 2022
 *  \author Rafael Ferreira Baptista - 93367 - April 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <libgen.h>
#include <unistd.h>
#include <pthread.h>

#include "chunks.h"

/** \brief function responsible to present the program usage */
static void printUsage(char *cmdName);

/** \brief worker life cycle routine */
static void *worker(void *par);

/** \brief worker threads return status array */
int *statusWorkers;

/** \brief main thread return status */
int statusProd;

/** \brief to store the determinant of each matrix */
double * matrixDeterminants;

/** \brief function to compute determinant of a matrix */
static void computeDeterminant(struct MatrixInfo * matrixinfo, double * determinant);

int main(int argc, char *argv[])
{   
    /* process command line arguments */

    int opt;            /* selected option */
    char *fName = "";   /* file name (initialized to "no name" by default) */
    int num_of_threads = 1; /* number of threads that will be used */

    opterr = 0;
    do
    {
        switch ((opt = getopt(argc, argv, "t:f:h")))
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
        case 't': /* file name */
            if (optarg[0] == '-')
            {
                fprintf(stderr, "%s: file name is missing\n", basename(argv[0]));
                printUsage(basename(argv[0]));
                return EXIT_FAILURE;
            }
            num_of_threads = atoi(optarg);
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
        printUsage(basename(argv[0]));
        return EXIT_FAILURE;
    }

    int *status_p;

    /* open the text file */

    FILE * fpointer;
    fpointer = fopen(fName, "rb");

    if (fpointer == NULL) {
        fprintf(stderr, "It occoured an error while openning file. \n"); 
        exit(EXIT_FAILURE);
    }

    /* measure time */

    struct timespec start, finish;
    double elapsed;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    /* generate worker threads */

    statusWorkers = malloc(num_of_threads * sizeof(int));   // Allocate memory to save the status of each worker
    pthread_t tIdWorkers[num_of_threads];
    unsigned int workers[num_of_threads];
    for (int i = 0; i < num_of_threads; i++)
        workers[i] = i;

    for (int i = 0; i < num_of_threads; i++)
    if (pthread_create (&tIdWorkers[i], NULL, worker, &workers[i]) != 0)                              /* thread worker */
    { perror ("error on creating thread worker");
        exit (EXIT_FAILURE);
    }

    /* read file content */

    int number_of_matrix;
    int order_of_matrix;
    
    // Read Number of Matrix from File
    if(fread(&number_of_matrix, sizeof(int), 1, fpointer) != 1)
        strerror(1);
    printf("Number of matrices to be read = %i \n", number_of_matrix);

    // Allocate memory to store each determinant
    matrixDeterminants = malloc( number_of_matrix * sizeof(double));        

    // Read Order of Matrix from File
    if(fread(&order_of_matrix, sizeof(int), 1, fpointer) != 1)
        strerror(1);
    printf("Matrices order = %i \n", order_of_matrix);

    // Store matrices in fifo
    for (int m = 1; m<=number_of_matrix; m++) {

        // Create buffer for the matrix
        double* buffer = malloc(order_of_matrix * order_of_matrix * sizeof(double));
        int s = fread(buffer, order_of_matrix * order_of_matrix * sizeof(double), 1, fpointer);
        if (s != 1)
            printf("Error creating matrix buffer.");

        // Save matrix in FIFO
        putMatrix(buffer, order_of_matrix, m);
    }
    
    /* close the text file */

    fclose(fpointer);

    /* save a struct in fifo for each thread to know that there are no more chunks to process */

    for (int i = 0; i < num_of_threads; i++) {
        endMatrix();
    }

    /* waiting for the termination of the intervening worker threads */

    for (int i = 0; i < num_of_threads; i++)
    { if (pthread_join (tIdWorkers[i], (void *) &status_p) != 0)                                       /* thread producer */
        { perror ("error on waiting for thread worker");
            exit (EXIT_FAILURE);
        }
        printf ("thread worker, with id %u, has terminated: ", i);
        printf ("its status was %d\n", *status_p);
    }

    /* measure time */
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &finish);
    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    /* print final results */

    for (int matrix_id = 0; matrix_id < number_of_matrix; matrix_id++) {
        printf("Processing matrix %d \n", matrix_id + 1);
        printf("Determinant: %.3e \n\n", matrixDeterminants[matrix_id]);
    }

    printf ("\nElapsed time = %.6f s\n", elapsed);
}


/**
 *  \brief print usage.
 */
static void printUsage(char *cmdName)
{
    fprintf(stderr, "\nSynopsis: %s OPTIONS [filename / positive number]\n"
                    "  OPTIONS:\n"
                    "  -h      --- print this help\n"
                    "  -f      --- filename\n"
                    "  -t      --- number of threads\n",
            cmdName);
}


/**
 *  \brief Function worker.
 *
 *  Its role is to get matrices of data and compute the determinant.
 *
 *  \param par pointer to application defined worker identification
 */
static void *worker(void *par) {
    unsigned int id = *((unsigned int *) par);      // worker id
    
    while (true) {
        // Get matrix
        struct MatrixInfo matrixinfo = getMatrix(id);

        // Checks if it is the matrix struct that tells that there are no more matrices to process
        if (matrixinfo.matrix_id == -1) break;

        // Process matrix
        double determinant = 1;
        computeDeterminant(&matrixinfo, &determinant);

        // Free the memory of the buffer
        free(matrixinfo.matrix_pointer);
        
        // Save result
        matrixDeterminants[matrixinfo.matrix_id - 1] = determinant;

        printf("Matrix %d processada pela thread %d\n", matrixinfo.matrix_id, id);
    }

    statusWorkers[id] = EXIT_SUCCESS;
    pthread_exit (&statusWorkers[id]);
}


/**
 *  \brief Function to compute the determinant of a matrix. 
 *
 *  \param matrixinfo pointer to the struct MatrixInfo containing the information about the matrix
 *  \param determinant pointer to a double to save the determinant value
 */
static void computeDeterminant(struct MatrixInfo * matrixinfo, double * determinant) {

    // Get information about the matrix
    int order_of_matrix = (*matrixinfo).order_of_matrix;
    double *matrix_pointer = (*matrixinfo).matrix_pointer;

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

    // Transform Matrix in a upper triangular matrix
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

    // Calculate Determinant from upper triangular matrix  (Multiply Diagonal Values)
    for(int l = 0; l < order_of_matrix; l++){
        *determinant = *determinant*matrix_coeficients[l][l];
    }

}
