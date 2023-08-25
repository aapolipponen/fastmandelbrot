# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -fopenmp -O3 -march=native -lmpfr -lgmp

# Linker flags
LDFLAGS = -lfltk

# Source files
SOURCES = main.cpp

# Output binary
OUTPUT = bin

all: $(OUTPUT)

$(OUTPUT): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(OUTPUT) $(LDFLAGS)

clean:
	rm -f $(OUTPUT)
