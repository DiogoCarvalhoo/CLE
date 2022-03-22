#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include <stdbool.h>

#include <wchar.h>
#include <locale.h>

#include <math.h>
#include <time.h>
#include <libgen.h>
#include <unistd.h>


static void printUsage(char *cmdName);


int main(int argc, char *argv[])
{   

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


    printf("File name = %s\n", fName);

    // Time Variables
    double t0, t1, t2; 
    t2 = 0.0;

    FILE * fpointer;

    int number_of_matrix;
    int order_of_matrix;

    // Open File
    fpointer = fopen(fName, "rb");

    if (fpointer == NULL) {
        fprintf(stderr, "It occoured an error while openning file. \n"); 
        exit(EXIT_FAILURE);
    }
    
    // Read Number of Matrix from File
    if(fread(&number_of_matrix, sizeof(int), 1, fpointer) != 1)
        strerror(1);
    printf("Number of Matrix: %i \n", number_of_matrix);

    // Read Order of Matrix from File
    if(fread(&order_of_matrix, sizeof(int), 1, fpointer) != 1)
        strerror(1);
    printf("Order of Matrix: %i \n", order_of_matrix);

    // Define matrix coeficients structure (2D Array)
    double matrix_coeficients[order_of_matrix][order_of_matrix];

    for (int m = 1; m<=number_of_matrix; m++) {

        t0 = ((double) clock ()) / CLOCKS_PER_SEC;

        // For each Matrix
        double determinant = 1;

        // Read Matrix Coefficients
        for (int l = 0; l < order_of_matrix; l++) {
            if(fread(&matrix_coeficients[l], sizeof(double), order_of_matrix, fpointer) != 1)
                strerror(1);
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
                    determinant = 0;
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
            determinant = determinant*matrix_coeficients[l][l];
        }

        t1 = ((double) clock ()) / CLOCKS_PER_SEC;
        t2 = t2 + (t1 - t0);

        // Output Determinant Result
        printf("Processing matrix %i \n", m);
        printf("Determinant: %.3e \n\n", determinant);

    }

    // Close File
    fclose(fpointer);

    printf ("\nElapsed time = %.6f s\n", t2);

}




static void printUsage(char *cmdName)
{
    fprintf(stderr, "\nSynopsis: %s OPTIONS [filename / positive number]\n"
                    "  OPTIONS:\n"
                    "  -h      --- print this help\n"
                    "  -f      --- filename\n",
            cmdName);
}