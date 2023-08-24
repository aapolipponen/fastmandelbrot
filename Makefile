# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -fopenmp

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
