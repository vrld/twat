# CHANGE THE VOICE HERE
VOICE=cmu_us_kal

# this should be fine
CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -ansi -g -msse -D VOICE=register_$(VOICE) -std=c99
LDFLAGS=-lflite_$(VOICE) -lflite_usenglish -lflite_cmulex -lflite -lm -lalut -lasound

SOURCES=twatc.c 
OBJS=$(SOURCES:.c=.o) ../udp.o ../signal_handler.o
EXEC=../twatc

all: $(SOURCES) $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXEC)
