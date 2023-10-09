VERBOSE = @
OBJDIR = build
CXX = g++
MKDIR = mkdir
CC_SOURCES = ../test-stream/console_out.cc ../test-stream/test.cc ../test-stream/file_out.cc ../object/outputstream.cc ../object/stringbuffer.cc
CXXFLAGS = -std=c++11 -m64 -I../object -I.
TARGET = $(OBJDIR)/test

all: run

run: $(TARGET)
	@./$<

$(TARGET): $(CC_SOURCES)
	$(VERBOSE) $(MKDIR) -p $(OBJDIR)
	$(VERBOSE) $(CXX) -o $@ $(CXXFLAGS) $^

clean:
	@echo "RM		$(OBJDIR)"
	$(VERBOSE) rm -rf $(OBJDIR)

.PHONY: all run clean
