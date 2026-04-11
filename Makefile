
# ==============================================================================
# NES_Emulator - Makefile
# ==============================================================================
 
CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
TARGET   := NES_Emulator
SRCDIR   := src
OBJDIR   := obj
 
# Auto-detect all .cpp files in src/ - just drop new files in and make will pick them up
SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))
 
# Platform detection
UNAME := $(shell uname -s)
 
ifeq ($(UNAME), Linux)
    LIBS := -lGL -lX11 -lpthread -lpng -lstdc++fs -lasound
endif
ifeq ($(UNAME), Darwin)
    LIBS := -framework OpenGL -framework Cocoa -lpthread -lpng
endif
 
# ==============================================================================
 
.PHONY: all clean debug
 
all: $(TARGET)
 
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo ""
	@echo "  Build successful! Run with: ./$(TARGET) <path-to-rom.nes>"
	@echo ""
 
# Compile each .cpp into obj/
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
 
# Create obj/ directory if it doesn't exist
$(OBJDIR):
	mkdir -p $(OBJDIR)
 
debug: CXXFLAGS += -g -DDEBUG -O0
debug: $(TARGET)
 
clean:
	rm -rf $(OBJDIR) $(TARGET)
	@echo "Cleaned."
