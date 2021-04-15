
# A very NAIVE Makefile

CFLAGS=-g -Iinclude/ -std=c++11 -Wall -pedantic -Werror #-DDEBUG
LDFLAGS= -lglfw -ldl -lmpv -lpthread

all:
	mkdir -p build/
	g++ glad/glad.c ${CFLAGS} -c -o build/glad.o
	g++ 3dmpv.cpp ${CFLAGS} -c -o build/3dmpv.o
	g++ -o 3dmpv build/*.o ${LDFLAGS}


clean:
	rm -f build/*.o
	rm -f 3dmpv
