#include <mpi.h>
#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "prot.h"

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

    if (rank == 0) {
        // get key length
        keyLength = atoi(argv[1]);

        // possible keys
        keyCount = pow(2, keyLength);

        // calculate key range for current process
        fromKey = 0;
        toKey = keyCount / 2;

        // set task state (OMP or CUDA)
        state = OMP;

        // open the encrypted text and known words files
        encryptedTextFile = fopen(argv[2], "r");

        if (!encryptedTextFile) {
            printf("\nCouldn't open file: %s\n", argv[2]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        if (argc > 3) {
            wordsFile = fopen(argv[3], "r");
        } else {
            printf("\nNo known word file was passed - will use default words file.\n");
            wordsFile = fopen("./words.txt", "r");
        }

        if (!wordsFile) {
            if (argc > 3) {
                printf("\nCouldn't open file: %s\n", argv[3]);
            } else {
                printf("\nCouldn't open file: ./words.txt\n");   
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