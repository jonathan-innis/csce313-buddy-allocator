# makefile

all: memtest

Ackerman.o: Ackerman.cpp 
	g++ -c -g Ackerman.cpp -std=c++11

BuddyAllocator.o : BuddyAllocator.cpp
	g++ -c -g BuddyAllocator.cpp -std=c++11

Main.o : Main.cpp
	g++ -c -g Main.cpp -std=c++11

memtest: Main.o Ackerman.o BuddyAllocator.o
	g++ -o memtest Main.o Ackerman.o BuddyAllocator.o -std=c++11

clean:
	rm *.o
