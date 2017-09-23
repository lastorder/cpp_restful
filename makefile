
COMPILER = g++

OUTPUT_TARGET = libevhttp.so

CPL_FLAGS = -g -std=c++11 -fPIC

CPL_INCLUDES = -Isrc -Iinclude -I3rd/linux/include

LNK_LIB_PATH = -L3rd/linux/lib

LNK_FLAGS = -shared -pthread

LNK_LIB_NAMES = -levent -levent_pthreads -lcrypto -lssl 

SRCS := $(wildcard src/*.cpp)

OBJS := $(patsubst %cpp,%o,$(SRCS))

all:$(OBJS)
	$(COMPILER) -o $(OUTPUT_TARGET) $(LNK_FLAGS) $(OBJS) $(LNK_LIB_PATH) $(LNK_LIB_NAMES)

%.o:%.cpp
	$(COMPILER) -c $(CPL_FLAGS) $(CPL_INCLUDES) $< -o $@

clean:
	rm -rf $(OUTPUT_TARGET) $(OBJS) $(TEST_OBJS) simple


TEST_SRCS := $(wildcard test/*.cpp)
TEST_OBJS := $(patsubst %cpp,%o,$(TEST_SRCS))
.PHONY: test
test:$(TEST_OBJS)
	$(COMPILER) -o simple test/simple.o -L./ -levhttp -lrt -lpthread
