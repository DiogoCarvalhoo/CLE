#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include <stdbool.h>


// Function to check if a char is vowel
int check_vowel(char c) {
    if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') {
        return 1;
    } else {
        return 0;
    }
}

// Function to check if a char is consonant
int check_consonant(char c) {
    if (c == 'b' || c == 'c' || c == 'd' || c == 'f' || c == 'g' || c == 'h' || c == 'j' || c == 'k' || c == 'l' || c == 'm' || c == 'n' || c == 'p' || c == 'q' || c == 'r' || c == 's' || c == 't' || c == 'v' || c == 'x' || c == 'y' || c == 'z' ) {
        return 1;
    } else {
        return 0;
    }
}


//Function to convert decimal into hexadecimal
char * decimalToHexa(int decimalnum) {
    int quotient, remainder;
    int j = 0;
    char *hexadecimalnum =  malloc (sizeof (char) * 8);

    quotient = decimalnum;
 
    while (decimalnum != 0) {
        remainder = decimalnum%16;
        if(remainder<10)
            remainder = remainder+48;
        else
            remainder = remainder+55;
        hexadecimalnum[j] = remainder;
        j++;
        decimalnum = decimalnum/16;
    }

    char *finalnumber =  malloc (sizeof (char) * 8);
    int k = 0;
    for (int i = j-1 ;i>=0;i--) {
	    finalnumber[k]  = hexadecimalnum[i];
        k++;
    }
    return finalnumber;
}



//Function to convert special-char hexadecimal code in associated non special char. Example: Return 'i' when receiving hexadecimal code of 'í' 
char convert_special_chars(char* code) {
    
    if (strcmp(code,"C3A1") == 0 || strcmp(code,"C3A0") == 0 || strcmp(code,"C3A2") == 0 || strcmp(code,"C3A3") == 0 || strcmp(code,"C381") == 0 || strcmp(code,"C380") == 0 || strcmp(code,"C382") == 0 || strcmp(code,"C383") == 0 ) {
        return 'a';
    }

    if (strcmp(code,"C3A9") == 0 || strcmp(code,"C3A8") == 0 || strcmp(code,"C3AA") == 0 || strcmp(code,"C389") == 0 || strcmp(code,"C388") == 0 || strcmp(code,"C38A") == 0 ) {
        return 'e';
    }

    if (strcmp(code,"C3AD") == 0 || strcmp(code,"C3AC") == 0 || strcmp(code,"C38D") == 0 || strcmp(code,"C38C") == 0 ) {
        return 'i';
    }

    if (strcmp(code,"C3B3") == 0 || strcmp(code,"C3B2") == 0 || strcmp(code,"C3B4") == 0 || strcmp(code,"C3B5") == 0 || strcmp(code,"C393") == 0 || strcmp(code,"C392") == 0 || strcmp(code,"C394") == 0 || strcmp(code,"C395") == 0 ) {
        return 'o';
    }

    if (strcmp(code,"C3BA") == 0 || strcmp(code,"C3B9") == 0 || strcmp(code,"C39A") == 0 || strcmp(code,"C399") == 0 ) {
        return 'u';
    }

    if (strcmp(code,"C3A7") == 0 || strcmp(code,"C387") == 0 ) {
        return 'c';
    }
}




char *sliceString(char *str, int start, int end)
{

    int i;
    int size = (end - start) + 2;
    char *output = (char *)malloc(size * sizeof(char));

    for (i = 0; start <= end; start++, i++)
    {
        output[i] = str[start];
    }

    output[size] = '\0';

    return output;
}





