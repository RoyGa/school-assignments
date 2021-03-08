#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "prot.h"

char *readStringFromFile(FILE *fp, size_t allocated_size, int *input_length)
{
    char *string;
    int ch;
    *input_length = 0;
    string = (char*)realloc(NULL, sizeof(char) * allocated_size);
    if (!string)
        return string;
    while (EOF != (ch = fgetc(fp)))
    {
        if (ch == EOF)
            break;
        
        string[*input_length] = ch;
        *input_length += 1;

        if (*input_length == allocated_size)
        {
            string = (char*)realloc(string, sizeof(char) * (allocated_size += EXTEND_SIZE));
            if (!string)
                return string;
        }
    }
    return (char*)realloc(string, sizeof(char) * (*input_length));
}

void binaryStringToBinary(char *string, size_t num_bytes)
{
    int i,byte;
    unsigned char binary_key[MAX_KEY_SIZE];
    for(byte = 0;byte<num_bytes;byte++)
    {
        binary_key[byte] = 0;
        for(i=0;i<8;i++)
        {
            binary_key[byte] = binary_key[byte] << 1;
            binary_key[byte] |= string[byte*8 + i] == '1' ? 1 : 0;  
        }
    }
    memcpy(string,binary_key,num_bytes);
}

char* decipherString(char *key, size_t key_len, char *input, int inputLength)
{
    int i, j = 0;
    char *output_str = (char*)malloc(inputLength * sizeof(char));
    if (!output_str)
    {
        fprintf(stderr, "Error reading string\n");
        exit(0);
    }
    for (i = 0; i < inputLength; i++, j++)
    {
        if (j == key_len)
            j = 0;
        output_str[i] = input[i] ^ key[j];
    }
    return output_str;
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