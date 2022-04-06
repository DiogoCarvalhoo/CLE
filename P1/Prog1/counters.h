/**
 *  \file counters.h (interface file)
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

#ifndef COUNTERS_H
#define COUNTERS_H

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
extern void storeFileNames(int nFileNames, char *fileNames[]);

/**
 *  \brief Get a chunk from the data transfer region.
 *
 *  Operation carried out by the workers.
 *
 *  \param id thread identifier
 *  \param file_id file identifier
 *  \param total_words number of total words
 *  \param num_of_words_starting_with_vowel_chars
 *  \param num_of_words_ending_with_consonant_chars
 *
 *  \return value
 */
extern void saveResults (int id, int file_id, int total_words, int num_of_words_starting_with_vowel_chars, int num_of_words_ending_with_consonant_chars);

/**
 *  \brief Print final results
 *
 *  Operation carried out by the main thread.
 *
 */
extern void printResults ();


/** \brief struct to store the counters of a file*/
struct FileCounters {
   char* file_name;        /* file name */  
   int total_num_of_words;    /* Number of total words */
   int num_of_words_starting_with_vowel_chars;    /* Number of words starting with vowel chars */
   int num_of_words_ending_with_consonant_chars;    /* Number of words ending with consonant chars */
} FileCounters;

#endif /* COUNTERS_H */
