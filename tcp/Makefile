CC = g++

build: clean main.o
	$(CC) main.o -o main

main.o: main.cpp
	$(CC) -c main.cpp

clean:
	-rm -rf *.o
	-rm -rf main