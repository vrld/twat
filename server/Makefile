CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -ansi -g -msse -std=c99
LDFLAGS=-lm -lcurl -lpthread

SOURCES=twats.c cJSON.c queue.c twitter.c stringutils.c
OBJS=$(SOURCES:.c=.o) ../udp.o ../signal_handler.o
EXEC=../twats

all: $(SOURCES) $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXEC)
