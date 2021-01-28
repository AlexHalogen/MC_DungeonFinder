CC = gcc
LD = gcc

CFLAGS = -lz -g -Wall -std=c99 -c
LDFLAGS = -lz -g -Wall -std=c99  -pthread

UTILS = auto_array.o queue.o callbacks.o
UTILS_LIB = $(patsubst %,utils/%, $(UTILS))

dungeon_finder: dungeon_finder.o cNBT/libnbt.a $(UTILS_LIB)
	$(LD) $^ $(LDFLAGS) -o $@

dungeon_finder.o: dungeon_finder.c
	$(CC) dungeon_finder.c $(CFLAGS)  -o $@

cNBT/libnbt.a: 
	$(MAKE) -C cNBT	libnbt.a

test: test.c $(UTILS_LIB)
	$(CC) $(CFLAGS)   $< -o test.o
	$(LD) $(LDFLAGS) test.o queue.o utils/callbacks.o auto_array.o -o test 
	
clean:
	(rm -f *.o *.a)
	(rm -f dungeon_finder test)
	(rm -f utils/*.o)
	$(MAKE) -C cNBT clean

.PHONY: clean cNBT/libnbt.a
