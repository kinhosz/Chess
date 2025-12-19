CXX := g++

BASE_FLAGS := -std=c++17 -Iinclude -Ilib/SFML -Wno-narrowing

DEBUG_FLAGS := -g -O0 -fsanitize=address,undefined
RELEASE_FLAGS := -O3

# SFML
LDFLAGS_BASE := -Llib/SFML -Wl,-rpath=lib/SFML \
                -lsfml-graphics -lsfml-window -lsfml-system

SRC_DIR := src
OBJ_DIR := obj
BIN := chess

SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))


# =========================================
# DEFAULT MODE = RELEASE
# =========================================
CXXFLAGS := $(BASE_FLAGS) $(RELEASE_FLAGS)
LDFLAGS := $(LDFLAGS_BASE)


# =========================================
# MAIN RULE
# =========================================
all: $(BIN)

debug: CXXFLAGS := $(BASE_FLAGS) $(DEBUG_FLAGS)
debug: LDFLAGS := $(LDFLAGS_BASE) -fsanitize=address,undefined
debug: clean $(BIN)


# =========================================
# BUILD RULES
# =========================================
$(BIN): $(OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)


# =========================================
# RUN HELPERS
# =========================================
run: all
	LD_LIBRARY_PATH=lib/SFML ./$(BIN)

run-debug: debug
	LD_LIBRARY_PATH=lib/SFML ./$(BIN)


clean:
	rm -rf $(OBJ_DIR) $(BIN)

include scripts/dependencies/install.mk
