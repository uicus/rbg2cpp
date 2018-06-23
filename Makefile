SUFFIXES += .d
NODEPS := clean distclean

TARGET := rbg2cpp
SRC_DIR := src
INC_DIR := inc
OBJ_DIR := obj
BIN_DIR := bin
DEP_DIR := dep
MAIN_FILE := $(SRC_DIR)/main.cpp
RBG_PARSER_DIR := rbgParser
PARSER_INC_DIR := $(RBG_PARSER_DIR)/src
PARSER_BIN_DIR := $(RBG_PARSER_DIR)/bin

C := g++
INCLUDE := -I$(INC_DIR) -I$(PARSER_INC_DIR)
CFLAGS := -Wall -Wextra -Wpedantic -O3 -flto -std=c++11 -s $(INCLUDE)

OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(wildcard $(SRC_DIR)/*.cpp))
DEPFILES := $(patsubst $(SRC_DIR)/%.cpp, $(DEP_DIR)/%.d, $(wildcard $(SRC_DIR)/*.cpp))

all: $(TARGET)

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
    -include $(DEPFILES)
endif

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_DIR)/%.d | $(OBJ_DIR)
	$(C) $(CFLAGS) -c $< -o $@

$(DEP_DIR)/%.d: $(SRC_DIR)/%.cpp | $(DEP_DIR)
	$(C) $(CFLAGS) -MM -MT '$(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$<) $@' $< -MF $@

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	cd $(RBG_PARSER_DIR); make librbgParser.a; cd ..
	$(C) $(CFLAGS) $(OBJECTS) $(PARSER_BIN_DIR)/librbgParser.a -o $(BIN_DIR)/$@

$(DEP_DIR):
	mkdir -p $(DEP_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	cd $(RBG_PARSER_DIR); make clean; cd ..
	rm -rf $(OBJ_DIR)
	rm -rf $(DEP_DIR)

distclean: clean
	cd $(RBG_PARSER_DIR); make distclean; cd ..
	rm -rf $(BIN_DIR)
