CFLAGS = -std=c99
LDLIBS = -lpthread
TARGETS = threadpool.o testThreadpool.o testThreadpool
all: testThreadpool

testThreadpool: threadpool.o testThreadpool.o

#
threadpool.o: threadpool.c threadpool.h
	gcc -c $< ${LDLIBS} ${CFLAGS}

testThreadpool.o: testThreadpool.c
	gcc -c $< ${LDLIBS} ${CFLAGS}

clean:
	rm -f ${TARGETS}