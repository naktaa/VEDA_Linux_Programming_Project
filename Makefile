.SUFFIXES:.c.o

MUSIC_SRCS = $(wildcard music/*.c)
MUSIC_OBJS = $(MUSIC_SRCS:.c=.o)
SERVER_OBJS = main.o init.o server.o event.o globals.o $(MUSIC_OBJS)
CLIENT_OBJS = client.o

FILES = main client
SRCS = $(SERVER_OBJS:.o=.c) $(CLIENT_OBJS:.o=.c)

SERVERCC = aarch64-linux-gnu-gcc
CLIENTCC = gcc
INC = -I include
LFLAGS = -lwiringPi -lpthread -ldl
RFLAGS = -rdynamic
.PHONY : lib clean  # 디렉토리와 이름이 겹칠 때 사용

all: lib main client

lib:
	cd lib ; $(MAKE)

main: $(SERVER_OBJS)
	$(SERVERCC) -o $@ $^ $(LFLAGS) $(RFLAGS) 
	
client: $(CLIENT_OBJS)
	$(CLIENTCC) -o $@ $^
	
.c.o:
	$(SERVERCC) $(INC) -c -g -o $@ $<

client.o: client.c
	$(CLIENTCC) $(INC) -c -g -o $@ $<

# install: 
# 	scp main pi@192.168.0.29:/home/pi/ttest/
# 	scp ./lib/lib*.so pi@192.168.0.29:/home/pi/ttest/lib/

clean:
	rm $(FILES) $(SERVER_OBJS) $(CLIENT_OBJS)
	cd lib ; $(MAKE) clean
	
# gcc -o total total.c -lwiringPi -lpthread -ldl -rdynamic 