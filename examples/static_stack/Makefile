EXE_FILE := ./static_stack
SRC_FILES := $(shell find ./ -type f -name '*.cpp')

CC := g++
LDFLAGS := 
CPPFLAGS := -std=c++11 \
			-I./ \
			-I../../include

# everything other than the default build should only be run explicitly
.PHONY: debug release clean gdb

# default build (release)
all: release

# debug build
debug: CPPFLAGS += -DDEBUG -g # add debug flags
debug: executable # compile

# release build
release: executable

# compile & link step
executable:
	$(CC) $(CPPFLAGS) $(LDFLAGS) -o $(EXE_FILE) $(SRC_FILES)

# command for starting debug session (requires debug build)
gdb: debug
	gdb $(EXE_FILE)

# quickly start the program. Will build release by default
run: executable
	$(EXE_FILE)

clean:
	rm $(EXE_FILE)