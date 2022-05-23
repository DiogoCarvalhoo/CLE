/**
 *  \file computeDet.c (implementation file)
 *
 *  \brief Problem name: Compute Matrix Determinant.
 *
 *  Concurrency based on Message Passing Interface (MPI).
 *
 *  \author Diogo Filipe Amaral Carvalho - 92969 - April 2022
 *  \author Rafael Ferreira Baptista - 93367 - April 2022
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <libgen.h>
#include <unistd.h>
#include <pthread.h>


/** \brief struct to store the information of one matrix */
struct MatrixInfo {
   int matrix_id;           /* matrix identifier */  
   int order_of_matrix;     /* Order of the matrix */
   double * matrix_pointer; /* Pointer to the start of the matrix */
};

/** \brief struct to store the results of a matrix */
struct MatrixResults {
   int matrix_id;                                     /* matrix identifier */  
   double determinant;                                /* Number of total words */
};

/** \brief function responsible to present the program usage */
static void printUsage(char *cmdName);

/** \brief worker life cycle routine */
static void worker(int rank);

/** \brief dispatcher life cycle routine */
static int dispatcher(char *fName);

/** \brief number of workers */
int number_of_workers;

/** \brief to store the determinant of each matrix */
double * matrixDeterminants;

/** \brief function to compute determinant of a matrix */
static void computeDeterminant(struct MatrixInfo * matrixinfo, double * determinant);

int main(int argc, char *argv[])
{   

    int rank, size;

    /* Initialize MPI */

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    number_of_workers = size - 1;   // Number of worker processes

    /* process command line arguments */

    int opt;            /* selected option */
    char *fName = "";   /* file name (initialized to "no name" by default) */

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
        printUsage(basename(argv[0]));
        return EXIT_FAILURE;
    }
    
    if (rank == 0) {

        /* measure time */

        struct timespec start, finish;
        double elapsed;
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);

        /* Launch Dispatcher */
        int number_of_matrix;
        number_of_matrix = dispatcher(fName);

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
        
    
    
    } else {
        
        /* Launch worker life cycle */
        
        worker(rank);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
    
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
 *  \brief Function dispatcher.
 *
 *  Its role is to read the file, create matrix, send those matrix to workers, receive the results and save the results.
 *
 *  \param fName name of the file with matrix
 */
static int dispatcher(char *fName) {

    int current_worker_to_receive_work = 1;  // Id of worker that will receive next matrix to process
    int number_of_matrix_sent = 0;           // Number of matrix sent to workers

    /* open the text file */

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

    // Allocate memory to store each determinant
    matrixDeterminants = malloc( number_of_matrix * sizeof(double));        

    // Read Order of Matrix from File
    if(fread(&order_of_matrix, sizeof(int), 1, fpointer) != 1)
        strerror(1);
    printf("Matrices order = %i \n", order_of_matrix);

    // Send matrices to workers
    for (int m = 1; m<=number_of_matrix; m++) {

        /* Create new structure to keep information of the matrix */
        struct MatrixInfo newMatrix;
        newMatrix.matrix_id = m;
        newMatrix.order_of_matrix = order_of_matrix;
        newMatrix.matrix_pointer  = malloc(order_of_matrix * order_of_matrix * sizeof(double));

        int s = fread(newMatrix.matrix_pointer, order_of_matrix * order_of_matrix * sizeof(double), 1, fpointer);
        if (s != 1)
            printf("Error creating matrix buffer.");

        /* Send Matrix to Worker */
        MPI_Send(&newMatrix, sizeof(struct MatrixInfo), MPI_BYTE, current_worker_to_receive_work, 0, MPI_COMM_WORLD);
        MPI_Send(newMatrix.matrix_pointer, order_of_matrix * order_of_matrix * sizeof(double), MPI_BYTE, current_worker_to_receive_work, 0, MPI_COMM_WORLD);

         /* Update current_worker_to_receive_work and number_of_matrix_sent variables */
        current_worker_to_receive_work = (current_worker_to_receive_work%number_of_workers)+1;
        number_of_matrix_sent += 1;

        if (number_of_matrix_sent == number_of_workers) {
            // Receive results from workers

            for (int i = 1; i <= number_of_workers; i++) {

                // Receive result
                struct MatrixResults results;

                MPI_Recv(&results, sizeof(struct MatrixResults), MPI_BYTE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // Save results
                matrixDeterminants[results.matrix_id - 1] = results.determinant;
            }

            // Reset number_of_matrix
            number_of_matrix_sent = 0;
        }
    }
    
    /* close the text file */

    fclose(fpointer);

    /* Receive results of last matrix from workers */
    if (number_of_matrix_sent > 0) {

        for (int i = 1; i <= number_of_matrix_sent; i++) {

            // Receive result
            struct MatrixResults results;

            MPI_Recv(&results, sizeof(struct MatrixResults), MPI_BYTE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Save results
            matrixDeterminants[results.matrix_id - 1] = results.determinant;
        }

    }

    /* Send message to each process to know that there are no more matrix to process */

    for (int i = 1; i <= number_of_workers; i++) {
        struct MatrixInfo newMatrix;
        newMatrix.matrix_id = -1;
        newMatrix.order_of_matrix = -1;

        // Send Special Matrix to Worker
        MPI_Send(&newMatrix, sizeof(struct MatrixInfo), MPI_BYTE, i, 0, MPI_COMM_WORLD);

    } 

    return number_of_matrix;
}


/**
 *  \brief Function worker.
 *
 *  Its role is to receive matrices of data and compute the determinant.
 *
 *  \param par pointer to application defined worker identification
 */
static void worker(int rank) {
    
    while (true) {
        // Get matrix
        struct MatrixInfo newMatrix;

        MPI_Recv(&newMatrix, sizeof(struct MatrixInfo), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Checks if it is the matrix struct that tells that there are no more matrices to process
        if (newMatrix.matrix_id == -1) break;

        newMatrix.matrix_pointer = malloc(newMatrix.order_of_matrix * newMatrix.order_of_matrix * sizeof(double));

        MPI_Recv(newMatrix.matrix_pointer, newMatrix.order_of_matrix * newMatrix.order_of_matrix * sizeof(double), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Process matrix
        double determinant = 1;
        computeDeterminant(&newMatrix, &determinant);
        
        // Free the memory of the buffer
        free(newMatrix.matrix_pointer);
        
        // Send results back to dispatcher
        struct MatrixResults results;
        results.matrix_id = newMatrix.matrix_id;
        results.determinant = determinant;

        // Save result
        MPI_Send(&results, sizeof(struct MatrixResults), MPI_BYTE, 0, 0, MPI_COMM_WORLD);

    }

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
