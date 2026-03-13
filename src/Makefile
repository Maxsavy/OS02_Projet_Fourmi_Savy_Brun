UNAME := $(shell uname -s)
ifeq ($(UNAME), Darwin)
    include Make_osx.inc
else ifeq ($(UNAME), Linux)
    include Make_linux.inc
else
    include Make_msys2.inc
endif

CXXFLAGS += -std=c++17 -fopenmp

ifdef DEBUG
    CXXFLAGS2 = $(CXXFLAGS) -g -O0 -Wall -fbounds-check -pedantic -D_GLIBCXX_DEBUG
else
    CXXFLAGS2 = $(CXXFLAGS) -O3 -march=native -Wall 
endif

ALL= ant_simu.exe ant_simu_vect.exe ant_simu_vect_parallel.exe

default: help

all: $(ALL)

clean:
	@rm -fr *.o *.exe *~

.cpp.o:
	$(CXX) $(CXXFLAGS2) -c $< -o $@

ant_simu_vect_parallel.o: ant_simu_vect_parallel.cpp
	$(MPICXX) $(CXXFLAGS2) -c $< -o $@

ant_simu.exe: ant.o fractal_land.o renderer.o window.o ant_simu.o
	$(CXX) $(CXXFLAGS2) $^ -o $@ $(LIB)

ant_simu_vect.exe: ant_vect.o fractal_land.o renderer_vect.o window.o ant_simu_vect.o
	$(CXX) $(CXXFLAGS2) $^ -o $@ $(LIB)

ant_simu_vect_parallel.exe: ant_vect.o fractal_land.o renderer_vect.o window.o ant_simu_vect_parallel.o
	$(MPICXX) $(CXXFLAGS2) $^ -o $@ $(LIB)

help:
	@echo "Available targets : "
	@echo "    all            : compile all executables"
	@echo "Add DEBUG=yes to compile in debug"
	@echo "Configuration :"
	@echo "    CXX      :    $(CXX)"
	@echo "    CXXFLAGS :    $(CXXFLAGS)"
	@echo "    MPICXX   :    $(MPICXX)"

%.html: %.md
	pandoc -s --toc $< --css=./github-pandoc.css --metadata pagetitle="OS202 - TD1" -o $@