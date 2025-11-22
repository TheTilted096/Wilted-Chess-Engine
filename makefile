EXE := Wilted-0-9-3-1-nnue
EVALFILE ?= src/wilted-net-1-0.bin
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