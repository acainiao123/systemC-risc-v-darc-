XLEN?=32
CC=g++
CCHOST=g++
CFLAGS=-Wno-overflow
EXEC=qsort

DRIVERSDIR=$(PWD)/../drivers


ifneq (, $(wildcard /bin/uname))
CPU_ARCH = $(shell /bin/uname -m)
endif

ifeq ($(CPU_ARCH),x86_64)
CWB_LIB   = $(CYBER_PATH)/lib64
ARCH_FLAG = 
else
CWB_LIB   = $(CYBER_PATH)/lib
ARCH_FLAG = -m32
endif

ifdef CYBER_SYSTEMC_TARGET_ARCH
CWB_SC_ARCH = $(CYBER_SYSTEMC_TARGET_ARCH)
else
ifeq (, $(findstring Windows,$(OS)))
ifeq ($(CPU_ARCH),x86_64)
CWB_SC_ARCH = linux64
else
CWB_SC_ARCH = linux
endif
else
CWB_SC_ARCH = mingw
endif
endif

ifdef CYBER_SYSTEMC_HOME
CWB_SC_HOME = $(CYBER_SYSTEMC_HOME)
else
CWB_SC_HOME = $(CYBER_PATH)/osci
endif
#CWB_SC_HOME = /mnt/c/systemc_s2c/systemc-2.3.3

CWB_SC_INCL = -I"$(CWB_SC_HOME)/include"
CWB_SC_LIB  = "$(CWB_SC_HOME)/lib-$(CWB_SC_ARCH)/libsystemc.a"

LIBS	= -lm $(CWB_SC_LIB)  -Wl,-rpath="$(CWB_LIB)"

SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, %.o, $(SRCS) )

# # test.o: $(SRCS)
# # test.o: *.cpp
# # test.o: %.cpp  -DPRT_MEMORY  -DMEMORY_MASK
# test.o: $(SRCS)
# 	$(CC) $^ $(CFLAGS)  -DPRT_MEMORY -g -DMEMORY_MASK -o $@
# 	# $(CCHOST) $(EXEC).c -o $(EXEC)
# 	# ./$(EXEC) > expectedOutput
# 	# rm -f $(EXEC)
all: test.exe

test.exe: $(OBJS)
	$(CC) $^  $(ARCH_FLAG)   $(LIBS) -I.    -o $@
#-DCHECKCYCLES
$(OBJS): %.o:%.cpp
	$(CC)  $(ARCH_FLAG)  $(CWB_SC_INCL) -DSYSCSIMULATION   -c  $^ -o $@

.PHONY: all clean

clean:
	rm -f *.o *.exe
