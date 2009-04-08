CC=g++ -g
CFLAGS=-c -Wall

all: awnw

awnw: awnw.o perlin.o terramap.o textures.o map.o
	$(CC) awnw.o perlin.o terramap.o textures.o map.o -lSDL -lGL -lGLU -lGLEW -o awnw

awnw.o: awnw.cpp terramap.h perlin.h textures.h
	$(CC) $(CFLAGS) awnw.cpp

perlin.o: perlin.cpp perlin.h
	$(CC) $(CFLAGS) perlin.cpp

terramap.o: terramap.cpp terramap.h textures.h
	$(CC) $(CFLAGS) terramap.cpp

textures.o: textures.cpp textures.h perlin.h map.h
	$(CC) $(CFLAGS) textures.cpp

map.o: map.cpp map.h
	$(CC) $(CFLAGS) map.cpp

clean:
	rm -rf *.o awnw
