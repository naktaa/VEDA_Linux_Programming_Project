.SUFFIXES:.c.o

OBJS = main.o
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

main: main.o
	$(CC) -o $@ $@.o $(LFLAGS) $(RFLAGS) 
	
.c.o:
	$(CC) -c -g $<

clean:
	rm $(FILES) *.o
	cd lib ; $(MAKE) clean
	
# gcc -o total total.c -lwiringPi -lpthread -ldl -rdynamic 