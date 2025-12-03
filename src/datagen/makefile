EXE := WiltedGen-2-0-0
EVALFILE ?= ../wilted-net-1-2.bin
ARCH := native

SOURCES := Datagen.cpp Match.cpp Game.cpp ../Interface.cpp ../Engine.cpp ../History.cpp ../TeaTable.cpp ../Searcher.cpp ../Evaluator.cpp ../Performer.cpp ../Generator.cpp ../Position.cpp

CXX := clang++

CXXFLAGS := -std=c++20 -O3 -march=$(ARCH) -Wall -g -DEVALFILE=\"$(EVALFILE)\"

SUFFIX :=

ifeq ($(OS), Windows_NT)
	SUFFIX := .exe
endif

OUT := $(EXE)$(SUFFIX)

$(EXE):
	$(CXX) -static $(CXXFLAGS) -o $(OUT) $(SOURCES)

# During Development, frequent compilation

datagen: Datagen.o Match.o Game.o Position.o Generator.o Performer.o Evaluator.o Searcher.o Engine.o TeaTable.o History.o Interface.o
	$(CXX) Datagen.o Match.o Game.o Engine.o TeaTable.o Searcher.o Evaluator.o Performer.o Generator.o Position.o History.o Interface.o $(CXXFLAGS) -o $(EXE)

Datagen.o: Datagen.cpp
	$(CXX) $(CXXFLAGS) -c Datagen.cpp -o Datagen.o

Position.o: ../Position.h ../Position.cpp
	$(CXX) $(CXXFLAGS) -c ../Position.cpp -o Position.o

Generator.o: ../Generator.h ../Generator.cpp
	$(CXX) $(CXXFLAGS) -c ../Generator.cpp -o Generator.o

Performer.o: ../Performer.h ../Performer.cpp
	$(CXX) $(CXXFLAGS) -c ../Performer.cpp -o Performer.o

Evaluator.o: ../Evaluator.h ../Evaluator.cpp
	$(CXX) $(CXXFLAGS) -c ../Evaluator.cpp -o Evaluator.o

Searcher.o: ../Searcher.h ../Timeman.h ../Princes.h ../Searcher.cpp
	$(CXX) $(CXXFLAGS) -c ../Searcher.cpp -o Searcher.o

TeaTable.o: ../TeaTable.h ../TeaTable.cpp
	$(CXX) $(CXXFLAGS) -c ../TeaTable.cpp -o TeaTable.o

History.o: ../History.h ../History.cpp
	$(CXX) $(CXXFLAGS) -c ../History.cpp -o History.o

Engine.o: ../Engine.h ../Engine.cpp
	$(CXX) $(CXXFLAGS) -c ../Engine.cpp -o Engine.o
	
Interface.o: ../Interface.h ../Interface.cpp
	$(CXX) $(CXXFLAGS) -c ../Interface.cpp -o Interface.o

Game.o: Game.h Game.cpp
	$(CXX) $(CXXFLAGS) -c Game.cpp -o Game.o

Match.o: Match.h Match.cpp
	$(CXX) $(CXXFLAGS) -c Match.cpp -o Match.o


clean: # Works on Windows
	del *.o;
	del *.exe;
