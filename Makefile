CXX=g++
CXXFLAGS+=-Wall -Wextra -Wno-unused-parameter -I./CImg-2.0.3_pre080217 -g -ggdb
LDFLAGS+=-L/usr/X11R6/lib -lm -lpthread -lX11

.PHONY:		all clean


all:	mapv lopmap

mapv:	mapv.o Transmitters.o
	g++ $(CFLAGS) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

lopmap:	lopmap.o Transmitters.o geo_distance.o
	g++ $(CFLAGS) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	-rm -rf mapv lopmap mapv.o lopmap.o Transmitters.o geo_distance.o


