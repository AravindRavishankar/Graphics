all: sample2D

sample2D: 3d.cpp glad.c
	g++ -o sample2D 3d.cpp glad.c   -lSOIL  -lglfw -lftgl  -lGL -ldl  -I/usr/local/include -I/usr/local/include/freetype2 -L/usr/local/lib

clean:
	rm sample2D
