# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Iinclude

# Directories
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = test

# Output file
OUTPUT = $(BUILD_DIR)/interpreter

# Source files
SRC_FILES = $(SRC_DIR)/Token.cpp

# Targets
all: $(OUTPUT)

$(OUTPUT): $(SRC_FILES)
	$(CXX) $(CXXFLAGS) -o $(OUTPUT) $(SRC_FILES)

run: $(OUTPUT)
	$(OUTPUT) $(FILE)

clean:
	rm -f $(OUTPUT)

.PHONY: all run clean
