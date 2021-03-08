Parallel and Distributed Computing - Final project


Compilling & Running the project -
- The project includes a makefile and other needed files, so can be run by running 'make run'.
- Alternativly:
    use 'mpiexec -np 2 ./project <length> <filename> <words>'
    <length> = The length of the encryption key
    <filename> = The ciphered file path
    <words> = The path to the known words file. This is optional and if not provided the program will use a default words file.

Project description -
1. MPI – is used to divide the program into two processes. Process 0 will fetch the data from the stdin and files and send it to process 1. Each process will try to decipher the ciphered text with half of the optional keys range.
My initial intention was that Process 0 will use OpenMP to parallel the work of checking the first half of optional keys, and Process 1 will use CUDA to check the second half of optional keys. But I didn’t get to do the CUDA part.
2. OpenMP – is used to parallel the work of finding the key by bruteforce. Each process uses 8 threads, each thread checks one key. 
3. CUDA – was not used in this project.

To determine if a key fit, the program checks how many known words exists in the decrypted string. It does that by checking each word of the decrypted string if it exists in the known words hashtable. The key that produced the decrypted string with the most known words will be the output.

The implementation of the hashmap being used in this project is taken from: https://github.com/sheredom/hashmap.h
