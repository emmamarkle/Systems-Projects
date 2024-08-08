CC          = gcc
DEBUG_FLAGS = -ggdb -Wall
CFLAGS      = -std=gnu99 $(DEBUG_FLAGS)

all: cache-sim 

cache.o: cache.c
	$(CC) $(CFLAGS) -c cache.c

cache-sim: cache.o 
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o cache-sim simulator.c -lm


clean:
	rm -rf *.o cache-sim
