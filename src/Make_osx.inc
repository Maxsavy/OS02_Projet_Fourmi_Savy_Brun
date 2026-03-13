CXX = g++-15
MPICXX = OMPI_CXX=g++-15 mpic++

SDL_CFLAGS := $(shell sdl2-config --cflags | sed 's|/SDL2||')
SDL_LIBS   := $(shell sdl2-config --libs)

CXXFLAGS += $(SDL_CFLAGS) -fopenmp
LIB += $(SDL_LIBS) -lpthread