CC = gcc
LD = gcc
AR = ar 

CFLAGS = -lz -g -Wall -std=c99 -c
LDFLAGS = -lz -g -Wall -std=c99  -pthread
ARFLAGS = -rcs
CNBTLIB_CFLAGS=-g -Wall -Wextra -std=c99 -pedantic -fPIC -lz -c

CNBTSRC = cNBT/buffer.c cNBT/nbt_loading.c cNBT/nbt_parsing.c cNBT/nbt_treeops.c cNBT/nbt_util.c
CNBTLIB = $(patsubst %.c, %.o, $(CNBTSRC) )


dungeon_finder: dungeon_finder.o libNBT.a queue.o auto_array.o utils/callbacks.o
	$(LD) $^ $(LDFLAGS) -o $@


dungeon_finder.o: dungeon_finder.c
	$(CC) dungeon_finder.c $(CFLAGS)  -o $@


libNBT.a: $(CNBTSRC)
	$(foreach sss,$(CNBTSRC),$(CC) $(CNBTLIB_CFLAGS) $(sss) -o $(patsubst %.c,%.o,$(sss));  )
	$(AR) $(ARFLAGS) $@ $(CNBTLIB)

queue.o: queue.c
	$(CC) $(CFLAGS)  $^ -o $@

#vector.o: vector.c
#	$(CC) $(CFLAGS) $^ -o $@

auto_array.o: auto_array.c
	$(CC) $(CFLAGS) $^ -o $@

utils/callbacks.o: utils/callbacks.h
	$(CC) $(CFLAGS) $^ -o $@

test: test.c queue.o auto_array.o utils/callbacks.o
	$(CC) $(CFLAGS)   $< -o test.o
	$(LD) $(LDFLAGS) test.o queue.o utils/callbacks.o auto_array.o -o test 
	
clean:
	(rm -f *.o *.a)
	(rm -f dungeon_finder test)

.PHONY: clean
