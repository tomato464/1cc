CFLAGS=-std=c11 -g -static -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

1cc: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): 1cc.h

test: 1cc
	./1cc tests/tests.c > tmp.s
	gcc -static -o tmp tmp.s
	./tmp

clean:
	rm -rf 1cc *.o *~tmp* tests/*~ tests/*.o

.PHONY: test clean
