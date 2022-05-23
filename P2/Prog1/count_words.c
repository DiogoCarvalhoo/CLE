/**
 *  \file count_words.c (implementation file)
 *
 *  \brief Problem name: Count words.
 *
 *  Concurrency based on Message Passage Interface (MPI).
 *
 *  \author Diogo Filipe Amaral Carvalho - 92969 - April 2022
 *  \author Rafael Ferreira Baptista - 93367 - April 2022
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include <stdbool.h>
#include <wchar.h>
#include <locale.h>
#include <time.h>
#include <pthread.h>

#include "counters.h"
#include "probConst.h"
#include "auxiliar_functions.h"


/** \brief struct to store the information of one chunk*/
struct ChunkInfo {
    int file_id;                     /* file identifier */  
    int chunk_size;                  /* Number of bytes of the chunk */
    unsigned char* chunk_info;       /* Pointer to the start of the chunk */
};

/** \brief struct to store the results of a file */
struct FileResults {
   int file_id;                                       /* file identifier */  
   int total_num_of_words;                            /* Number of total words */
   int num_of_words_starting_with_vowel_chars;        /* Number of words starting with vowel chars */
   int num_of_words_ending_with_consonant_chars;      /* Number of words ending with consonant chars */
};

/** \brief worker life cycle routine */
static void worker(int rank);

/** \brief dispatcher life cycle routine */
static void dispatcher(char *filenames[], int number_of_files);

/** \brief count the words inside a chunk */
static void processChunk(struct ChunkInfo * chunkinfo, int * total_num_of_words, int * num_of_words_starting_with_vowel_chars, int * num_of_words_ending_with_consonant_chars);

/** \brief number of workers */
int number_of_workers;

/** \brief ideally number of bytes that a chunk should have */
int num_bytes = N;  

/**
 *  \brief Main thread.
 *
 *  Its role is to initialize MPI, launch dispatcher and workers and measure execution time.
 *  The workers will receive chunks of data, process those chunks and send the results back to dispatcher.
 *  The dispatcher will read files, create chunks, send chunks to workers and receive results.
 */

int main(int argc, char *argv[])
{   
    int rank, size;

    /* Initialize MPI */

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    setlocale(LC_ALL, "en_US.UTF-8");

    number_of_workers = size - 1;   // Number of worker processes

    if (rank == 0) {


        /* measure time */

        struct timespec start, finish;
        double elapsed;
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);

        /* Read File Names */

        char *filenames[argc-1];

        for (int i = 1; i<argc; i++) {
            filenames[i-1] = argv[i];
        }


        /* Launch Dispatcher */

        dispatcher(filenames, argc-1);


        /* measure time */

        clock_gettime(CLOCK_MONOTONIC_RAW, &finish);
        elapsed = (finish.tv_sec - start.tv_sec);
        elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

        /* print final results */

        printResults();
        printf("\nElapsed time = %.6f s\n", elapsed);


    } else {
        
        /* Launch worker life cycle */
        
        worker(rank);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
    
}



/**
 *  \brief Function dispatcher.
 *
 *  Its role is to read files, create chunks of data, send those chunks to workers, receive the results and save the results.
 *
 *  \param filenames pointer to array that contains the name of each file
 *  \param number_of_files total number of files to analyze
 */
