CPPSRCS		= ssalloc.cc main.cc

TARGET		= ../../../bin/cuckoo

    
ROOT 		?= ..
LIBSSMEM := $(ROOT)/external

CPP			= g++

CPPFLAGS	=-std=c++11 -DINITIALIZE_FROM_ONE=1 -O3 -m64 -DNDEBUG  -D_GNU_SOURCE -DTAS -DDEFAULT -DCORE_NUM=32 -DFREQ_GHZ=2.400 -Wall -m64 -DGC=1  -fno-strict-aliasing -lrt -pthread -I$(ROOT)/include -I$(LIBSSMEM)/include

#-m64 -DGC=1 -DCOMPUTE_LATENCY -DDO_TIMINGS -DUSE_SSPFD -DLATENCY_ALL_CORES=0 -fno-strict-aliasing  
LFLAGS		=-std=c++11 -DINITIALIZE_FROM_ONE=1 -O3 -m64 -DNDEBUG -D_GNU_SOURCE -DTAS -DDEFAULT -DCORE_NUM=32 -DFREQ_GHZ=2.400 -Wall -m64 -DGC=1  -fno-strict-aliasing -lrt -lm -pthread -I$(ROOT)/include -I$(LIBSSMEM)/include -L$(LIBSSMEM)/lib -L/usr/lib -lsspfd_x86_64 -lssmem_x86_64

#CPPFLAGS	=-std=c++11 -O3 -m64 -DNDEBUG -DNO_SET_CPU -D_GNU_SOURCE -DTAS -DOPTERON -DCORE_NUM=48 -DFREQ_GHZ=2.100 -Wall -fno-strict-aliasing  -lrt -pthread -I$(ROOT)/include -I$(LIBSSMEM)/include
#LFLAGS		=-std=c++11 -O3 -m64 -DNDEBUG -DNO_SET_CPU -D_GNU_SOURCE -DTAS -DOPTERON -DCORE_NUM=48 -DFREQ_GHZ=2.100 -Wall -fno-strict-aliasing  -lrt -pthread -I$(ROOT)/include -L$(LIBSSMEM)/lib -lsspfd_x86_64 -lssmem_x86_64

OBJS		= $(CPPSRCS:.cpp=.o)

all: $(TARGET)

main.o:
	$(CPP) $(CPPFLAGS) -c ./benchmarks/main.cc 
ssalloc.o:
	$(CPP) $(CPPFLAGS) -c ./benchmarks/ssalloc.cc 
#cpp_framework.o:
#	$(CPP) $(CPPFLAGS) -c ./framework/cpp_framework.cpp -g

$(TARGET): $(OBJS)
	$(CPP) $(LFLAGS) $(OBJS) -o $(TARGET) 

clean:
	rm -f main.o ssalloc.o $(TARGET)

depend:
	mkdep $(SRCS)
