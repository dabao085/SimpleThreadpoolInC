CFLAGS = -g -Isrc
LDLIBS = -lpthread
TARGETS = tests/testThreadpool tests/testShutdown tests/testPress lib/libthreadpool.so lib/libthreadpool.a
all: ${TARGETS}

#make自动推导
tests/testThreadpool: src/threadpool.o tests/testThreadpool.o
tests/testShutdown: src/threadpool.o tests/testShutdown.o
tests/testPress: src/threadpool.o tests/testPress.o 
src/threadpool.o: src/threadpool.c src/threadpool.h
tests/testThreadpool.o: src/threadpool.c tests/testThreadpool.c
tests/testShutdown.o: src/threadpool.c tests/testShutdown.c
test/testPress.o: src/threadpool.c tests/testPress.c

#
shared: libthreadpool.so
static: libthreadpool.a

#
lib/libthreadpool.so: src/threadpool.c src/threadpool.h
	gcc -shared -fPIC ${CFLAGS} -o $@ $< ${LDLIBS}

src/libthreadpool.o: src/threadpool.c src/threadpool.h	
	gcc -c ${CFLAGS} -o $@ $<

lib/libthreadpool.a: src/libthreadpool.o
	ar rcs $@ $^
#
#threadpool.o: threadpool.c threadpool.h
#	gcc -c $< ${LDLIBS} ${CFLAGS}

#testThreadpool.o: testThreadpool.c
#	gcc -c $< ${LDLIBS} ${CFLAGS}

#testShutdown.o: testShutdown.c
#	gcc -c $< ${LDLIBS} ${CFLAGS}	

#testPress.o: testPress.c
#	gcc -c $< ${LDLIBS} ${CFLAGS}	

clean:
	rm -f ${TARGETS} */*.o