CXXFLAGS = -std=c++11 -O3 -fopenmp
CCFLAGS = -std=c++11 -O3 -fopenmp
CXX = g++
CC = g++

all: main

main: main.o data.o
	g++ $^ -o main -O3 -fopenmp

clean:
	rm -f *.o main