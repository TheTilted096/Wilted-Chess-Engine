EXE := Wilted
#no evalfile yet
ARCH := native
SOURCES := Wilted.cpp Generator.cpp Position.cpp

CXX := clang++

CXXFLAGS := -std=c++20 -O3 -march=$(ARCH) -static -Wall -g #-fconstexpr-steps=...

SUFFIX :=

ifeq ($(OS), Windows_NT)
	SUFFIX := .exe
endif

OUT := $(EXE)$(SUFFIX)

# One-Shot Compilation, for OpenBench

$(EXE):
	$(CXX) $(CXXFLAGS) -o $(OUT) $(SOURCES)

# During Development, frequent compilation

engine: Wilted.o Position.o Generator.o
	$(CXX) Wilted.o Position.o Generator.o $(CXXFLAGS) -o wiltest

Wilted.o: Wilted.cpp
	$(CXX) $(CXXFLAGS) -c Wilted.cpp -o Wilted.o

Position.o: Position.h Position.cpp
	$(CXX) $(CXXFLAGS) -c Position.cpp -o Position.o

Generator.o: Generator.h Generator.cpp
	$(CXX) $(CXXFLAGS) -c Generator.cpp -o Generator.o

clean: # Works on Windows
	del *.o;
#del wiltest.exe;