#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include <stdbool.h>
#include <wchar.h>
#include <locale.h>
#include <time.h>


// Function to check if a char is vowel
int check_vowel(char c) {
    if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
        c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U') {
        return 1;
    } else {
        return 0;
    }
}

// Function to check if a char is consonant
int check_consonant(unsigned char c) {
    if (!check_vowel(c) && isalpha(c)) {
        return 1;
    } else {
        return 0;
    }
}

// Function to check if a char is decimal digit
int check_decimal_digit(unsigned char c) {
    if (isdigit(c)) {
        return 1;
    } else {
        return 0;
    }
}

// Function to check if a char is _
int check_underscore(unsigned char c) {
    if (c == '_') {
        return 1;
    } else {
        return 0;
    }
}

// Function to check if a char is a whitespace symbol
int check_whitespace(unsigned char c) {
    if ( c == 0x20 || c == 0x9 || c == 0xA || c == 0xD){
        return 1;
    } else {
        return 0;
    }
}

// Function to check if a char is a separation symbol
int check_separation(unsigned char c) {
    if (c == '-' || c == 0xE2809C || c == 0xe2809D || c == '"' || c == '[' || c == ']' || c == '(' || c == ')'){ 
        return 1;
    } else {
        return 0;
    }
}

// Function to check if char is a punctuation symbol
int check_punctuation(unsigned char c) {
    if( c == '.' || c == ',' || c == ':' || c == ';' || c == '?' || c =='!' || c == 0xE28093 || c == 0xE280A6 ){
        return 1;
    } else {
        return 0;
    }
}

// Function to check if char is apostrophe
int check_apostrophe(unsigned char c) {
    if( c == 0x27 || c == 0xE28098 || c == 0xE28099 ){
        return 1;
    } else {
        return 0;
    }
}


// Function to convert multibyte chars to singlebyte chars. For example: Given á -> return a
char convert_special_chars(wchar_t c) {
    switch (c) {
        // á à â ã - a
        case L'à': c=L'a'; break;
        case L'á': c=L'a'; break;
        case L'â': c=L'a'; break;
        case L'ã': c=L'a'; break;
        // Á À Â Ã - A
        case L'Á': c=L'A'; break;
        case L'À': c=L'A'; break;
        case L'Â': c=L'A'; break;
        case L'Ã': c=L'A'; break;
        // é è ê - e
        case L'è': c=L'e';break;
        case L'é': c=L'e';break;
        case L'ê': c=L'e';break;
        // É È Ê - E
        case L'É': c=L'E';break;
        case L'È': c=L'E';break;
        case L'Ê': c=L'E';break;
        // í ì - i
        case L'í': c=L'i';break;
        case L'ì': c=L'i';break;
        // Í Ì - I
        case L'Í': c=L'I';break;
        case L'Ì': c=L'I';break;
        // ó ò ô õ - o
        case L'ó': c=L'o'; break;
        case L'ò': c=L'o'; break;
        case L'ô': c=L'o'; break;
        case L'õ': c=L'o'; break;
        // Ó Ò Ô Õ - O
        case L'Ó': c=L'O'; break;
        case L'Ò': c=L'O'; break;
        case L'Ô': c=L'O'; break;
        case L'Õ': c=L'O'; break;
        // ú ù ü - u
        case L'ú': c=L'u';break;
        case L'ù': c=L'u';break;
        case L'ü': c=L'u';break;
        // Ú Ù - U
        case L'Ú': c=L'U';break;
        case L'Ù': c=L'U';break;
        // Ç ç - C
        case L'Ç': c=L'C';break;
        case L'ç': c=L'C';break;

        default:    break;
    }
    return c;
}



int main(int argc, char *argv[])
{   
    setlocale(LC_ALL, "en_US.UTF-8");

    // Timer
    double t0, t1, t2;
    t2 = 0.0;

    // Iterate over all files passed by arguments
    for(int i=1;i<argc;i++){

        FILE * fpointer;
        wchar_t c;  // Initialization of variable used to store the char
        int num_of_words_starting_with_vowel_chars = 0;
        int num_of_words_ending_with_consonant_chars = 0;
        int total_num_of_words = 0;

        // Flag used to determine if the algorithm is handling the char inside a word context or not
        bool inword = false;  

        // Flag used to check if the last char of a word was consonant or not     
        bool lastCharWasConsonant = false;

        // Open file
        fpointer = fopen(argv[i], "r");
        if (fpointer == NULL)
            //printf("It occoured an error while openning file: %s \n", file_name);  -> Se descomentar da erro pq?
            exit(EXIT_FAILURE);

        t0 = ((double) clock ()) / CLOCKS_PER_SEC;

        /*
        Parse file contents.
        Approach given by the professor:
        For each char until the end of file.
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
        while ((c = fgetwc(fpointer)) != EOF) {
            // Convert multybyte chars to singlebyte chars
            c = convert_special_chars(c);
            //printf("%c\n", c);
    
            if (inword) {

                if (check_consonant(c)) {
                    lastCharWasConsonant = true;
                } else if (check_vowel(c) || check_decimal_digit(c) || check_underscore(c) || check_apostrophe(c)) {
                    lastCharWasConsonant = false;
                } else if (check_whitespace(c) || check_separation(c) || check_punctuation(c)) {
                    inword = false;
                    if (lastCharWasConsonant) {
                        num_of_words_ending_with_consonant_chars += 1;
                    }
                    lastCharWasConsonant = false;
                }

            } else {

                if (check_whitespace(c) || check_separation(c) || check_punctuation(c) || check_apostrophe(c)) {
                    lastCharWasConsonant = false;
                } else if (check_vowel(c)) {
                    inword = true;
                    total_num_of_words += 1;
                    num_of_words_starting_with_vowel_chars += 1;
                } else if (check_consonant(c)) {
                    inword = true;
                    total_num_of_words += 1;
                    lastCharWasConsonant = true;
                } else if (check_decimal_digit(c) || check_underscore(c)) {
                    inword = true;
                    total_num_of_words += 1;
                }
            }
            
        }

        t1 = ((double) clock ()) / CLOCKS_PER_SEC;
        t2 += t1 - t0;

        // Close text file
        fclose(fpointer);

        // Print final results for current file
        printf("\nFile Name: %s\n", argv[i]);
        printf("Total number of words: %d\n", total_num_of_words);
        printf("Number of words starting with a vowel char: %d\n", num_of_words_starting_with_vowel_chars);
        printf("Number of words ending with a consonant char: %d\n", num_of_words_ending_with_consonant_chars);

    }

    printf("\nElapsed time = %.6f s\n", t2);
}

