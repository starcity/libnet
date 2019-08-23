
TARGET=client

CURRENTPWD=$(shell pwd)

BASELIB=$(CURRENTPWD)

TOOLSPWD=$(CURRENTPWD)

CXXFLAGS +=-DDEBUG
CXXFLAGS +=-O0 -g -Wall
CXXFLAGS +=-std=c++11
CXXFLAGS +=-I$(CURRENTPWD)/../../lib
CXXFLAGS +=-L$(CURRENTPWD)/../../lib

CC=g++


SOURCES=$(CURRENTPWD)/client.o  \
		$(CURRENTPWD)/main.o  \

#LIBDIR=-L$(CURRENTPWD)/../lib
LIB=-lpthread -lnet

$(TARGET):$(SOURCES)
	$(CC) -o $@ $^ $(CXXFLAGS) $(INCLUDE) $(LIB)


clean:
	rm $(SOURCES) $(TARGET) -f
