TARGET=client

CURRENTPWD=$(shell pwd)
BASELIB=$(CURRENTPWD)
TOOLSPWD=$(CURRENTPWD)

CXXFLAGS += -DDEBUG
CXXFLAGS += -O0 -g -Wall
CXXFLAGS += -std=c++11
CXXFLAGS +=-I$(TOOLSPWD)/../../lib

CC=g++

SOURCES=$(CURRENTPWD)/main.o  \

LIBDIR=-L$(CURRENTPWD)/../../lib
LIB=-lpthread  -lnet

$(TARGET):$(SOURCES)
	$(CC) -o $@ $^ $(CXXFLAGS) $(INCLUDE) $(LIB) $(LIBDIR)


clean:
	rm $(SOURCES) $(TARGET) -f
