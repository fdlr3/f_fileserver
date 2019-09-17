#DEBUG => "make BUILD=debug" || RELEASE => "make"
#declare variables
ifeq ($(BUILD),debug)   
#Debug build
CFLAGS += -O0 -c -Wall -Werror
else
#Release build
CFLAGS += -O3 -c
endif

CC=gcc

a.out: obj/main.o obj/server.o obj/logger.o obj/file_manager.o obj/config_reader.o
	$(CC) obj/server.o obj/logger.o obj/file_manager.o \
	obj/config_reader.o obj/main.o -o a.out

obj/config_reader.o: src/config_reader.c
	$(CC) $(CFLAGS) src/config_reader.c -o obj/config_reader.o

obj/server.o: src/server.c
	$(CC) $(CFLAGS) src/server.c -o obj/server.o

obj/file_manager.o: src/file_manager.c
	$(CC) $(CFLAGS) src/file_manager.c -o obj/file_manager.o

obj/logger.o: src/logger.c
	$(CC) $(CFLAGS) src/logger.c -o obj/logger.o


obj/main.o: main.c
	$(CC) $(CFLAGS) main.c -o obj/main.o


clean:
	rm obj/*.o a.out
