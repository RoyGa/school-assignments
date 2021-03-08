build:
	mpicxx -fopenmp -c cipher.c -o cipher.o
	mpicxx -fopenmp -c general_functions.c -o general_functions.o
	mpicxx -fopenmp -c OMPWork.c -o OMPWork.o
	mpicxx -fopenmp -c main.c -o main.o
	mpicxx -fopenmp -o project cipher.o general_functions.o OMPWork.o main.o 

clean:
	rm -f *.o ./project

run:
	mpiexec -np 2 ./project 8 ciphered.txt 
