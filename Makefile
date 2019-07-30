SUFFIXES += .d
NODEPS := clean distclean

TARGET := rbg2cpp
SRC_DIR := src
INC_DIR := inc
OBJ_DIR := obj
BIN_DIR := bin
DEP_DIR := dep
TEST_DIR = test
MAIN_FILE := $(SRC_DIR)/main.cpp
RBG_PARSER_DIR := rbgParser
PARSER_INC_DIR := $(RBG_PARSER_DIR)/src
PARSER_BIN_DIR := $(RBG_PARSER_DIR)/bin

C := g++
INCLUDE := -I$(INC_DIR) -I$(PARSER_INC_DIR)
COMMON_FLAGS = -Wall -Wextra -Wpedantic -Ofast -march=native -std=c++17

COMPILER_FLAGS := $(COMMON_FLAGS) -s $(INCLUDE)
SIMULATIONS_FLAGS := $(COMMON_FLAGS) -flto

SIMULATIONS := 100
DEPTH := 3
MEMORY := 2000000

OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(wildcard $(SRC_DIR)/*.cpp))
DEPFILES := $(patsubst $(SRC_DIR)/%.cpp, $(DEP_DIR)/%.d, $(wildcard $(SRC_DIR)/*.cpp))

all: $(TARGET)

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
    -include $(DEPFILES)
endif

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_DIR)/%.d | $(OBJ_DIR)
	$(C) $(COMPILER_FLAGS) -c $< -o $@

$(DEP_DIR)/%.d: $(SRC_DIR)/%.cpp | $(DEP_DIR)
	$(C) $(COMPILER_FLAGS) -MM -MT '$(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$<) $@' $< -MF $@

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	cd $(RBG_PARSER_DIR); make librbgParser.a; cd ..
	$(C) $(COMPILER_FLAGS) $(OBJECTS) $(PARSER_BIN_DIR)/librbgParser.a -o $(BIN_DIR)/$@

$(DEP_DIR):
	mkdir -p $(DEP_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

simulate_%: $(RBG_PARSER_DIR)/examples/%.rbg
	@rm -rf $(TEST_DIR)/reasoner.*
	@rm -rf $(TEST_DIR)/test
	@echo "Running $(TARGET)..."
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(BIN_DIR)/$(TARGET) -o reasoner $<
	@mv reasoner.hpp $(TEST_DIR)/
	@mv reasoner.cpp $(TEST_DIR)/
	@echo "Running $(C)..."
	@taskset -c 0 time -v -p sh -c "$(C) $(SIMULATIONS_FLAGS) -c -o $(TEST_DIR)/reasoner.o $(TEST_DIR)/reasoner.cpp; $(C) $(SIMULATIONS_FLAGS) -o $(TEST_DIR)/test $(TEST_DIR)/reasoner.o $(TEST_DIR)/simulation.cpp"
	@echo "******** Running simulation... ********"
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(TEST_DIR)/test $(SIMULATIONS)

simulate_old:
	@taskset -c 0 time -v -p sh -c "$(C) $(SIMULATIONS_FLAGS) -c -o $(TEST_DIR)/reasoner.o $(TEST_DIR)/reasoner.cpp; $(C) $(SIMULATIONS_FLAGS) -o $(TEST_DIR)/test $(TEST_DIR)/reasoner.o $(TEST_DIR)/simulation.cpp"
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(TEST_DIR)/test $(SIMULATIONS)

benchmark_%: $(RBG_PARSER_DIR)/examples/%.rbg
	@rm -rf $(TEST_DIR)/reasoner.*
	@rm -rf $(TEST_DIR)/test
	@echo "Running $(TARGET)..."
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(BIN_DIR)/$(TARGET) -o reasoner $<
	@mv reasoner.hpp $(TEST_DIR)/
	@mv reasoner.cpp $(TEST_DIR)/
	@echo "Running $(C)..."
	@taskset -c 0 time -v -p sh -c "$(C) $(SIMULATIONS_FLAGS) -c -o $(TEST_DIR)/reasoner.o $(TEST_DIR)/reasoner.cpp; $(C) $(SIMULATIONS_FLAGS) -o $(TEST_DIR)/test $(TEST_DIR)/reasoner.o $(TEST_DIR)/benchmark-flatmc.cpp"
	@echo "******** Running benchmark flat MC... ********"
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(TEST_DIR)/test $(SIMULATIONS)

perft_%: $(RBG_PARSER_DIR)/examples/%.rbg
	@rm -rf $(TEST_DIR)/reasoner.*
	@rm -rf $(TEST_DIR)/test
	@echo "Running $(TARGET)..."
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(BIN_DIR)/$(TARGET) -o reasoner $<
	@mv reasoner.hpp $(TEST_DIR)/
	@mv reasoner.cpp $(TEST_DIR)/
	@echo "Running $(C)..."
	@taskset -c 0 time -v -p sh -c "$(C) $(SIMULATIONS_FLAGS) -c -o $(TEST_DIR)/reasoner.o $(TEST_DIR)/reasoner.cpp; $(C) $(SIMULATIONS_FLAGS) -o $(TEST_DIR)/test $(TEST_DIR)/reasoner.o $(TEST_DIR)/perft.cpp"
	@echo "******** Running perft... ********"
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(TEST_DIR)/test $(DEPTH)

clean:
	cd $(RBG_PARSER_DIR); make clean; cd ..
	rm -rf $(OBJ_DIR)
	rm -rf $(DEP_DIR)
	rm -rf $(TEST_DIR)/reasoner.*
	rm -rf $(TEST_DIR)/test

distclean: clean
	cd $(RBG_PARSER_DIR); make distclean; cd ..
	rm -rf $(BIN_DIR)
