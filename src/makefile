EXE := Wilted-0-3-1-0
#no evalfile yet
ARCH := native
SOURCES := Wilted.cpp Searcher.cpp Evaluator.cpp Generator.cpp Position.cpp

CXX := clang++

CXXFLAGS := -std=c++20 -O3 -march=$(ARCH) -static -Wall -g
#-fconstexpr-steps=...

SUFFIX :=

ifeq ($(OS), Windows_NT)
	SUFFIX := .exe
endif

OUT := $(EXE)$(SUFFIX)

# One-Shot Compilation, for OpenBench

$(EXE):
	$(CXX) $(CXXFLAGS) -o $(OUT) $(SOURCES)

# During Development, frequent compilation

engine: Wilted.o Position.o Generator.o Evaluator.o Searcher.o
	$(CXX) Wilted.o Searcher.o Evaluator.o Generator.o Position.o $(CXXFLAGS) -o $(EXE)

Wilted.o: Wilted.cpp
	$(CXX) $(CXXFLAGS) -c Wilted.cpp -o Wilted.o

Position.o: Position.h Position.cpp
	$(CXX) $(CXXFLAGS) -c Position.cpp -o Position.o

Generator.o: Generator.h Generator.cpp
	$(CXX) $(CXXFLAGS) -c Generator.cpp -o Generator.o

Evaluator.o: Evaluator.h Evaluator.cpp
	$(CXX) $(CXXFLAGS) -c Evaluator.cpp -o Evaluator.o

Searcher.o: Searcher.h Timeman.h Princes.h Searcher.cpp
	$(CXX) $(CXXFLAGS) -c Searcher.cpp -o Searcher.o

clean: # Works on Windows
	del *.o;
	del *.exe;
#del wiltest.exe;