#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 

// Function to check if a char is vowel
int check_vowel(char c) {
    if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') {
        return 1;
    } else {
        return 0;
    }
}


char convert_multibyte(wchar_t c) {
    switch (c) {
        case L'à': c=L'a'; break;
        case L'á': c=L'a'; break;
        case L'â': c=L'a'; break;
        case L'ã': c=L'a'; break;
        
        case L'Á': c=L'A'; break;
        case L'À': c=L'A'; break;
        case L'Â': c=L'A'; break;
        case L'Ã': c=L'A'; break;
        
        case L'è': c=L'e';break;
        case L'é': c=L'e';break;
        case L'ê': c=L'e';break;
        
        case L'É': c=L'E';break;
        case L'È': c=L'E';break;
        case L'Ê': c=L'E';break;
        
        case L'í': c=L'i';break;
        case L'ì': c=L'i';break;
        
        case L'Í': c=L'I';break;
        case L'Ì': c=L'I';break;
        
        case L'ó': c=L'o'; break;
        case L'ò': c=L'o'; break;
        case L'ô': c=L'o'; break;
        case L'õ': c=L'o'; break;
        
        case L'Ó': c=L'O'; break;
        case L'Ò': c=L'O'; break;
        case L'Ô': c=L'O'; break;
        case L'Õ': c=L'O'; break;
        
        case L'ú': c=L'u';break;
        case L'ù': c=L'u';break;
        case L'ü': c=L'u';break;
        
        case L'Ú': c=L'U';break;
        case L'Ù': c=L'U';break;
        
        case L'Ç': c=L'C';break;
        case L'ç': c=L'C';break;

        default:    break;
    }
    return c;
}


// Function to check if a word only contains alphanumeric or _ characters 
int check_word(unsigned char * word) {
    printf("%s\n", word);

    for (int i=0; word[i]; i++) {
        wchar_t c = word[i];
        wchar_t byte0 = word[i];
        if (byte0 & (1<<7) && byte0 & (1<<6)) {
            i += 1;

            char byte_one[6]; 
            byte_one[0] = word[i] & (1<<5);
            byte_one[1] = word[i] & (1<<4);
            byte_one[2] = word[i] & (1<<3);
            byte_one[3] = word[i] & (1<<2);
            byte_one[4] = word[i] & (1<<1);
            byte_one[5] = word[i] & (1<<0);


            if (byte0 & (1<<5)) {
                i += 1;
                c += (wchar_t) word[i];
                if (byte0 & (1<<4)) {
                    i += 1;
                    c += (wchar_t) word[i];
                }
            } else {
                char final_char[11];
                final_char[0] = byte0 & (1<<4);
                final_char[1] = byte0 & (1<<3);
                final_char[2] = byte0 & (1<<2);
                final_char[3] = byte0 & (1<<1);
                final_char[4] = byte0 & (1<<0);

                strcat(final_char, byte_one);
            }
            
            printf("sou multibyte");
            if (c == L'É') {
                printf("entrei qui scrlll");
            }
            c = convert_multibyte(c);
            printf("%c\n", c);
        } 
        printf("%i\n",c);
        if (isalnum(word[i]) == 0 && word[i] != '_' ) {
            return 0;
        }
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

                if (check_word(word)) {

                    total_num_of_words += 1;

                    // Get first and last char of the word and convert to lowercase in order to facilitate the if conditions
                    char first_char = tolower(*word);
                    char last_char = tolower(*(word + strlen(word) - 1));
                    printf("%c\n",first_char);
                    // Check if word starts with a vowel
                    if (check_vowel(first_char)) {
                        num_of_words_starting_with_vowel_chars += 1;
                    } 
                    
                    // Check if word ends with a consonant
                    if (check_vowel(last_char) == 0 && last_char >= 97 && last_char <= 122 ) {
                        num_of_words_ending_with_consonant_chars += 1;
                    }

                }
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

