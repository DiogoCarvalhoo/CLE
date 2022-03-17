#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include <stdbool.h>

#include <wchar.h>
#include <locale.h>

#include <unistd.h>



int main(void)
{   

    char * directoryName = "dataSetProb2b/computeDet/";

    char * array[4];
    array[0] = "mat128_32.bin";
    array[1] = "dataSetProb2b/computeDet/mat128_64.bin";
    array[2] = "dataSetProb2b/computeDet/mat128_128.bin";
    array[3] = "dataSetProb2b/computeDet/mat128_256.bin";

    printf("%s\n", directoryName);


    // Iterate over all file names
    for (int f = 0; f<4; f++) {

        printf("File: %s\n ", array[f]);

        FILE * fpointer;

        int number_of_matrix;
        int order_of_matrix;

        fpointer = fopen(array[f], "rb");

        if (fpointer == NULL)
            //fprintf(stderr, "It occoured an error while openning file. \n"); 
            exit(EXIT_FAILURE);

        
        // Read Number of Matrix from File
        if(fread(&number_of_matrix, sizeof(int), 1, fpointer) != 1)
            strerror(1);
        printf("Number of Matrix: %i \n", number_of_matrix);

        // Read Order of Matrix from File
        if(fread(&order_of_matrix, sizeof(int), 1, fpointer) != 1)
            strerror(1);
        printf("Order of Matrix: %i \n", order_of_matrix);

        int result = order_of_matrix * order_of_matrix;
        double matrix_coeficients[order_of_matrix][order_of_matrix];

        for (int m = 1; m<number_of_matrix; m++) {
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
                    printf("VIM AQUI AO ELSE ! \n\n\n");
                    // Find column != 0.0 to be changed
                    int columnToChange = -1;
                    for (int j=l+1; j<order_of_matrix; j++) {
                        if (matrix_coeficients[l][j] != 0.0) {
                            columnToChange = j;
                        }
                    }

                    if (columnToChange == -1) {
                        determinant = 0;
                        printf("VOU DAR BREAK AQUI ! \n\n\n");
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


            // Calculate Determinant from upper triangular matrix
            for(int l = 0; l < order_of_matrix; l++){
                determinant = determinant*matrix_coeficients[l][l];
            }

/*
            // Print Matrix For Debug Purposes
            printf("Matrix Number: %i \n", m);
            for (int l = 0; l<order_of_matrix; l++) {
                for (int c = 0; c<order_of_matrix; c++) {
                    printf("%4.01f ", matrix_coeficients[l][c]);
                }

                printf("\n");
            }
*/

            // Print Matrix Determinant
            printf("Determinant: %f \n", determinant);
            printf("----\n");

            //sleep(3);
        }

        fclose(fpointer);

    }

}

