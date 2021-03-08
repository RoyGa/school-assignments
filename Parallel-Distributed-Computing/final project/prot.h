#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "hashmap.h"

#include <mpi.h>
#include <omp.h>

#define START_SIZE 512
#define EXTEND_SIZE 32
#define MAX_KEY_SIZE 4

typedef enum {
    OMP,
    CUDA
} State;

typedef struct data {
    char *text;
    char *key;
    int match;
} Data;

char *readStringFromFile(FILE *fp, size_t allocated_size, int *input_length);
void binaryStringToBinary(char *string, size_t num_bytes);
char* decipherString(char *key, size_t key_len, char *input, int inputLength);

//
int calcLength(int num);
char* decToBinary(int n, int keyLength);

struct hashmap_s createKnownWordsHashmap(char* knownWords);
Data decryptText(char *encryptedText, int encryptedTextLength, int keyLength, struct hashmap_s words, int from, int to, State state);
int countKnownWords(char* text, struct hashmap_s wordsHashmap);

Data OMPWork(char *encryptedText, int encryptedTextLength, int keyLength ,struct hashmap_s words, int from, int to);
