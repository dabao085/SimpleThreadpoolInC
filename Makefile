CFLAGS = -g
LDLIBS = -lpthread
TARGETS = threadpool.o testThreadpool.o testShutdown.o testPress.o testThreadpool testShutdown testPress
all: testThreadpool testShutdown testPress

testThreadpool: threadpool.o testThreadpool.o
testShutdown: threadpool.o testShutdown.o
testPress: threadpool.o testPress.o 
#
threadpool.o: threadpool.c threadpool.h
	gcc -c $< ${LDLIBS} ${CFLAGS}

testThreadpool.o: testThreadpool.c
	gcc -c $< ${LDLIBS} ${CFLAGS}

testShutdown.o: testShutdown.c
	gcc -c $< ${LDLIBS} ${CFLAGS}	

testPress.o: testPress.c
	gcc -c $< ${LDLIBS} ${CFLAGS}	

clean:
	rm -f ${TARGETS}