#include "prot.h"
// #include "hashmap.h"

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