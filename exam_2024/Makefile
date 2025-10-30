# Compiler and Flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -I src/
OPTFLAGS = -O2 -DNDEBUG
DEBUGFLAGS = -O0 -g
LTOFLAGS = -flto
LIBS = -lexpat -pthread

# Project Structure
SRC_DIR = src
RELEASE_DIR = bin
DEBUG_DIR = debug

# Source Files and Headers
SRCS = $(wildcard $(SRC_DIR)/**/*.cpp $(SRC_DIR)/*.cpp)
HEADERS = $(wildcard $(SRC_DIR)/**/*.hh $(SRC_DIR)/*.hh)

# Object Files
RELEASE_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(RELEASE_DIR)/%.o,$(SRCS))
DEBUG_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(DEBUG_DIR)/%.o,$(SRCS))

# Targets
all: release debug

release: $(RELEASE_DIR)/mc

debug: $(DEBUG_DIR)/mc

# Release Binary
$(RELEASE_DIR)/mc: $(RELEASE_OBJS)
	@mkdir -p $(RELEASE_DIR)
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) $(LTOFLAGS) -o $@ $(RELEASE_OBJS) $(LIBS)

# Debug Binary
$(DEBUG_DIR)/mc: $(DEBUG_OBJS)
	@mkdir -p $(DEBUG_DIR)
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -o $@ $(DEBUG_OBJS) $(LIBS)

# Compile Release Object Files
$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)  # Create necessary subdirectories for object files
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) $(LTOFLAGS) -c $< -o $@

# Compile Debug Object Files
$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)  # Create necessary subdirectories for object files
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -c $< -o $@

# Clean Up
clean:
	rm -rf $(RELEASE_DIR) $(DEBUG_DIR)

.PHONY: all release debug clean
