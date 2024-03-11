OUT = avl

OBJS = avl.o main.o
CFLAGS = -g -Wall -Werror -Wpedantic -fanalyzer
LDFLAGS = -g

avl: $(OBJS)
	$(CC) -o $(OUT) $(LDFLAGS) $^

clean:
	rm -f $(OUT) $(OBJS)