static void dispatcher(char *filenames[], int number_of_files) {

    int current_worker_to_receive_work = 1;  // Id of worker that will receive next chunk to process
    int number_of_chunks_sent = 0;           // Number of chunks sent to workers

    /* Initalize counters to 0 for each file */

    storeFileNames(number_of_files, filenames);

    /* Iterate over all files passed in arguments */

    for(int i=0;i<number_of_files;i++){

        FILE * fpointer;
        unsigned char byte;        // Variable used to store each byte of the file  
        unsigned char *character;  // Variable used to store the char (singlebyte or multibyte)

        /* Open file */
        fpointer = fopen(filenames[i], "r");
        if (fpointer == NULL) {
            printf("It occoured an error while openning file: %s \n", filenames[i]);
            exit(EXIT_FAILURE);
        }

        /* Get file size */
        fseek(fpointer, 0, SEEK_END);
        int size_of_file = ftell(fpointer);

        fseek(fpointer, 0, SEEK_SET);           // Seek file to the start
        int number_of_processed_bytes = 0;      // Variable to also keep track of the initial position of the current chunk
        int size_of_current_chunk;              // the size of the chunk will probably vary, not always num_bytes
            
        /* While there are still bytes to create a chunk */
        while (number_of_processed_bytes < size_of_file) {

            int size_of_current_char = 0; // Number of bytes read for the current char (it can be single byte or multibyte)

            if ( (number_of_processed_bytes + num_bytes) > size_of_file ) {     // If it is the last chunk of the file
                size_of_current_chunk = size_of_file - number_of_processed_bytes;   // Size of current chunk will be the remaining bytes
            } else {
                size_of_current_chunk = num_bytes;  // Default size of chunk
                fseek(fpointer, number_of_processed_bytes + size_of_current_chunk, SEEK_SET);       // Seek file to the end of chunk

                /* Update the size of chunk in order to cut the file without cutting a word or a multibyte char */
                while (true) {                  
                    byte = fgetc(fpointer);    // Read a byte

                    size_of_current_char = 1;  
                    character = malloc((1+1)* sizeof(unsigned char) );      // the last byte of the character is required to be 0
                    character[0] = byte;

                    // We just need to check if it is a 3-byte char or a single byte char because the safe-cut-chars are:
                    //  - whitespace (single byte)
                    //  - separation (single byte or multibyte)
                    //  - punctuation (single byte or multibyte)
                    if ( byte > 224 && byte < 240) {     // 3-byte char
                        // Create the 3-byte character
                        character = realloc(character, (3+1)* sizeof(unsigned char) );      // re allocate memory in order to save a 3-byte char
                        byte = fgetc(fpointer);    // Read another byte
                        character[1] = byte;
                        byte = fgetc(fpointer);    // Read another byte
                        character[2] = byte;
                        character[3] = 0;
                        size_of_current_char += 2;
                    } else {                        // single byte char
                        character[1] = 0;
                    }

                    // If char is whitespace | separation | punctuation, then it is a safe place to cut the chunk, so we break
                    if (check_whitespace(character) || check_separation(character) || check_punctuation(character)) {
                        break;
                    }

                    // Increment the size of chunk by the number of bytes read
                    size_of_current_chunk += size_of_current_char;
                }
            }

            /* Seek file to the initial position of the chunk */
            fseek(fpointer, number_of_processed_bytes, SEEK_SET);

            /* Create new structure to keep information of the chunk */
            struct ChunkInfo newChunk;
            newChunk.file_id = i;
            newChunk.chunk_size = size_of_current_chunk+size_of_current_char;
            newChunk.chunk_info = (unsigned char*) malloc(size_of_current_chunk+size_of_current_char);
                        
            int s = fread(newChunk.chunk_info, size_of_current_chunk+size_of_current_char, 1, fpointer);
            if (s != 1)
                printf("Error creating chunk buffer.");

            /* Send Chunk to Worker */
            MPI_Send(&newChunk, sizeof(struct ChunkInfo), MPI_BYTE, current_worker_to_receive_work, 0, MPI_COMM_WORLD);
            MPI_Send(newChunk.chunk_info, newChunk.chunk_size, MPI_BYTE, current_worker_to_receive_work, 0, MPI_COMM_WORLD);

            /* Update current_worker_to_receive_work and number_of_chunks_sent variables */
            current_worker_to_receive_work = (current_worker_to_receive_work%number_of_workers)+1;
            number_of_chunks_sent += 1;

            number_of_processed_bytes += size_of_current_chunk;

            if (number_of_chunks_sent == number_of_workers) {
                // Receive results from workers

                for (int i = 1; i <= number_of_workers; i++) {

                    // Receive result
                    struct FileResults results;

                    MPI_Recv(&results, sizeof(struct FileResults), MPI_BYTE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    // Save results
                    saveResults(results.file_id, results.total_num_of_words, results.num_of_words_starting_with_vowel_chars, results.num_of_words_ending_with_consonant_chars);
                }

                // Reset number_of_chunks
                number_of_chunks_sent = 0;
            }

        }

        // Close file
        fclose(fpointer);
    }

    /* Receive results of last chunks from workers */
    if (number_of_chunks_sent > 0) {

        for (int i = 1; i <= number_of_chunks_sent; i++) {

            // Receive result
            struct FileResults results;

            MPI_Recv(&results, sizeof(struct FileResults), MPI_BYTE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Save results
            saveResults(results.file_id, results.total_num_of_words, results.num_of_words_starting_with_vowel_chars, results.num_of_words_ending_with_consonant_chars);
        }
            
    }
        

    /* Send message to each process to know that there are no more chunks to process */

    for (int i = 1; i <= number_of_workers; i++) {
        struct ChunkInfo newChunk;
        newChunk.file_id = -1;
        newChunk.chunk_size = -1;

        // Send Special Chunk to Worker
        MPI_Send(&newChunk, sizeof(struct ChunkInfo), MPI_BYTE, i, 0, MPI_COMM_WORLD);

    }    

}






/**
 *  \brief Function worker.
 *
 *  Its role is to get chunks of data and count the words. After that, it sends the results back to dispatcher.
 *
 *  \param rank worker process identification
 */
static void worker(int rank) {
    
    while (true) {

        // Get chunk of data
        struct ChunkInfo newChunk;

        MPI_Recv(&newChunk, sizeof(struct ChunkInfo), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Checks if it is the chunk that tells that there are no more chunks to process
        if (newChunk.file_id == -1) break;

        newChunk.chunk_info = (unsigned char*) malloc(newChunk.chunk_size);

        MPI_Recv(newChunk.chunk_info, newChunk.chunk_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Process chunk of data
        int total_num_of_words = 0;
        int num_of_words_starting_with_vowel_chars = 0;
        int num_of_words_ending_with_consonant_chars = 0;

        processChunk(&newChunk, &total_num_of_words, &num_of_words_starting_with_vowel_chars, &num_of_words_ending_with_consonant_chars);

        // Free the memory of the buffer
        free(newChunk.chunk_info);

        // Send results back to dispatcher
        struct FileResults results;
        results.file_id = newChunk.file_id;
        results.total_num_of_words = total_num_of_words;
        results.num_of_words_starting_with_vowel_chars = num_of_words_starting_with_vowel_chars;
        results.num_of_words_ending_with_consonant_chars = num_of_words_ending_with_consonant_chars;
        
        MPI_Send(&results, sizeof(struct FileResults), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
    }

}


/**
 *  \brief Function to count the words of a chunk. 
 *  It counts the total number of words, the number of words starting with vowel chars 
 *  and the number of words ending with consonant chars.
 *
 *  \param chunkinfo pointer to the struct ChunkInfo containing the information about the chunk
 *  \param total_num_of_words pointer to a int to save the total number of words
 *  \param num_of_words_starting_with_vowel_chars pointer to a int to save the number of words starting with vowel
 *  \param num_of_words_ending_with_consonant_chars pointer to a int to save the number of words ending with consonant
 */
static void processChunk(struct ChunkInfo * chunkinfo, int * total_num_of_words, int * num_of_words_starting_with_vowel_chars, int * num_of_words_ending_with_consonant_chars) {
    // Flag used to determine if the algorithm is handling the char inside a word context or not
    bool inword = false;  

    // Flag used to check if the last char of a word was consonant or not     
    bool lastCharWasConsonant = false;
    
    //printf("Chunk data: %s\n", (*chunkinfo).chunk_info);
    unsigned char byte;             // Variable used to store each byte of the chunk
    int i = 0;                      // Counter to make sure to read only chunk_size bytes
    unsigned char *character;       // Initialization of variable used to store the char (singlebyte or multibyte)
    


    while (i < (*chunkinfo).chunk_size) {

        // Construction of the char
        byte = (*chunkinfo).chunk_info[i];        // Read a byte


        character = malloc((1+1)* sizeof(unsigned char) );      // Allocate memory to store the character. For now, it is a single byte
        character[0] = byte;

        if (byte > 192 && byte < 224) {   // 2-byte char
            i++;
            character = realloc(character, (2+1)* sizeof(unsigned char) );
            character[1] = (*chunkinfo).chunk_info[i];
            character[2] = 0;
        } else if ( byte > 224 && byte < 240) {     // 3-byte char
            character = realloc(character, (3+1)* sizeof(unsigned char) );
            i++;
            character[1] = (*chunkinfo).chunk_info[i];
            i++;
            character[2] = (*chunkinfo).chunk_info[i];
            character[3] = 0;
        } else if ( byte > 240 ) {     // 4-byte char
            character = realloc(character, (4+1)* sizeof(unsigned char) );
            i++;
            character[1] = (*chunkinfo).chunk_info[i];
            i++;
            character[2] = (*chunkinfo).chunk_info[i];
            i++;
            character[3] = (*chunkinfo).chunk_info[i];
            character[4] = 0;
        } else {        // single byte char
            character[1] = 0;
        }

        // Convert multybyte chars to singlebyte chars according the Portuguese special characters encoding
        if (character[0] == 0xC3) {
            byte = convert_special_chars(character[1]);
            character = realloc(character, (1+1)* sizeof(unsigned char) );      // Convert multibyte char to single byte
            character[0] = byte;
            character[1] = 0;
        }


        /*
        Parse file contents.
        Approach given by the professor:
        If inword = false:
            If char is whitespace/separation/punctuation/aphostrophe: inword remains set to false.
            If char is vowel/consonant/decimal_digit/underscore: inword is set to true, increment total words
                in particular case char is vowel: increment words starting with vowel
                in particular case char it consonant: set lastCharWasConsonant to true
        If inword = true:
            If char is vowel/consonant/decimal_digit/underscore/apostrophe: inword stays in true.
                in particular case char is consonant: set lastCharWasConsonant to true. Otherwise set lastCharWasConsonant to false
            If char is whitespace/separation/punctuation: change inword to false
                if lastCharWasConsonant is true: increment num_of_words_ending_with_consonant_chars
                lastCharWasConsonant = false
        */
 
 
        if (inword) {

            if (check_consonant(character)) {
                lastCharWasConsonant = true;
            } else if (check_vowel(character) || check_decimal_digit(character) || check_underscore(character) || check_apostrophe(character)) {
                lastCharWasConsonant = false;
            } else if (check_whitespace(character) || check_separation(character) || check_punctuation(character)) {
                inword = false;
                if (lastCharWasConsonant) {
                    *num_of_words_ending_with_consonant_chars += 1;
                }
                lastCharWasConsonant = false;
            }

        } else {

            if (check_whitespace(character) || check_separation(character) || check_punctuation(character) || check_apostrophe(character)) {
                lastCharWasConsonant = false;
            } else if (check_vowel(character)) {
                inword = true;
                *total_num_of_words += 1;
                *num_of_words_starting_with_vowel_chars += 1;
            } else if (check_consonant(character)) {
                inword = true;
                *total_num_of_words += 1;
                lastCharWasConsonant = true;
            } else if (check_decimal_digit(character) || check_underscore(character)) {
                inword = true;
                *total_num_of_words += 1;
            }
        }
        
        i++;
        free(character);
    }

}
