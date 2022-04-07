/**
 *  \file auxiliar_functions.h (interface file)
 *
 *  \brief Problem name: Count words.
 *
 *  In this file the 8 boolean functions used to process a chunk are defined and a function responsible of
 *  convert a multibyte char to a single byte char.
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Generator thread of the intervening entities.
 *
 *  \author Diogo Filipe Amaral Carvalho - 92969 - April 2022
 *  \author Rafael Ferreira Baptista - 93367 - April 2022
 */
#ifndef COUNT_WORDS_FUNCTIONS_H
#define COUNT_WORDS_FUNCTIONS_H

// Function to check if a char is vowel
extern int check_vowel(unsigned char *c);

// Function to check if a char is consonant
extern int check_consonant(unsigned char *c);

// Function to check if a char is decimal digit
extern int check_decimal_digit(unsigned char *c);

// Function to check if a char is _
extern int check_underscore(unsigned char *c);

// Function to check if a char is a whitespace symbol
extern int check_whitespace(unsigned char *c);

// Function to check if a char is a separation symbol
extern int check_separation(unsigned char *c);

// Function to check if char is a punctuation symbol
extern int check_punctuation(unsigned char *c);

// Function to check if char is apostrophe
extern int check_apostrophe(unsigned char *c);

// Function to convert multibyte chars to singlebyte chars. For example: Given á -> return a
extern char convert_special_chars(unsigned char c);

#endif /* COUNT_WORDS_FUNCTIONS_H */