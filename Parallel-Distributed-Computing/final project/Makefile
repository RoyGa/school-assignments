build:
	mpicxx -fopenmp -c cipher.c -o cipher.o
	mpicxx -fopenmp -c main.c -o main.o
	mpicxx -fopenmp -o project cipher.o main.o 

clean:
	rm -f *.o ./project

run:
	mpiexec -np 2 ./project 8 ciphered.txt words.txt
