CC=g++
CFLAGS=-c -Wall

all: awnw

awnw: awnw.o perlin.o terramap.o textures.o
	$(CC) awnw.o perlin.o terramap.o textures.o -lSDL -lGLU -o awnw

awnw.o: awnw.cpp terramap.h perlin.h textures.h
	$(CC) $(CFLAGS) awnw.cpp

perlin.o: perlin.cpp perlin.h
	$(CC) $(CFLAGS) perlin.cpp

terramap.o: terramap.cpp terramap.h
	$(CC) $(CFLAGS) terramap.cpp

textures.o: textures.cpp textures.h perlin.h
	$(CC) $(CFLAGS) textures.cpp

clean:
	rm -rf *.o awnw
