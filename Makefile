CXX := g++
CXXFLAGS := -std=c++17 -Iinclude -Ilib/SFML -Wno-narrowing
LDFLAGS := -Llib/SFML -Wl,-rpath=lib/SFML -lsfml-graphics -lsfml-window -lsfml-system

SRC_DIR := src
OBJ_DIR := obj
BIN := chess

SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

# Make
all: $(BIN)

# Compiling
$(BIN): $(OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

# .cpp -> .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Creating obj/
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# make run
run: all
	LD_LIBRARY_PATH=lib/SFML ./$(BIN)

# make clean
clean:
	rm -rf $(OBJ_DIR) $(BIN)

include scripts/dependencies/install.mk

