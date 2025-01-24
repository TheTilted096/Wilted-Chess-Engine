# makefile

debug:
	clang++ -O2 -Wall -static -g -march=native -o wtest WiltedDriver.cpp WiltedPosition.cpp WiltedBitboards.cpp 

norm:
	clang++ -O2 -Wall -static -g -march=native -o wnorm WiltedDriver.cpp WiltedPosition.cpp WiltedBitboards.cpp 

# debug:
#clang++ -O2 -Wall -static -g -march=native -o stilted-28-test STiltedDriverV3.cpp STiltedMoveGenV5.cpp STiltedSearchEvalV4.cpp
