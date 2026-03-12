CXX = g++-15
# The sed command below removes the extra /SDL2 from the path
SDL_CFLAGS := $(shell sdl2-config --cflags | sed 's|/SDL2||')
SDL_LIBS   := $(shell sdl2-config --libs)

# Add them to the variables
CXXFLAGS += $(SDL_CFLAGS) -fopenmp
LIB += $(SDL_LIBS) -lpthread