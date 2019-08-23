TOPDIR=../../../..


CXXFLAGS+= -Wl,--no-whole-archive   -pthread -fPIC
 
TARGET="libnet.a"


LIBS=
#CXXFLAGS += -DDEBUG
CXXFLAGS += -o0 -g -Wall
CXXFLAGS += -std=c++11

SOURCES=io_service.o \
	mc_tcp.o	\


CC=g++
AR=ar
all:$(SOURCES)
	$(AR) rcs $(TARGET) *.o 
	

.PHONY: clean
clean:
	rm -f  ${TARGET} $(SOURCES) 
