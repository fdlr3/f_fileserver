#declare variables
CC=gcc
CFLAGS=-c -Wall

a.out: obj/server.o obj/logger.o obj/file_manager.o obj/main.o
	$(CC) obj/server.o obj/logger.o obj/file_manager.o obj/main.o -o a.out

obj/main.o: main.c
	$(CC) $(CFLAGS) main.c -o obj/main.o

obj/logger.o: src/logger.c
	$(CC) $(CFLAGS) src/logger.c -o obj/logger.o

obj/file_manager.o: src/file_manager.c
	$(CC) $(CFLAGS) src/file_manager.c -o obj/file_manager.o

obj/server.o: src/server.c
	$(CC) $(CFLAGS) src/server.c -o obj/server.o

clean:
	rm obj/*.o output
