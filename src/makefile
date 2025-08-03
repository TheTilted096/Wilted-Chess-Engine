EXE := Wilted-0-6-0-0
#no evalfile yet
ARCH := native
SOURCES := Wilted.cpp Engine.cpp History.cpp TeaTable.cpp Searcher.cpp Evaluator.cpp Generator.cpp Position.cpp

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

engine: Wilted.o Position.o Generator.o Evaluator.o Searcher.o Engine.o TeaTable.o History.o
	$(CXX) Wilted.o Engine.o TeaTable.o Searcher.o Evaluator.o Generator.o Position.o History.o $(CXXFLAGS) -o $(EXE)

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

TeaTable.o: TeaTable.h TeaTable.cpp
	$(CXX) $(CXXFLAGS) -c TeaTable.cpp -o TeaTable.o

History.o: History.h History.cpp
	$(CXX) $(CXXFLAGS) -c History.cpp -o History.o

Engine.o: Engine.h Engine.cpp
	$(CXX) $(CXXFLAGS) -c Engine.cpp -o Engine.o

clean: # Works on Windows
	del *.o;
	del *.exe;
#del wiltest.exe;