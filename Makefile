CC=musl-gcc
OUT = avl

OBJS = avl.o main.o avl_alloc.o
CFLAGS = --std=c89 -Og -g -Wall -Werror -Wpedantic -fanalyzer
LDFLAGS = -g

avl: $(OBJS)
	$(CC) -o $(OUT) $(LDFLAGS) $^

clean:
	rm -f $(OUT) $(OBJS)
