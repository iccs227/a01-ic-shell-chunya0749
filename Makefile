CC = gcc
CFLAGS = -Wall -g
BINARY = icsh
OBJS = icsh.o job.o signal.o

all: $(BINARY)

$(BINARY): $(OBJS)
	$(CC) $(CFLAGS) -o $(BINARY) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(BINARY) *.o
