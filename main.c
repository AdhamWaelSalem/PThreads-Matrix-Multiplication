#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    int *row;
    int *column;
    int size;
    int jump;
    int* result;
} Details;

void* calculateElement(void *arg) {
    int element = 0;
    Details details = *(Details *) arg;
    for (int i = 0; i < details.size; ++i) {
        element += details.row[i] * details.column[i * (details.jump)];
    }
    *details.result = element;
    free(arg);
    return NULL;
}
void* calculateRow(void *arg) {
    int element = 0;
    Details details = *(Details *) arg;
    for (int i = 0; i < details.jump; ++i) {
        for (int j = 0; j < details.size; ++j) {
            element += details.row[j] * (details.column[j * details.jump + i]);
        }
        details.result[i] = element;
        element = 0;
    }
    free(arg);
    return NULL;
}


int main() {

    //Declaring Attributes
    char fileName[128];
    FILE *file;
    int rows_1 = 0, rows_2 = 0, columns_1 = 0, columns_2 = 0;
    printf("\nMATRIX MULTIPLIER USING THREADS\n");

    while (1) {
        //Opening File for Reading
        while (1) {
            printf("\nEnter source file (to exit type exit) :\n");
            fgets(fileName, 127, stdin);
            fileName[strlen(fileName) - 1] = '\0';
            if (strcasecmp(fileName,"exit") == 0)
                exit(0);
            file = fopen(fileName, "r");
            if (file)
                break;
            printf("File not found, please try again!\n\n");
        }

        //Reading File
        //Take First Matrix Dimensions
        fscanf(file, "%d ", &rows_1);
        fscanf(file, "%d ", &columns_1);

        //Allocate First Matrix
        int matrix_1[rows_1][columns_1];

        //Fill First Matrix Elements
        for (int i = 0; i < rows_1; ++i) {
            for (int j = 0; j < columns_1; ++j) {
                fscanf(file, "%d ", &matrix_1[i][j]);
            }
        }

        //Take Second Matrix Dimensions
        fscanf(file, "%d ", &rows_2);
        fscanf(file, "%d ", &columns_2);

        //Allocate Second Matrix
        int matrix_2[rows_2][columns_2];

        //Fill Second Matrix Elements
        for (int i = 0; i < rows_2; ++i) {
            for (int j = 0; j < columns_2; ++j) {
                if (j == columns_2 - 1)
                    fscanf(file, "%d\n", &matrix_2[i][j]);
                else
                    fscanf(file, "%d ", &matrix_2[i][j]);
            }
        }
        char output[135] = {"output[ "};
        strcat(output,fileName);
        strcat(output," ].txt");
        FILE * outputFile = fopen(output,"w");
        if (!outputFile) {
            printf("Not able to Create output File\n\n");
            exit(0);
        }
        //Validate Multiplication
        if (columns_1 != rows_2) {
            fprintf(outputFile,"Matrix %d x %d cannot be multiplied by "
                               "Matrix %d x %d\n\n", rows_1, columns_1, rows_2, columns_2);
            fprintf(outputFile,"Reason:\n"
                               "If [a x b] * [c x d]\n"
                               "b must be equal to c and the output is [a x d]");
        }

        //Multiplication
        else {
            clock_t elapsedClocks;
            int matrix[rows_1][columns_2];
            Details * details;

            //Start of Threads per Element
            elapsedClocks = clock();
            pthread_t elements[rows_1][columns_2];
            for (int i = 0; i < rows_1; ++i) {
                for (int j = 0; j < columns_2; ++j) {
                    details = malloc(sizeof(Details));
                    details->size = rows_2;
                    details->row = matrix_1[i];
                    details->column = &matrix_2[0][j];
                    details->jump = columns_2;
                    details->result = &matrix[i][j];
                    if (pthread_create(&elements[i][j], NULL, &calculateElement, details) != 0)
                        perror("Thread creation failed.\n");

                }
            }
            for (int i = 0; i < rows_1; ++i) {
                for (int j = 0; j < columns_2; ++j) {
                    if (pthread_join(elements[i][j], NULL) != 0)
                        perror("Thread joining failed.\n");
                }
            }
            elapsedClocks = clock() - elapsedClocks;
            //End of Threads per Element

            //Print Thread per Element Results
            double elapsedTime = ((double)elapsedClocks)/CLOCKS_PER_SEC;
            fprintf(outputFile,"Thread per Element: elapsed time = %lf seconds\n", elapsedTime);
            for (int i = 0; i < rows_1; ++i) {
                for (int j = 0; j < columns_2; ++j) {
                    fprintf(outputFile, "%d\t", matrix[i][j]);
                }
                fprintf(outputFile,"\n");
            }

            //Start of Thread per Row
            elapsedClocks = clock();
            pthread_t rows[rows_1];
            for (int i = 0; i < rows_1; ++i) {
                details = malloc(sizeof(Details));
                details->column = matrix_2[0];
                details->jump = columns_2;
                details->row = matrix_1[i];
                details->result = matrix[i];
                details->size = rows_2;
                if (pthread_create(&rows[i], NULL, &calculateRow, details) != 0)
                    perror("Thread creation failed.\n");
            }
            for (int i = 0; i < rows_1; ++i) {
                if (pthread_join(rows[i], NULL) != 0)
                    perror("Thread joining failed.\n");
            }
            elapsedClocks = clock() - elapsedClocks;
            //End of Thread per Row

            //Print Thread per Row Results
            elapsedTime = ((double)elapsedClocks)/CLOCKS_PER_SEC;
            fprintf(outputFile,"\nThread per Row: elapsed time = %lf seconds\n", elapsedTime);
            for (int i = 0; i < rows_1; ++i) {
                for (int j = 0; j < columns_2; ++j) {
                    fprintf(outputFile, "%d\t", matrix[i][j]);
                }
                fprintf(outputFile,"\n");
            }
        }
        fclose(outputFile);

        //Exit Program
        while (1) {
            printf("\nResults created in file : output[ %s ].txt\n\n",fileName);
            printf("Do you want to exit? enter yes or no\n");
            fgets(fileName, 63, stdin);
            fileName[strlen(fileName) - 1] = '\0';
            if (strcasecmp(fileName, "yes") == 0)
                exit(0);
            else if (strcasecmp(fileName, "no") != 0) {
                printf("Invalid Input\n");
                continue;
            }
            break;
        }

    }
}

