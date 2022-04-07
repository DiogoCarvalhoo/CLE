# CLE

# Assignment I - Program 1

92969 - Diogo Carvalho

93367 - Rafael Baptista


## How to compile

```
gcc -Wall -O3 -o count_words count_words.c chunks.c counters.c auxiliar_functions.c -lpthread -lm
```

## How to run

```
./count_words 4 text0.txt text1.txt text2.txt text3.txt text4.txt
```

```
Arguments:
The first argument should be the number of threads.
The following arguments are the text files to be processed.
```
