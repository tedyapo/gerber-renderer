binaries = gerb3d
objects = Material.o Process.o Board.o BumpMap.o MaterialMap.o zpr.o json_parsing.o

all : ${binaries} ${objects}

CFLAGS = -Wall -fopenmp -std=c++0x -O3

vector_test: vector_test.cpp VectorN.hpp
	g++ ${CFLAGS} -o $@ $<

Material.o : Material.cpp Material.hpp
	g++ ${CFLAGS} -c $< -o $@

Process.o : Process.cpp Process.hpp Material.hpp
	g++ ${CFLAGS} -c $< -o $@

BumpMap.o : BumpMap.cpp BumpMap.hpp
	g++ ${CFLAGS} `pkg-config --cflags libgerbv` -c $< -o $@

MaterialMap.o : MaterialMap.cpp MaterialMap.hpp
	g++ ${CFLAGS} `pkg-config --cflags libgerbv` -c $< -o $@

Board.o : Board.cpp Board.hpp Process.hpp Material.hpp
	g++ ${CFLAGS} `pkg-config --cflags libgerbv` -c $< -o $@

json_parsing.o : json_parsing.cpp Board.o
	g++ ${CFLAGS} `pkg-config --cflags libgerbv` -c $^ -o $@ Board.o

zpr.o: zpr.c
	gcc -c -o $@ $^

gerb3d: gerb3d.cpp zpr.o Material.o Process.o json_parsing.o BumpMap.o MaterialMap.o Board.o
	g++ $(CFLAGS)  -o $@ $^ `pkg-config --cflags libgerbv` `pkg-config --libs libgerbv` -lGL -lGLU -lglut -lpthread -ljson-c

clean:
	rm -f *~ ${binaries} ${objects}

