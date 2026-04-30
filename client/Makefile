# Setting up our compiler flags and destinations
PROG   = mugstring_client
DST	   = ../filesystem-overlay/usr/local/sbin/
CC     = arm-linux-gnueabihf-g++
# CC	   = g++
CFLAGS = -g -std=c++17 -Wall -Werror
LIBS   =

# Setting up the src and object paths
ROOT_SRC  = $(wildcard *.cpp)                        # all .cpp files in root
SRC_FILES = $(shell find src -name '*.cpp')          # all .cpp files under src/, recursively
SRC       = $(ROOT_SRC) $(SRC_FILES)

ROOT_OBJ = $(patsubst %.cpp,     obj/%.o,     $(ROOT_SRC))
SRC_OBJ  = $(patsubst src/%.cpp, obj/src/%.o, $(SRC_FILES))
OBJ      = $(ROOT_OBJ) $(SRC_OBJ)

# Make sure the commands 'all' and 'clean' runs properly
# even if you included a file with the same name
.PHONY: all clean

# --- default target ---
all: $(PROG)
	cp $(PROG) $(DST)

# --- link ---
# Explains to the compiler that all .o files in these
# directories will build the PROG (.exe file)
$(PROG): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LIBS)

# --- compile (root .cpp -> obj/*.o) ---
# Can be transalated to:
# for every .cpp file in root/, build the responding .o file in obj/.
# First make sure directory obj/ exists and then compile.
obj/%.o: %.cpp | obj
	$(CC) $(CFLAGS) -c $< -o $@

# --- compile (src/**/*.cpp -> obj/src/**/*.o) ---
# % matches any path including slashes, so this handles any subfolder depth.
# mkdir -p creates the output directory on demand.
obj/src/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# --- ensure folders exist ---
obj:
	mkdir -p obj

# --- clean ---
clean:
	rm -f $(PROG) $(DST)/$(PROG)
	rm -rf obj