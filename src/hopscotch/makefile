CPPSRCS		= ssalloc.cpp main.cpp cpp_framework.cpp 
#hopscotch.cpp
ROOT 		?= ../..
LIBSSMEM := $(ROOT)/external
TARGET		= $(ROOT)/bin/hopscotch

#BINS = $(BINDIR)/hopscotch
CPP			= g++

CPPFLAGS	=-std=c++11  -c -D_REENTRANT -O3 -DDEFAULT -DNDEBUG -m64 -DINITIALIZE_FROM_ONE=0  -DINTEL64 -D_GNU_SOURCE -DTAS  -DCORE_NUM=64 -DFREQ_GHZ=2.400 -Wall -fno-strict-aliasing -lrt -pthread -I$(ROOT)/include -I$(LIBSSMEM)/include
#CPPFLAGS -mrtm -mhle	+= -DCOMPUTE_LATENCY -lm -lsspfd -DDO_TIMINGS -DUSE_SSPFD -DLATENCY_ALL_CORES=0 

LFLAGS		=-std=c++11 -O3 -m64  -DNDEBUG -D_REENTRANT -DINITIALIZE_FROM_ONE=0  -DINTEL64 -D_GNU_SOURCE -DTAS -DCORE_NUM=64 -DFREQ_GHZ=2.100 -Wall -DDEFAULT -lm -fno-strict-aliasing -lrt -pthread -I$(ROOT)/include -L$(LIBSSMEM)/lib -lssmem_x86_64 -lsspfd_x86_64 -lm 
#-DNO_SET_CPU -DDO_TIMINGS -DUSE_SSPFD -DLATENCY_ALL_CORES=0 -DCOMPUTE_LATENCY -DNDEBUG
OBJS		= $(CPPSRCS:.cpp=.o)

all: $(TARGET)

hopscotch.o: 
	$(CPP) $(CPPFLAGS) -c ./src/hopscotch.cpp
main.o:
	$(CPP) $(CPPFLAGS) -c ./src/main.cpp
	#$(CPP) $(CPPFLAGS) -c ./test/test.cpp
ssalloc.o:
	$(CPP) $(CPPFLAGS) -c ./src/ssalloc.cpp 
cpp_framework.o:
	$(CPP) $(CPPFLAGS) -c ./src/cpp_framework.cpp 

$(TARGET): $(OBJS)
	$(CPP) $(LFLAGS) $(OBJS) -o $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

depend:
	mkdep $(SRCS)
