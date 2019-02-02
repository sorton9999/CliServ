TARGET  = Client
SRCS    += \
	Client.cpp
CXX      = g++
OBJS    = ${SRCS:.cpp=.o} 
DEPS    = ${SRCS:.cpp=.dep}
XDEPS   = $(wildcard ${DEPS}) 

CXXFLAGS += -Wall -Wreorder -std=c++11 -pedantic -g -fPIC
CXXINCLUDES += -I. -I../lib/ServiceIF
LDFLAGS += -L../lib/ServiceIF
LIBS    += -lpthread -lServiceIF

.PHONY: all clean distclean 
all:: ${TARGET} 

ifneq (${XDEPS},) 
include ${XDEPS} 
endif

${TARGET}: ${OBJS} 
	${CXX} ${LDFLAGS} -o $@ $^ ${LIBS} 

${OBJS}: %.o: %.cpp %.dep 
	${CXX} ${CXXFLAGS} ${CXXINCLUDES} -o $@ -c $< 

${DEPS}: %.dep: %.cpp Makefile 
	${CXX} ${CXXFLAGS} -MM $< > $@ 

clean:: 
	-rm -f *~ *.o *.dep ${TARGET} 

distclean:: clean
