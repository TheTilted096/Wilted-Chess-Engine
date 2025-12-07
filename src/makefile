EXE := Wilted-1-1-1-4
EVALFILE ?= wilted-net-1-3.bin
ARCH := native
SOURCES := *.cpp
CXX := clang++

CXXFLAGS := -std=c++20 -O3 -march=$(ARCH) -Wall -g
#-fconstexpr-steps=...

SUFFIX :=

ifeq ($(OS), Windows_NT)
	SUFFIX := .exe
endif

OUT := $(EXE)$(SUFFIX)

# One-Shot Release Compilation

$(EXE):
	$(CXX) $(CXXFLAGS) -o $(OUT) $(SOURCES)

# During Development, frequent compilation

engine: Wilted.o Position.o Generator.o Performer.o Evaluator.o Searcher.o Engine.o TeaTable.o History.o Interface.o
	$(CXX) Wilted.o Engine.o TeaTable.o Searcher.o Evaluator.o Performer.o Generator.o Position.o History.o Interface.o $(CXXFLAGS) -o $(EXE)

Wilted.o: Wilted.cpp
	$(CXX) $(CXXFLAGS) -c Wilted.cpp -o Wilted.o

Position.o: Position.h Position.cpp
	$(CXX) $(CXXFLAGS) -c Position.cpp -o Position.o

Generator.o: Generator.h Generator.cpp
	$(CXX) $(CXXFLAGS) -c Generator.cpp -o Generator.o

Performer.o: Performer.h Performer.cpp
	$(CXX) $(CXXFLAGS) -c Performer.cpp -o Performer.o

Evaluator.o: Evaluator.h Evaluator.cpp
	$(CXX) $(CXXFLAGS) -DEVALFILE=\"$(EVALFILE)\" -c Evaluator.cpp -o Evaluator.o

Searcher.o: Searcher.h Timeman.h Princes.h Searcher.cpp
	$(CXX) $(CXXFLAGS) -c Searcher.cpp -o Searcher.o

TeaTable.o: TeaTable.h TeaTable.cpp
	$(CXX) $(CXXFLAGS) -c TeaTable.cpp -o TeaTable.o

History.o: History.h History.cpp
	$(CXX) $(CXXFLAGS) -c History.cpp -o History.o

Engine.o: Engine.h Engine.cpp
	$(CXX) $(CXXFLAGS) -c Engine.cpp -o Engine.o
	
Interface.o: Interface.h Interface.cpp
	$(CXX) $(CXXFLAGS) -c Interface.cpp -o Interface.o

ifeq ($(OS), Windows_NT)
clean:
	-del *.o 2>nul
	-del *.exe 2>nul
else
clean:
	rm -f *.o
	rm -f $(EXE)
endif