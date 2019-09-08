#declare variables
CC=gcc
CFLAGS=-c -Wall -Werror

jernej: 
	$(CC) -g -D JERNEJ_LINUX src/server.c src/logger.c src/file_manager.c \
	src/config_reader.c main.c  -o a.out

droplet: 
	$(CC) -g -D DROPLET_LINUX src/server.c src/logger.c src/file_manager.c \
	src/config_reader.c main.c  -o a.out

duler: 
	$(CC) -g -D FILIP_LINUX src/server.c src/logger.c src/file_manager.c \
	src/config_reader.c main.c  -o a.out

a.out: obj/main.o obj/server.o obj/logger.o obj/file_manager.o obj/config_reader.o
	$(CC) obj/server.o obj/logger.o obj/file_manager.o \
	obj/config_reader.o obj/main.o -o a.out

obj/main.o: main.c
	$(CC) $(CFLAGS) main.c -o obj/main.o

obj/logger.o: src/logger.c
	$(CC) $(CFLAGS) src/logger.c -o obj/logger.o

obj/file_manager.o: src/file_manager.c
	$(CC) $(CFLAGS) src/file_manager.c -o obj/file_manager.o

obj/server.o: src/server.c
	$(CC) $(CFLAGS) src/server.c -o obj/server.o

obj/config_reader.o: src/config_reader.c
	$(CC) $(CFLAGS) src/config_reader.c -o obj/config_reader.o

clean:
	rm obj/*.o output
