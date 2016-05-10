CFLAGS=-std=gnu99 -O2 -s
WFLAGS=-Wall -Wextra -pedantic
LFLAGS=-lm
CC=cc

all: password-generator

%: %.c
	${CC} ${WFLAGS} ${CFLAGS} -o $@ $^ ${LFLAGS}

clean:
	rm correcthorsebatterystaple
