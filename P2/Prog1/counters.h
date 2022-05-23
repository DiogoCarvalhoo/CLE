/**
 *  \file counters.h (interface file)
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

#ifndef COUNTERS_H
#define COUNTERS_H

/**
 *  \brief Save the file names and initialize counters.
 *
 *  Operation carried out by the dispatcher.
 *
 *  \param nFileNames number of files
 *  \param fileNames array with file names
 *
 */
extern void storeFileNames(int nFileNames, char *fileNames[]);

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
extern void saveResults (int file_id, int total_words, int num_of_words_starting_with_vowel_chars, int num_of_words_ending_with_consonant_chars);

/**
 *  \brief Print final results
 *
 *  Operation carried out by the dispatcher.
 *
 */
extern void printResults ();


/** \brief struct to store the counters of a file*/
struct FileCounters {
   char* file_name;                                /* file name */  
   int total_num_of_words;                         /* Number of total words */
   int num_of_words_starting_with_vowel_chars;     /* Number of words starting with vowel chars */
   int num_of_words_ending_with_consonant_chars;   /* Number of words ending with consonant chars */
} FileCounters;

#endif /* COUNTERS_H */
