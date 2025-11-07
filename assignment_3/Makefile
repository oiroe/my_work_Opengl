# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Iinclude
LDFLAGS = -Wl,--copy-dt-needed-entries -lassimp -lglfw -lGL -ldl -lpthread

# Directories
SRC_DIR = .
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Target executable
TARGET = game

# Source files
SOURCES = main.cpp shader.cpp mesh.cpp model.cpp glad.c
OBJECTS = $(addprefix $(OBJ_DIR)/, $(SOURCES:.cpp=.o))
OBJECTS := $(OBJECTS:.c=.o)

# Default target
all: $(BUILD_DIR) $(OBJ_DIR) $(TARGET)

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete! Run with: ./$(TARGET)"

# Compile C++ source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile C source files (glad.c)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Clean complete!"

# Rebuild from scratch
rebuild: clean all

# Run the game
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean rebuild run
