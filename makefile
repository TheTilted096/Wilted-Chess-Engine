EXE := Wilted-1-1-1-6
EVALFILE ?= src/wilted-net-1-4.bin
ARCH := native
SOURCES := src/*.cpp

CXX := clang++

CXXFLAGS := -std=c++20 -O3 -march=$(ARCH) -Wall -g -DEVALFILE=\"$(EVALFILE)\"
#-fconstexpr-steps=...

SUFFIX :=

ifeq ($(OS), Windows_NT)
	SUFFIX := .exe
endif

OUT := $(EXE)$(SUFFIX)

# One-Shot Compilation, for OpenBench

$(EXE):
	$(CXX) $(CXXFLAGS) -o $(OUT) $(SOURCES)

release:
	$(CXX) -static $(CXXFLAGS) -o $(OUT) $(SOURCES)