// Function to check if a word only contains alphanumeric or _ characters 
int check_word(unsigned char * word, int * num_of_words_starting_with_vowel_chars, int * num_of_words_ending_with_consonant_chars, int * total_num_of_words) {

    int chars_read = 0;
    char firstChar;
    char lastChar;

    bool lastCharFlag = false;

    for (int i=0; word[i]; i++) {
        

        char finalChar;
        char char_code[100] = "";
        wchar_t byte0 = word[i];

        if (byte0 & (1<<7) && byte0 & (1<<6)) {
            // If char is multibyte char

            // Calculate char hexadecimal code
            char *result = decimalToHexa(word[i]);
            strcat(char_code, result);
            free(result);
            i += 1;

            if (byte0 & (1<<5)) {

                char *result = decimalToHexa(word[i]);
                strcat(char_code, result);
                free(result);
                i += 1;

                if (byte0 & (1<<4)) {
                    char *result = decimalToHexa(word[i]);
                    strcat(char_code, result);
                    free(result);
                    i += 1;

                } else {
                    char *result = decimalToHexa(word[i]);
                    strcat(char_code, result);
                    free(result);
                }
            } else {
                char *result = decimalToHexa(word[i]);
                strcat(char_code, result);
                free(result);
            }
            
            // Transform special char hexadecimal code in associated non-special char.
            char c = convert_special_chars(char_code);

            // Verify if non-special char is alphanumeric or underscore characters
            if (isalnum(c) == 0 && c != '_' ) {
                return 0;
            }

            finalChar = c;

        } else {
            // If char is not multibyte char

            // Verify if char is alphanumeric or underscore characters
            if (isalnum(word[i]) == 0 && word[i] != '_' ) {
                return 0;
            }

            finalChar = word[i];

        }

        if (chars_read == 0) {
            firstChar = finalChar;
        }
        lastChar = finalChar;

        chars_read++;
    }

    *total_num_of_words =  *total_num_of_words + 1;

    // Get first and last char of the word and convert to lowercase in order to facilitate the if conditions
    //char first_char = tolower(*word);
    //char last_char = tolower(*(word + strlen(word) - 1));
    
    //printf("First Char: %c\n",firstChar);
    // Check if word starts with a vowel
    if (check_vowel(firstChar)) {
        *num_of_words_starting_with_vowel_chars = *num_of_words_starting_with_vowel_chars + 1;
    } 
    
    //printf("Last Char: %c\n",lastChar);
    // Check if word ends with a consonant
    if (check_consonant(lastChar)) {
        *num_of_words_ending_with_consonant_chars = *num_of_words_ending_with_consonant_chars + 1;
    }

    return 1;
}




int main(void)
{   
    char file_names_input[50];
    char * file_name;

    // Get user input and save in file_names_input variable
    printf("Enter the names of the files separated by spaces: ");
    fgets(file_names_input, 50, stdin);

    // Remove end of line char from the user input
    file_names_input[strcspn(file_names_input, "\n\r")] = 0;    

    // Split file names by spaces and get the first filename
    char * rest_files = file_names_input;   // Variable needed tosplit the file names using the strtok_r function
    file_name = strtok_r(file_names_input, " ", &rest_files);

    // Iterate over all file names
    while (file_name != NULL) {

        FILE * fpointer;
        char line[255];
        int num_of_words_starting_with_vowel_chars = 0;
        int num_of_words_ending_with_consonant_chars = 0;
        int total_num_of_words = 0;
        unsigned char * word;
        char delimit[] = " .?!,:;-()[]/“”\"‘’";

        fpointer = fopen(file_name, "r");

        if (fpointer == NULL)
            //printf("It occoured an error while openning file: %s \n", file_name);  -> Se descomentar da erro pq?
            exit(EXIT_FAILURE);

        // Read file line by line
        while (fgets(line, 255, fpointer)) {
            //printf("%s\n", line);
            
            // Remove line break at the end of the line
            line[strcspn(line, "\n\r")] = 0;

            // Split line by spaces and get the first word
            char * rest_words = line;
            word = strtok_r(line, delimit, &rest_words);

            // Iterate over all next words in the line
            while( word != NULL ) {

                //printf("New Word: %s\n", word);

                check_word(word, &num_of_words_starting_with_vowel_chars, &num_of_words_ending_with_consonant_chars, &total_num_of_words);

                // Get next word
                word = strtok_r(NULL, delimit, &rest_words);
            }
        }

        fclose(fpointer);

        printf("\nFile Name: %s\n", file_name);
        printf("Total number of words: %d\n", total_num_of_words);
        printf("Number of words starting with a vowel char: %d\n", num_of_words_starting_with_vowel_chars);
        printf("Number of words ending with a consonant char: %d\n", num_of_words_ending_with_consonant_chars);

        // Read next file name
        file_name = strtok_r(NULL, " ", &rest_files);
    }

}

