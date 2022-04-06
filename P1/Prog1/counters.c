/**
 *  \file counters.c (implementation file)
 *
 *  \brief Problem name: Count Words.
 *
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Data transfer region implemented as a monitor.
 *
 *  Definition of the operations carried out by the main thread:
 *     \li storeFileNames
 *     \li printResults
 * 
 *  Definition of the operations carried out by the workers:
 *     \li saveResults
 *
 *  \author Diogo Filipe Amaral Carvalho - 92969 - April 2022
 *  \author Rafael Ferreira Baptista - 93367 - April 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "probConst.h"

/** \brief struct to store the counters of a file */
struct FileCounters {
   char* file_name;                                   /* file name */  
   int total_num_of_words;                            /* Number of total words */
   int num_of_words_starting_with_vowel_chars;        /* Number of words starting with vowel chars */
   int num_of_words_ending_with_consonant_chars;      /* Number of words ending with consonant chars */
};

/** \brief total number of files to process */
static int num_of_files;

/** \brief storage region for counters */
static struct FileCounters * mem;

/** \brief workers threads return status array */
extern int statusWorkers[N];

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;


/**
 *  \brief Save results performed by a worker
 *
 *  Operation carried out by the worker threads.
 *
 *  \param id thread identifier
 *  \param file_id file identifier
 *  \param total_words number of total words
 *  \param num_of_words_starting_with_vowel_chars
 *  \param num_of_words_ending_with_consonant_chars
 */
void saveResults (int id, int file_id, int total_words, int num_of_words_starting_with_vowel_chars, int num_of_words_ending_with_consonant_chars)
{
  if ((statusWorkers[id] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
     { errno = statusWorkers[id];                                                            /* save error in errno */
       perror ("error on entering monitor(CF)");
       statusWorkers[id] = EXIT_FAILURE;
       pthread_exit (&statusWorkers[id]);
     }

  mem[file_id].total_num_of_words += total_words;
  mem[file_id].num_of_words_starting_with_vowel_chars += num_of_words_starting_with_vowel_chars;
  mem[file_id].num_of_words_ending_with_consonant_chars += num_of_words_ending_with_consonant_chars;

  if ((statusWorkers[id] = pthread_mutex_unlock (&accessCR)) != 0)                                  /* exit monitor */
     { errno = statusWorkers[id];                                                            /* save error in errno */
       perror ("error on exiting monitor(CF)");
       statusWorkers[id] = EXIT_FAILURE;
       pthread_exit (&statusWorkers[id]);
     }
}

/**
 *  \brief Save the file names and initialize counters.
 *
 *  Operation carried out by the main thread.
 *
 *  \param nFileNames number of files
 *  \param fileNames array with file names
 *
 *  \return value
 */
void storeFileNames(int nFileNames, char *fileNames[]) {
  
    if ((pthread_mutex_lock (&accessCR)) != 0) {                             /* enter monitor */                       
       perror ("error on entering monitor(CF)");                            /* save error in errno */
       int status = EXIT_FAILURE;
       pthread_exit(&status);
    }
    
    num_of_files = nFileNames;                     /* number of files */

    mem = malloc(num_of_files * sizeof(struct FileCounters));   /* memory allocation for the region storing the counters */

    for (int i=0; i<nFileNames; i++) {
        mem[i].file_name = fileNames[i];
        mem[i].total_num_of_words = 0;
        mem[i].num_of_words_starting_with_vowel_chars = 0;
        mem[i].num_of_words_ending_with_consonant_chars = 0;
    }   

    if ((pthread_mutex_unlock (&accessCR)) != 0) {                   /* exit monitor */                                                
       perror ("error on exiting monitor(CF)");                     /* save error in errno */
       int status = EXIT_FAILURE;
       pthread_exit(&status);
    }

}



/**
 *  \brief Print final results
 *
 *  Operation carried out by the main thread.
 *
 */
void printResults ()
{
  if ((pthread_mutex_lock (&accessCR)) != 0) {                             /* enter monitor */                       
       perror ("error on entering monitor(CF)");                            /* save error in errno */
       int status = EXIT_FAILURE;
       pthread_exit(&status);
    }

  for (int i = 0; i<num_of_files; i++) {
    printf("File name: %s\n", mem[i].file_name);
    printf("Total number of words: %d\n", mem[i].total_num_of_words);
    printf("Number of words starting with a vowel char: %d\n", mem[i].num_of_words_starting_with_vowel_chars);
    printf("Number of words ending with a consonant char: %d\n", mem[i].num_of_words_ending_with_consonant_chars);
  }
  

  if ((pthread_mutex_unlock (&accessCR)) != 0) {                   /* exit monitor */                                                
       perror ("error on exiting monitor(CF)");                     /* save error in errno */
       int status = EXIT_FAILURE;
       pthread_exit(&status);
    }
}