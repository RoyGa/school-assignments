#include <mpi.h>
#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "prot.h"
#include "hashmap.h"

// returns an hashmap populated with the known words
struct hashmap_s createKnownWordsHashmap(char* knownWords) {
    const unsigned initial_size = 2;
    struct hashmap_s hashmap;
    char* word;
    int placeholder = 0;

    // init hashmap for known words
    if (0 != hashmap_create(initial_size, &hashmap)) {
        printf("Failed to create hashmap.");
    }

    // put known words into the hashmap
    while ((word = strsep(&knownWords, "\n")) != NULL) {
        if (0 != hashmap_put(&hashmap, word, strlen(word), &placeholder)) {
            printf("Failed to put element into hashmap.");
        }
    }

    return hashmap;
}

Data decryptText(char *encryptedText, int encryptedTextLength, int keyLength, struct hashmap_s words, int from, int to, State state) {
    if (state == OMP) {
        return OMPWork(encryptedText, encryptedTextLength, keyLength, words, from, to);
    } else if (state == CUDA) {}
}

int countKnownWords(char* text, struct hashmap_s wordsHashmap) {
    int counter = 0;
    char* word;

    // solution for strsep overwrite problem - copy words to a temp string
    char* textBuffer = (char*)malloc(sizeof(char) * strlen(text));
    strcpy(textBuffer, text);

    // iterate over deciphered text words
    while ((word = strsep(&textBuffer, " ")) != NULL) {
        // check if current deciphered word exists in known words hashmap
        if (hashmap_get(&wordsHashmap, word, strlen(word)) != NULL) {
            counter++;
        }
    }

    free(textBuffer);

    return counter;
}