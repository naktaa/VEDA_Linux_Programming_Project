.SUFFIXES:.c.o

OBJS = main.o init.o server.o event.o globals.o
FILES = main
SRCS = $(OBJS:.o=.c)

CC = gcc
INC = -I include
LFLAGS = -lwiringPi -lpthread -ldl
RFLAGS = -rdynamic
.PHONY : lib clean  # 디렉토리와 이름이 겹칠 때 사용

all: lib main

lib:
	cd lib ; $(MAKE)

main: ${OBJS}
	$(CC) -o $@ $(OBJS) $(LFLAGS) $(RFLAGS) 
	
.c.o:
	$(CC) $(INC) -c -g $<

clean:
	rm $(FILES) $(OBJS)
	cd lib ; $(MAKE) clean
	
# gcc -o total total.c -lwiringPi -lpthread -ldl -rdynamic 