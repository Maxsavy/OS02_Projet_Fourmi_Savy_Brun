# --- Compiler and Flags ---
# Use 'ls /opt/homebrew/bin/g++-*' to check.
CXX = g++-15
# The sed command below removes the extra /SDL2 from the path
SDL_CFLAGS := $(shell sdl2-config --cflags | sed 's|/SDL2||')
SDL_LIBS   := $(shell sdl2-config --libs)

CXXFLAGS = -std=c++17 -fopenmp -O3 -march=native -Wall $(SDL_CFLAGS)
LIB = -lpthread $(SDL_LIBS)

# --- Debug vs Release ---
ifdef DEBUG
    CXXFLAGS += -g -O0 -Wall -fbounds-check -pedantic -D_GLIBCXX_DEBUG
else
    CXXFLAGS += -O3 -march=native -Wall
endif

# Combine SDL flags into the main variables
CXXFLAGS += $(SDL_CFLAGS)
LIB = -lpthread $(SDL_LIBS)

# --- Targets ---
ALL = ant_simu.exe 

all: $(ALL)

# How to link the final executable
ant_simu.exe : ant.o fractal_land.o renderer.o window.o ant_simu.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIB)	

# How to compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -fr *.o *.exe *~

help:
	@echo "Available targets : "
	@echo "    all            : compile all executables"
	@echo "    clean          : remove objects and executables"
	@echo "Add DEBUG=yes to compile in debug mode"
	@echo "Configuration :"
	@echo "    CXX      : $(CXX)"
	@echo "    SDL_INC  : $(SDL_CFLAGS)"

# Documentation (optional)
%.html: %.md
	pandoc -s --toc $< --css=./github-pandoc.css --metadata pagetitle="OS202 - TD1" -o $@