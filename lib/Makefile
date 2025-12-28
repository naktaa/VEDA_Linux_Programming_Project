.SUFFIXES:.c.o

all: libbuzzer.so libcds.so libseven.so

OBJS = buzzer_thread.o cds_thread.o seven_thread.o
OBJS1 = buzzer_thread.o
OBJS2 = cds_thread.o
OBJS3 = seven_thread.o

FILES = libbuzzer.so libcds.so libseven.so

SRCS = $(OBJS:.o=.c)
CC = gcc
# CC = aarch64-linux-gnu-gcc  # cross comfile
INC = -I ../include
CFLAGS = -c -fPIC
OFLAGS = -shared -o
LDFLAGS = -lwiringPi -lpthread

libbuzzer.so: $(OBJS1)
	$(CC) $(OFLAGS) $@ $(OBJS1) $(LDFLAGS)
	
libcds.so: $(OBJS2)
	$(CC) $(OFLAGS) $@ $(OBJS2) $(LDFLAGS)
	
libseven.so: $(OBJS3)
	$(CC) $(OFLAGS) $@ $(OBJS3) $(LDFLAGS)
	
.c.o:
	$(CC) $(INC) $(CFLAGS) $<
	
clean:
	rm *.o $(FILES)
	
# gcc -c -fPIC ***.c
# gcc -shared -o %%%.so ***.o
# gcc -o total total.c -lwiringPi -lpthread -ldl -rdynamic 