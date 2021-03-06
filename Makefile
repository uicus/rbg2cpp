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
RBG_GAMES_DIR := rbgGames
PARSER_INC_DIR := $(RBG_PARSER_DIR)/src
PARSER_BIN_DIR := $(RBG_PARSER_DIR)/bin

SIMULATIONS := 1000
DEPTH := 3
TIME := 1000
MEMORY := 2097152
RANDGEN := 0

C := g++
INCLUDE := -I$(INC_DIR) -I$(PARSER_INC_DIR)
COMMON_FLAGS = -Wall -Wextra -Wpedantic -std=c++17
COMPILER_FLAGS := $(COMMON_FLAGS) -O3 -march=native -flto -s $(INCLUDE)

DEBUG_FLAGS := $(COMMON_FLAGS) -O0 -g -DRBG_RANDOM_GENERATOR=$(RANDGEN)
RELEASE_FLAGS := $(COMMON_FLAGS) -Ofast -march=native -flto -s -DNDEBUG -DRBG_RANDOM_GENERATOR=$(RANDGEN)

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

define RUN_TEST
$(1)_%: $(RBG_GAMES_DIR)/games/%.rbg
	@rm -rf $(TEST_DIR)/reasoner.*
	@rm -rf $(TEST_DIR)/test
	@echo "Running $(TARGET)..."
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(BIN_DIR)/$(TARGET) $(2) -o reasoner $$<
	@mv reasoner.hpp $(TEST_DIR)/
	@mv reasoner.cpp $(TEST_DIR)/
	@echo "Running $(C)..."
	@taskset -c 0 time -v -p sh -c "$(C) $(3) -c -o $(TEST_DIR)/reasoner.o $(TEST_DIR)/reasoner.cpp; $(C) $(3) -o $(TEST_DIR)/test $(TEST_DIR)/reasoner.o $(TEST_DIR)/$(4).cpp"
	@echo "******** Running $(5)... ********"
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(TEST_DIR)/test $(6)
endef
define RUN_DEBUG_TEST
$(1)_%: $(RBG_GAMES_DIR)/games/%.rbg
	@rm -rf $(TEST_DIR)/reasoner.*
	@rm -rf $(TEST_DIR)/test
	@echo "Running $(TARGET)..."
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(BIN_DIR)/$(TARGET) $(2) -o reasoner $$<
	@mv reasoner.hpp $(TEST_DIR)/
	@mv reasoner.cpp $(TEST_DIR)/
	@echo "Running $(C)..."
	@taskset -c 0 time -v -p sh -c "$(C) $(3) -c -o $(TEST_DIR)/reasoner.o $(TEST_DIR)/reasoner.cpp; $(C) $(3) -o $(TEST_DIR)/test $(TEST_DIR)/reasoner.o $(TEST_DIR)/$(4).cpp"
	@echo "******** Running $(5)... ********"
	@ulimit -Sv $(MEMORY) && valgrind $(TEST_DIR)/test $(6)
endef

$(eval $(call RUN_TEST,simulate,,$(RELEASE_FLAGS),simulation,simulation,$(SIMULATIONS)))
$(eval $(call RUN_TEST,simulate_semisplit,-getters a,$(RELEASE_FLAGS),simulation_semisplit,simulation,$(SIMULATIONS)))
$(eval $(call RUN_TEST,benchmark,,$(RELEASE_FLAGS),benchmark,benchmark flat MC,$(TIME)))
$(eval $(call RUN_TEST,benchmark_semisplita,-getters a,$(RELEASE_FLAGS) -DSEMISPLIT_ACTIONS=1,benchmark_semisplit,benchmark semisplit flat MC,$(TIME)))
$(eval $(call RUN_TEST,benchmark_semisplits,-getters s,$(RELEASE_FLAGS) -DSEMISPLIT_ACTIONS=0,benchmark_semisplit,benchmark semisplit flat MC,$(TIME)))
$(eval $(call RUN_TEST,perft,,$(RELEASE_FLAGS),perft,perft,$(DEPTH)))
$(eval $(call RUN_TEST,perft_semisplit,-getters a,$(RELEASE_FLAGS),perft_semisplit,perft,$(DEPTH)))

$(eval $(call RUN_DEBUG_TEST,debug_simulate,,$(DEBUG_FLAGS),simulation,simulation,$(SIMULATIONS)))
$(eval $(call RUN_DEBUG_TEST,debug_simulate_semisplit,-getters a,$(DEBUG_FLAGS),simulation_semisplit,simulation,$(SIMULATIONS)))
$(eval $(call RUN_DEBUG_TEST,debug_benchmark,,$(DEBUG_FLAGS),benchmark,benchmark flat MC,$(TIME)))
$(eval $(call RUN_DEBUG_TEST,debug_benchmark_semisplita,-getters a,$(DEBUG_FLAGS) -DSEMISPLIT_ACTIONS=1,benchmark_semisplit,benchmark semisplit flat MC,$(TIME)))
$(eval $(call RUN_DEBUG_TEST,debug_benchmark_semisplits,-getters s,$(DEBUG_FLAGS) -DSEMISPLIT_ACTIONS=0,benchmark_semisplit,benchmark semisplit flat MC,$(TIME)))
$(eval $(call RUN_DEBUG_TEST,debug_perft,,$(DEBUG_FLAGS),perft,perft,$(DEPTH)))
$(eval $(call RUN_DEBUG_TEST,debug_perft_semisplit,-getters a,$(DEBUG_FLAGS),perft_semisplit,perft,$(DEPTH)))


benchmark_semisplit_old:
	@taskset -c 0 time -v -p sh -c "$(C) $(RELEASE_FLAGS) -c -o $(TEST_DIR)/reasoner.o $(TEST_DIR)/reasoner.cpp; $(C) $(RELEASE_FLAGS) -o $(TEST_DIR)/test $(TEST_DIR)/reasoner.o $(TEST_DIR)/benchmark_semisplit.cpp"
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(TEST_DIR)/test $(TIME)

benchmark_old:
	@taskset -c 0 time -v -p sh -c "$(C) $(RELEASE_FLAGS) -c -o $(TEST_DIR)/reasoner.o $(TEST_DIR)/reasoner.cpp; $(C) $(RELEASE_FLAGS) -o $(TEST_DIR)/test $(TEST_DIR)/reasoner.o $(TEST_DIR)/benchmark.cpp"
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(TEST_DIR)/test $(TIME)

simulate_old:
	@taskset -c 0 time -v -p sh -c "$(C) $(RELEASE_FLAGS) -c -o $(TEST_DIR)/reasoner.o $(TEST_DIR)/reasoner.cpp; $(C) $(RELEASE_FLAGS) -o $(TEST_DIR)/test $(TEST_DIR)/reasoner.o $(TEST_DIR)/simulation.cpp"
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(TEST_DIR)/test $(SIMULATIONS)

clean:
	cd $(RBG_PARSER_DIR); make clean; cd ..
	rm -rf $(OBJ_DIR)
	rm -rf $(DEP_DIR)
	rm -rf $(TEST_DIR)/reasoner.*
	rm -rf $(TEST_DIR)/test

distclean: clean
	cd $(RBG_PARSER_DIR); make distclean; cd ..
	rm -rf $(BIN_DIR)
