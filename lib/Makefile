TARGET=libnet.a

CURRENTPWD=$(shell pwd)
BASELIB=$(CURRENTPWD)
TOOLSPWD=$(CURRENTPWD)/

CXXFLAGS += -DDEBUG
CXXFLAGS += -O0 -g -Wall
CXXFLAGS += -std=c++11
CXXFLAGS +=-I$(TOOLSPWD)

CC=g++
AR=ar

SOURCES=$(CURRENTPWD)/io_service.o  \
		$(CURRENTPWD)/mc_tcp.o  \

LIB=-lpthread 

$(TARGET):$(SOURCES)
	$(CC) -c $(CXXFLAGS) $(LIB) $(LIBDIR)
	$(AR) rcs $(TARGET) $(SOURCES)


clean:
	rm $(SOURCES) $(TARGET) -f
