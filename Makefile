
# A very NAIVE Makefile

CFLAGS=-Iinclude/ -std=c++11 -Wall -pedantic -Werror
LDFLAGS=-lglfw -ldl -lmpv

all:
	mkdir -p build/
	g++ glad/glad.c ${CFLAGS} -c -o build/glad.o
	g++ 3dmpv.cpp ${CFLAGS} -c -o build/3dmpv.o
	g++ -o 3dmpv build/*.o ${LDFLAGS}


clean:
	rm -f build/*.o
	rm -f 3dmpv
