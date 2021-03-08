#include <mpi.h>
#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "cipher.h"
#include "hashmap.h"

typedef enum {
    OMP,
    CUDA
} State;

typedef struct data {
    char *text;
    char *key;
    int match;
} Data;

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

int calcLength(int num) {
    int length = 0;

    while (num != 0) {
        num = num / 2;
        length++;
    }

    return length;
}

// converts decimal number to its binary string representation
char* decToBinary(int n, int keyLength) { 
    int length = calcLength(n);
    char* binaryKeyStr = (char*)malloc((length + 1) * sizeof(char));
    int i = 0;

    while (n > 0) { 
        binaryKeyStr[length - 1 - i] = n % 2 + 48; 
        n = n / 2; 
        i++; 
    }

    binaryKeyStr[length] = '\0';
    return binaryKeyStr;
}

Data OMPWork(char *encryptedText, int encryptedTextLength, int keyLength ,struct hashmap_s words, int from, int to) {
    Data res;

    // allocate memory for result data
    res.key = (char*)malloc(keyLength * sizeof(char));
    res.text = (char*)malloc(encryptedTextLength * sizeof(char));
    res.match = 0;

    // set the number of threads to be used
    omp_set_num_threads(4);

    // iterate over the keys in (from, to) range
    #pragma omp parallel for
    for (int key = from; key < to; key++) {
        // convert decimal number to its binary string representation
        char* keyStr = decToBinary(key, keyLength);

        // convert the string (representing the binary key) to binary
        binaryStringToBinary(keyStr, sizeof(keyStr));

        // try to decipher the text using the current key
        char *decipheredText = decipherString(keyStr, keyLength/8, encryptedText, encryptedTextLength);

        // count how many known words the deciphered text contains
        int count = countKnownWords(decipheredText, words);

        // if current deciphered text contains more known words than other
        // texts deciphered so far - update the results
        if (count > res.match) {
            #pragma omp critical
            {
                res.key = decToBinary(key, keyLength);
                res.text = decipheredText;
                // strcpy(res.text, decipheredText);
                res.match = count;
            }
        }
    }
    return res;
}

Data decryptText(char *encryptedText, int encryptedTextLength, int keyLength, struct hashmap_s words, int from, int to, State state) {
    if (state == OMP) {
        return OMPWork(encryptedText, encryptedTextLength, keyLength, words, from, to);
    } else if (state == CUDA) {}
}

int main(int argc, char *argv[]) {
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Status mpi_status;

    int keyLength, encryptedTextLength, wordsLength;
    FILE *encryptedTextFile, *wordsFile;
    char *encryptedText, *words;

    int keyCount, fromKey, toKey;
    State state;
    struct hashmap_s wordsHashmap;

    // rank 0 will be the main process:
    //      - get length of the key
    //      - open ciphered file
    //      - open words file
    //      - read the ciphered.txt file into a string (?)
    //      - read the words.txt file into a string array (?)
    //      - send key length
    //      - send ciphered string length
    //      - send words length
    //      - send the ciphered string
    //      - send known words array

    // other rank will be the child processes:
    //      - recieve key length
    //      - recieve ciphered string length
    //      - recieve words length
    //      - recieve the ciphered string
    //      - recieve known words array

    if (rank == 0) {
        // get key length
        keyLength = atoi(argv[1]);

        // 
        keyCount = pow(2, keyLength);

        // calculate key range for current process
        fromKey = 0;
        toKey = keyCount / 2;

        // set task state (OMP or CUDA)
        state = OMP;

        // open the encrypted text and known words files
        encryptedTextFile = fopen(argv[2], "r");

        if (!encryptedTextFile) {
            printf("Couldn't open file: %s", argv[2]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        if (argc > 3) {
            wordsFile = fopen(argv[3], "r");
        } else {
            printf("No known word file given - will use default words file.");
            wordsFile = fopen("./words.txt", "r");
        }

        wordsFile = fopen(filename, "r");

        if (!wordsFile) {
            if (argc > 3) {
                printf("Couldn't open file: %s", argv[3]);
            } else {
                printf("Couldn't open file: ./words.txt");   
            }
        }

        // read files into strings
        encryptedText = readStringFromFile(encryptedTextFile, START_SIZE, &encryptedTextLength);
        words = readStringFromFile(wordsFile, START_SIZE, &wordsLength);

        // send data to other process
        MPI_Send(&keyLength, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(&encryptedTextLength, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(&wordsLength, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(encryptedText, encryptedTextLength, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
		MPI_Send(words, wordsLength, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
        MPI_Send(&keyCount, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    } else {
        // recieve lengths data from main process
        MPI_Recv(&keyLength, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &mpi_status);
        MPI_Recv(&encryptedTextLength, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &mpi_status);
        MPI_Recv(&wordsLength, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &mpi_status);

        // allocate memory according to recieved lengths
        encryptedText = (char*)malloc(encryptedTextLength * sizeof(char));
        words = (char*)malloc(wordsLength * sizeof(char));
        
        // recieve encrypted text and known words from main process
        MPI_Recv(encryptedText, encryptedTextLength, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &mpi_status);
        MPI_Recv(words, wordsLength, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &mpi_status);

        MPI_Recv(&keyCount, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &mpi_status);

        // calculate key range for current process
        fromKey = keyCount / 2 + 1;
        toKey = keyCount;

        // set task state (OMP or CUDA)
        state = OMP;
    }

    // generate hashmap of known words
    wordsHashmap = createKnownWordsHashmap(words);

    // try to decrypt the ciphered text
    Data resData = decryptText(encryptedText, encryptedTextLength, keyLength, wordsHashmap, fromKey, toKey, state);
    
    if (rank == 0) {
        int match;
        char* key;
        char* text;

        // allocate memory for results of the other process
        key = (char*)malloc(sizeof(char) * keyLength);
        text = (char*)malloc(sizeof(char) * encryptedTextLength);

        // recieve result data from other process
        MPI_Recv(&match, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &mpi_status);
        MPI_Recv(key, keyLength, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &mpi_status);
        MPI_Recv(text, encryptedTextLength, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &mpi_status);
        
        // compare this process results to the results of the other process 
        if (resData.match > match) {// this process found a better matching key
            printf("Deciphered text:\n%s\n Key: \n%s \n", resData.text, resData.key);
        } else {// the other process found a better matching key
            printf("Deciphered text:\n%s\nKey:\n%s \n", text, key);
        }

        free(key);
        free(text);
    } else {
        // send result data to the other process
        MPI_Send(&resData.match, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(resData.key, keyLength, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
        MPI_Send(resData.text, encryptedTextLength, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }

    free(words);
    free(encryptedText);
    hashmap_destroy(&wordsHashmap);

    MPI_Finalize();

    return 0;
}