/**
 *  \file count_words.c (implementation file)
 *
 *  \brief Problem name: Count words.
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
#include <ctype.h> 
#include <stdbool.h>
#include <wchar.h>
#include <locale.h>
#include <time.h>
#include <pthread.h>

#include "chunks.h"
#include "probConst.h"
#include "counters.h"
#include "count_words_functions.h"


/** \brief worker life cycle routine */
static void *worker(void *par);

/** \brief count the words inside a chunk */
static void processChunk(struct ChunkInfo * chunkinfo, int * total_num_of_words, int * num_of_words_starting_with_vowel_chars, int * num_of_words_ending_with_consonant_chars);

/** \brief worker threads return status array */
int statusWorkers[N];

/** \brief main thread return status */
int statusProd;

/** \brief ideally number of bytes a chunk should have */
int num_bytes = N;  

/**
 *  \brief Main thread.
 *
 *  Its role is to generate the worker threads, create data chunks and save in the shared region.
 *  The worker threads will process the data chunks and save in another shared region.
 *  The main thread will wait for the threads termination and then print the final results.
 */
int main(int argc, char *argv[])
{   
    setlocale(LC_ALL, "en_US.UTF-8");
    int *status_p;
    // Timer
    double t0, t1, t2;
    t2 = 0.0;

    // Save filenames in the shared region and initialize counters to 0
    char *filenames[argc-2];
    for(int i=0; i<argc-2; i++) filenames[i] = argv[i+2];
    storeFileNames(argc-2, filenames);

    // Assign ids to each worker thread
    int num_of_threads = atoi(argv[1]);     // Get the number of threads from the program first argument
    pthread_t tIdWorkers[num_of_threads];
    unsigned int workers[num_of_threads];
    for (int i = 0; i < num_of_threads; i++)
        workers[i] = i;

    /* generate worker threads */
    
    for (int i = 0; i < num_of_threads; i++)
    if (pthread_create (&tIdWorkers[i], NULL, worker, &workers[i]) != 0)                              /* thread worker */
    { perror ("error on creating thread worker");
        exit (EXIT_FAILURE);
    }

    /* generate the chunks of each file and put in FIFO */
    
    t0 = ((double) clock ()) / CLOCKS_PER_SEC;
    // Iterate over all files passed by arguments
    for(int i=0;i<argc-2;i++){

        FILE * fpointer;
        unsigned char byte;        // Variable used to store each byte of the file  
        unsigned char *character;  // Initialization of variable used to store the char (singlebyte or multibyte)

        // Open file
        fpointer = fopen(filenames[i], "r");
        if (fpointer == NULL) {
            printf("It occoured an error while openning file: %s \n", argv[i]);
            exit(EXIT_FAILURE);
        }

        // Get file size
        fseek(fpointer, 0, SEEK_END);
        int size_of_file = ftell(fpointer);
        printf("File size: %d\n", size_of_file);

        fseek(fpointer, 0, SEEK_SET);           // Seek file to the start
        int number_of_processed_bytes = 0;      // Variable to also keep track of the initial position of the current chunk
        int size_of_current_chunk;              // the size of the chunk will probably vary, not always num_bytes

        // While there are still bytes to create a chunk 
        while (number_of_processed_bytes < size_of_file) {

            int size_of_current_char = 0; // Number of bytes read for the current char (it can be single byte or multibyte)

            if ( (number_of_processed_bytes + num_bytes) > size_of_file ) {     // If it is the last chunk of the file
                size_of_current_chunk = size_of_file - number_of_processed_bytes;   // Size of current chunk will be the remaining bytes
            } else {
                size_of_current_chunk = num_bytes;  // Default size of chunk
                fseek(fpointer, number_of_processed_bytes + size_of_current_chunk, SEEK_SET);       // Seek file to the end of chunk

                // Update the size of chunk in order to cut the file without cutting a word or a multibyte char
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

            // Seek file to the initial position of the chunk
            fseek(fpointer, number_of_processed_bytes, SEEK_SET);

            // Create buffer for the chunk with the current chunk + the last character
            unsigned char* buffer = malloc(size_of_current_chunk+size_of_current_char);
            int s = fread(buffer, size_of_current_chunk+size_of_current_char, 1, fpointer);
            if (s != 1)
                printf("Error creating chunk buffer.");

            // Save chunk in FIFO
            putChunk(buffer, size_of_current_chunk+size_of_current_char, i);

            number_of_processed_bytes += size_of_current_chunk;
        }

    }

    /* Save a struct in fifo for each thread to know that there are no more chunks to process */

    for (int i = 0; i < num_of_threads; i++) {
        endChunk();
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

    t1 = ((double) clock ()) / CLOCKS_PER_SEC;
    t2 += t1 - t0;

    /* print final results */

    printResults();
    printf("\nElapsed time = %.6f s\n", t2);
}


/**
 *  \brief Function worker.
 *
 *  Its role is to get chunks of data and count the words. After that, it incrementes the counters in shared region.
 *
 *  \param par pointer to application defined producer identification
 */
static void *worker(void *par) {
    unsigned int id = *((unsigned int *) par);      // worker id
    
    while (true) {
        // Get chunk of data
        struct ChunkInfo chunkinfo = getChunk(id);

        // Checks if it is the chunk that tells that there are no more chunks to process
        if (chunkinfo.fileId == -1) break;

        // Process chunk of data
        int total_num_of_words = 0;
        int num_of_words_starting_with_vowel_chars = 0;
        int num_of_words_ending_with_consonant_chars = 0;
        processChunk(&chunkinfo, &total_num_of_words, &num_of_words_starting_with_vowel_chars, &num_of_words_ending_with_consonant_chars);
        
        // Save chunk of data
        saveResults(id, chunkinfo.fileId, total_num_of_words, num_of_words_starting_with_vowel_chars, num_of_words_ending_with_consonant_chars);
    }

    statusWorkers[id] = EXIT_SUCCESS;
    pthread_exit (&statusWorkers[id]);
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

    //printf("Chunk data: %s\n", (*chunkinfo).file_pointer);
    unsigned char byte;             // Variable used to store each byte of the chunk
    int i = 0;                      // Counter to make sure to read only chunk_size bytes
    unsigned char *character;       // Initialization of variable used to store the char (singlebyte or multibyte)
    
    while (i < (*chunkinfo).chunk_size) {

        // Construction of the char
        byte = (*chunkinfo).file_pointer[i];        // Read a byte
        character = malloc((1+1)* sizeof(unsigned char) );      // Allocate memory to store the character. For now, it is a single byte
        character[0] = byte;

        if (byte > 192 && byte < 224) {   // 2-byte char
            i++;
            character = realloc(character, (2+1)* sizeof(unsigned char) );
            character[1] = (*chunkinfo).file_pointer[i];
            character[2] = 0;
        } else if ( byte > 224 && byte < 240) {     // 3-byte char
            character = realloc(character, (3+1)* sizeof(unsigned char) );
            i++;
            character[1] = (*chunkinfo).file_pointer[i];
            i++;
            character[2] = (*chunkinfo).file_pointer[i];
            character[3] = 0;
        } else if ( byte > 240 ) {     // 4-byte char
            character = realloc(character, (4+1)* sizeof(unsigned char) );
            i++;
            character[1] = (*chunkinfo).file_pointer[i];
            i++;
            character[2] = (*chunkinfo).file_pointer[i];
            i++;
            character[3] = (*chunkinfo).file_pointer[i];
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