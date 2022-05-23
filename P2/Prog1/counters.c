/**
 *  \file counters.c (implementation file)
 *
 *  \brief Problem name: Count Words.
 *
 *  In this file the structure and functions used to store the counters of each file are implemented.
 *
 *  Definition of the operations carried out by the dispatcher:
 *     \li storeFileNames
 *     \li saveResults
 *     \li printResults
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

/** \brief storage structure for counters */
static struct FileCounters * mem;


/**
 *  \brief Save results is performed to update file counter with new results
 *
 *  Operation carried out by the dispatcher.
 *
 *  \param file_id file identifier
 *  \param total_words number of total words
 *  \param num_of_words_starting_with_vowel_chars
 *  \param num_of_words_ending_with_consonant_chars
 * 
 */
void saveResults (int file_id, int total_words, int num_of_words_starting_with_vowel_chars, int num_of_words_ending_with_consonant_chars)
{

  mem[file_id].total_num_of_words += total_words;
  mem[file_id].num_of_words_starting_with_vowel_chars += num_of_words_starting_with_vowel_chars;
  mem[file_id].num_of_words_ending_with_consonant_chars += num_of_words_ending_with_consonant_chars;

}

/**
 *  \brief Save the file names and initialize counters.
 *
 *  Operation carried out by the dispatcher.
 *
 *  \param nFileNames number of files
 *  \param fileNames array with file names
 *
 */
void storeFileNames(int nFileNames, char *fileNames[]) {

    num_of_files = nFileNames;                                /* number of files */

    mem = malloc(nFileNames * sizeof(struct FileCounters));   /* allocation of memory for the structure that will store the counters */

    for (int i=0; i<nFileNames; i++) {
        mem[i].file_name = fileNames[i];
        mem[i].total_num_of_words = 0;
        mem[i].num_of_words_starting_with_vowel_chars = 0;
        mem[i].num_of_words_ending_with_consonant_chars = 0;
    }   

}


/**
 *  \brief Print final results
 *
 *  Operation carried out by the dispatcher.
 *
 */
void printResults ()
{
  for (int i = 0; i<num_of_files; i++) {
    printf("File name: %s\n", mem[i].file_name);
    printf("Total number of words: %d\n", mem[i].total_num_of_words);
    printf("Number of words starting with a vowel char: %d\n", mem[i].num_of_words_starting_with_vowel_chars);
    printf("Number of words ending with a consonant char: %d\n", mem[i].num_of_words_ending_with_consonant_chars);
  }
  
}