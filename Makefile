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

SIMULATIONS := 100
SEMILENGTH := 0
DEPTH := 3
TIME := 100
MEMORY := 2000000
RANDGEN := 0

C := g++
INCLUDE := -I$(INC_DIR) -I$(PARSER_INC_DIR)
COMMON_FLAGS = -Wall -Wextra -Wpedantic -std=c++17

COMPILER_FLAGS := $(COMMON_FLAGS) -O3 -march=native -flto -s $(INCLUDE)
SIMULATIONS_FLAGS := $(COMMON_FLAGS) -Ofast -march=native -flto -s -DNDEBUG -DSEMILENGTH=$(SEMILENGTH) -DRBG_RANDOM_GENERATOR=$(RANDGEN)

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

define RUN_SIMULATION
$(1)_%: $(RBG_GAMES_DIR)/games/%.rbg
	@rm -rf $(TEST_DIR)/reasoner.*
	@rm -rf $(TEST_DIR)/test
	@echo "Running $(TARGET)..."
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(BIN_DIR)/$(TARGET) $(2) -o reasoner $$<
	@mv reasoner.hpp $(TEST_DIR)/
	@mv reasoner.cpp $(TEST_DIR)/
	@echo "Running $(C)..."
	@taskset -c 0 time -v -p sh -c "$(C) $(SIMULATIONS_FLAGS) $(3) -c -o $(TEST_DIR)/reasoner.o $(TEST_DIR)/reasoner.cpp; $(C) $(SIMULATIONS_FLAGS) $(3) -o $(TEST_DIR)/test $(TEST_DIR)/reasoner.o $(TEST_DIR)/$(4).cpp"
	@echo "******** Running $(5)... ********"
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(TEST_DIR)/test $(6)
endef

$(eval $(call RUN_SIMULATION,simulate,,$(RELEASE_FLAGS),simulation,simulation,$(SIMULATIONS)))
$(eval $(call RUN_SIMULATION,simulate_modsplit,-fmod-split,$(RELEASE_FLAGS),simulation_semisplit,simulation,$(SIMULATIONS)))
$(eval $(call RUN_SIMULATION,simulate_customsplit,-fcustom-split,$(RELEASE_FLAGS),simulation_semisplit,simulation,$(SIMULATIONS)))
$(eval $(call RUN_SIMULATION,verify,,$(RELEASE_FLAGS),verifier,verifier,$(SIMULATIONS)))
$(eval $(call RUN_SIMULATION,estimate_customsplit,-fcustom-split,$(RELEASE_FLAGS),estimation_semisplit,simulation,$(SIMULATIONS)))
$(eval $(call RUN_SIMULATION,benchmark,,$(RELEASE_FLAGS),benchmark_flatmc,benchmark flat MC,$(TIME)))
$(eval $(call RUN_SIMULATION,benchmark_modsplit,-fmod-split,$(RELEASE_FLAGS),benchmark_flatmc_semisplit,benchmark semisplit flat MC,$(TIME)))
$(eval $(call RUN_SIMULATION,benchmark_customsplit,-fcustom-split,$(RELEASE_FLAGS),benchmark_flatmc_semisplit,benchmark semisplit flat MC,$(TIME)))
$(eval $(call RUN_SIMULATION,perft,,$(RELEASE_FLAGS),perft,perft,$(DEPTH)))
$(eval $(call RUN_SIMULATION,perft_semisplit,-fmod-split,$(RELEASE_FLAGS),perft_semisplit,perft,$(DEPTH)))
$(eval $(call RUN_SIMULATION,perft_customsplit,-fcustom-split,$(RELEASE_FLAGS),perft_semisplit,perft,$(DEPTH)))

$(eval $(call RUN_SIMULATION,debug_simulate,,$(DEBUG_FLAGS),simulation,simulation,$(SIMULATIONS)))
$(eval $(call RUN_SIMULATION,debug_simulate_modsplit,-fmod-split,$(DEBUG_FLAGS),simulation_semisplit,simulation,$(SIMULATIONS)))
$(eval $(call RUN_SIMULATION,debug_simulate_customsplit,-fcustom-split,$(DEBUG_FLAGS),simulation_semisplit,simulation,$(SIMULATIONS)))
$(eval $(call RUN_SIMULATION,debug_estimate_customsplit,-fcustom-split,$(DEBUG_FLAGS),estimation_semisplit,simulation,$(SIMULATIONS)))
$(eval $(call RUN_SIMULATION,debug_benchmark,,$(DEBUG_FLAGS),benchmark_flatmc,benchmark flat MC,$(TIME)))
$(eval $(call RUN_SIMULATION,debug_benchmark_semisplit,-fmod-split,$(DEBUG_FLAGS),benchmark_flatmc_semisplit,benchmark semisplit flat MC,$(TIME)))
$(eval $(call RUN_SIMULATION,debug_benchmark_customsplit,-fcustom-split,$(DEBUG_FLAGS),benchmark_flatmc_semisplit,benchmark semisplit flat MC,$(TIME)))
$(eval $(call RUN_SIMULATION,debug_perft,,$(DEBUG_FLAGS),perft,perft,$(DEPTH)))
$(eval $(call RUN_SIMULATION,debug_perft_modsplit,-fmod-split,$(DEBUG_FLAGS),perft_semisplit,perft,$(DEPTH)))
$(eval $(call RUN_SIMULATION,debug_perft_customsplit,-fcustom-split,$(DEBUG_FLAGS),perft_semisplit,perft,$(DEPTH)))


benchmark_semisplit_old:
	@taskset -c 0 time -v -p sh -c "$(C) $(SIMULATIONS_FLAGS) -c -o $(TEST_DIR)/reasoner.o $(TEST_DIR)/reasoner.cpp; $(C) $(SIMULATIONS_FLAGS) -o $(TEST_DIR)/test $(TEST_DIR)/reasoner.o $(TEST_DIR)/benchmark_flatmc_semisplit.cpp"
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(TEST_DIR)/test $(TIME)

benchmark_old:
	@taskset -c 0 time -v -p sh -c "$(C) $(SIMULATIONS_FLAGS) -c -o $(TEST_DIR)/reasoner.o $(TEST_DIR)/reasoner.cpp; $(C) $(SIMULATIONS_FLAGS) -o $(TEST_DIR)/test $(TEST_DIR)/reasoner.o $(TEST_DIR)/benchmark_flatmc.cpp"
	@ulimit -Sv $(MEMORY) && taskset -c 0 time -v $(TEST_DIR)/test $(TIME)

simulate_old:
	@taskset -c 0 time -v -p sh -c "$(C) $(SIMULATIONS_FLAGS) -c -o $(TEST_DIR)/reasoner.o $(TEST_DIR)/reasoner.cpp; $(C) $(SIMULATIONS_FLAGS) -o $(TEST_DIR)/test $(TEST_DIR)/reasoner.o $(TEST_DIR)/simulation.cpp"
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
