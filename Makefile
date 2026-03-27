# Roucaarize Makefile
# Tree-Walking Interpreter Build System

# Compiler settings
CXX := g++
CXXFLAGS := -std=c++20 -pthread -Wall -Wextra -Wpedantic -O3 -march=native -fno-rtti -fno-stack-protector -MMD -MP
CXXFLAGS_DEBUG := -std=c++20 -pthread -Wall -Wextra -Wpedantic -g -O0 -DDEBUG -MMD -MP

# Directories
SRC_DIR := core/src
STDLIB_SRC := core/stdlib/src
INC_DIR := core/include
STDLIB_INC := core/stdlib/include
BUILD_DIR = build
BIN_DIR := bin

# Source files
SOURCES_CORE := $(wildcard $(SRC_DIR)/*.cpp)
SOURCES_STDLIB := $(wildcard $(STDLIB_SRC)/*.cpp)
OBJECTS_CORE := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES_CORE))
OBJECTS_STDLIB := $(patsubst $(STDLIB_SRC)/%.cpp,$(BUILD_DIR)/stdlib/%.o,$(SOURCES_STDLIB))
OBJECTS := $(OBJECTS_CORE) $(OBJECTS_STDLIB)
OBJECTS_DEBUG_CORE := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/debug/%.o,$(SOURCES_CORE))
OBJECTS_DEBUG_STDLIB := $(patsubst $(STDLIB_SRC)/%.cpp,$(BUILD_DIR)/debug/stdlib/%.o,$(SOURCES_STDLIB))
OBJECTS_DEBUG := $(OBJECTS_DEBUG_CORE) $(OBJECTS_DEBUG_STDLIB)

# Target
TARGET := $(BIN_DIR)/roucaarize
TARGET_DEBUG := $(BIN_DIR)/roucaarize-debug

# Include path
INCLUDES := -I$(INC_DIR) -I$(STDLIB_INC)

# Default target
.PHONY: all
all: release

# Release build
.PHONY: release
release: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@
	@echo "Build complete: $@"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/stdlib/%.o: $(STDLIB_SRC)/%.cpp | $(BUILD_DIR)/stdlib
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Debug build
.PHONY: debug
debug: $(TARGET_DEBUG)

$(TARGET_DEBUG): $(OBJECTS_DEBUG) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS_DEBUG) $(OBJECTS_DEBUG) -o $@
	@echo "Debug build complete: $@"

$(BUILD_DIR)/debug/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)/debug
	$(CXX) $(CXXFLAGS_DEBUG) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/debug/stdlib/%.o: $(STDLIB_SRC)/%.cpp | $(BUILD_DIR)/debug/stdlib
	$(CXX) $(CXXFLAGS_DEBUG) $(INCLUDES) -c $< -o $@

# Create directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/stdlib:
	mkdir -p $(BUILD_DIR)/stdlib

$(BUILD_DIR)/debug:
	mkdir -p $(BUILD_DIR)/debug

$(BUILD_DIR)/debug/stdlib:
	mkdir -p $(BUILD_DIR)/debug/stdlib

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Clean complete"

# Rebuild
.PHONY: rebuild
rebuild: clean all

# Run interpreter
.PHONY: run
run: $(TARGET)
	./$(TARGET)

# Run with script
.PHONY: run-script
run-script: $(TARGET)
	@if [ -z "$(SCRIPT)" ]; then \
		echo "Usage: make run-script SCRIPT=path/to/script.rou"; \
	else \
		./$(TARGET) $(SCRIPT); \
	fi

# Run examples
.PHONY: run-examples
run-examples: $(TARGET)
	@echo "Running examples..."
	@for f in examples/*.rou; do \
		echo "Running $$f..."; \
		./$(TARGET) $$f; \
	done

# Install to /usr/local/bin
.PHONY: install
install: clean release
	install -m 755 $(TARGET) /usr/local/bin/roucaarize
	@echo "Installed to /usr/local/bin/roucaarize"

# Uninstall
.PHONY: uninstall
uninstall:
	rm -f /usr/local/bin/roucaarize
	@echo "Uninstalled roucaarize"

# Listing Target
list:
	@mkdir -p z_listing
	@echo "================================================================================" > z_listing/listing.txt
	@echo "ROUCAARIZE PROJECT CODEBASE LISTING" >> z_listing/listing.txt
	@echo "Generated on: $$(date)" >> z_listing/listing.txt
	@echo "================================================================================" >> z_listing/listing.txt
	@echo "" >> z_listing/listing.txt
	@echo "This document contains a comprehensive listing of all source code files within" >> z_listing/listing.txt
	@echo "the Roucaarize project, including core, stdlib, and examples." >> z_listing/listing.txt
	@echo "Roucaarize is a minimalist, tree-walking interpreter for Linux orchestration." >> z_listing/listing.txt
	@echo "" >> z_listing/listing.txt
	@echo "Directory Structure Overview:" >> z_listing/listing.txt
	@echo "1. core/include: Header files for the Lexer, Parser, and Evaluator." >> z_listing/listing.txt
	@echo "2. core/src: Implementation of the tree-walking interpreter logic." >> z_listing/listing.txt
	@echo "3. core/stdlib: Standard library modules (sys, fs, net, proc, string, time)." >> z_listing/listing.txt
	@echo "4. examples: Collection of example scripts demonstrating language features." >> z_listing/listing.txt
	@echo "" >> z_listing/listing.txt
	@echo "================================================================================" >> z_listing/listing.txt
	@echo "" >> z_listing/listing.txt
	@for f in $$(find core examples -type f | sort) Makefile; do \
		echo "FILE: $$f" >> z_listing/listing.txt; \
		echo "--------------------------------------------------------------------------------" >> z_listing/listing.txt; \
		cat "$$f" >> z_listing/listing.txt; \
		echo -e "\n\n" >> z_listing/listing.txt; \
	done
	@echo "Codebase listing generated at z_listing/listing.txt"

# Core Listing Target
core-list:
	@mkdir -p z_listing
	@echo "================================================================================" > z_listing/core_listing.txt
	@echo "ROUCAARIZE CORE CODEBASE LISTING" >> z_listing/core_listing.txt
	@echo "Generated on: $$(date)" >> z_listing/core_listing.txt
	@echo "================================================================================" >> z_listing/core_listing.txt
	@echo "" >> z_listing/core_listing.txt
	@echo "This document contains a listing of all source code files within the core/ directory." >> z_listing/core_listing.txt
	@echo "Roucaarize is a minimalist, tree-walking interpreter for Linux orchestration." >> z_listing/core_listing.txt
	@echo "" >> z_listing/core_listing.txt
	@echo "================================================================================" >> z_listing/core_listing.txt
	@echo "" >> z_listing/core_listing.txt
	@for f in $$(find core -type f | sort); do \
		echo "FILE: $$f" >> z_listing/core_listing.txt; \
		echo "--------------------------------------------------------------------------------" >> z_listing/core_listing.txt; \
		cat "$$f" >> z_listing/core_listing.txt; \
		echo -e "\n\n" >> z_listing/core_listing.txt; \
	done
	@echo "Core codebase listing generated at z_listing/core_listing.txt"

# Help
.PHONY: all release debug clean rebuild run run-script run-examples install uninstall help list core-list
help:
	@echo "Roucaarize Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all          Build release version (default)"
	@echo "  release      Build optimized interpreter"
	@echo "  debug        Build debug version with symbols"
	@echo "  clean        Remove build artifacts"
	@echo "  rebuild      Clean and rebuild"
	@echo "  run          Run without script"
	@echo "  run-script   Run script: make run-script SCRIPT=file.rou"
	@echo "  run-examples Run all example scripts"
	@echo "  list         Generate codebase listing"
	@echo "  core-list    Generate core-only listing"
	@echo "  install      Install to /usr/local/bin"
	@echo "  help         Show this help"

# Include generated dependencies
-include $(OBJECTS:.o=.d)
-include $(OBJECTS_DEBUG:.o=.d)
