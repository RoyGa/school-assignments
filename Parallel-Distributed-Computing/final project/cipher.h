#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define START_SIZE 512
#define EXTEND_SIZE 32
#define MAX_KEY_SIZE 4

char *readStringFromFile(FILE *fp, size_t allocated_size, int *input_length);
void binaryStringToBinary(char *string, size_t num_bytes);
void cipher(char *key, size_t key_len, FILE *input, FILE *output);
void printHelp(char *argv);
char* decipherString(char *key, size_t key_len, char *input, int inputLength);