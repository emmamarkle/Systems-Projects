CC          = gcc
DEBUG_FLAGS = -ggdb -Wall
CFLAGS      = `pkg-config fuse --cflags --libs` $(DEBUG_FLAGS)

all: mirrorfs caesarfs versfs

mirrorfs: mirrorfs.c
	$(CC) mirrorfs.c $(CFLAGS) -o mirrorfs

caesarfs: caesarfs.c
	$(CC) caesarfs.c $(CFLAGS) -o caesarfs

versfs: versfs.c
	$(CC) versfs.c $(CFLAGS) -o versfs

clean:
	rm -f mirrorfs caesarfs versfs
