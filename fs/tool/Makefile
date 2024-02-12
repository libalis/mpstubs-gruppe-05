VERBOSE = @
BINARY = fstool
OBJDIR = ./.build
CXXFLAGS = -std=c++11 -O2 -Wall -Wextra -Wno-error=unused-parameter -D_GNU_SOURCE -DNDEBUG -I ../../
CC_SOURCES = $(shell find .. -name "*.cc" -and ! -name '*disk.cc')
CC_OBJECTS = $(notdir $(CC_SOURCES:.cc=.o))
DEP_FILES = $(patsubst %.o,$(OBJDIR)/%.d,$(CC_OBJECTS))
OBJPRE = $(addprefix $(OBJDIR)/,$(ASM_OBJECTS) $(CC_OBJECTS))

VPATH = $(sort $(dir $(CC_SOURCES)))

all: $(BINARY)

# Rezept zum Übersetzen des Programms
$(BINARY): $(OBJPRE) $(MAKEFILE_LIST)
	@echo "CXX		$@"
	@if test \( ! \( -d $(@D) \) \) ;then mkdir -p $(@D);fi
	$(VERBOSE) $(CXX) $(CXXFLAGS) -o $@ $(OBJPRE)

# Standardrezept zur Erzeugung der Abhaengigkeitsdateien
$(OBJDIR)/%.d : %.cc $(MAKEFILE_LIST)
	@echo "DEP		$@"
	@if test \( ! \( -d $(@D) \) \) ;then mkdir -p $(@D);fi
	$(VERBOSE) $(CXX) $(CXXFLAGS) -MM -MT $(OBJDIR)/$*.o -MF $@ $<

# Standardrezept zur Erzeugung von Objektdateien
$(OBJDIR)/%.o : %.cc $(MAKEFILE_LIST)
	@echo "CXX		$@"
	@if test \( ! \( -d $(@D) \) \) ;then mkdir -p $(@D);fi
	$(VERBOSE) $(CXX) -c $(CXXFLAGS) -o $@ $<

# --------------------------------------------------------------------------
# Standardrezept 'clean' loescht das generierte System, die Objektdateien und
# die Abhaengigkeitsdateien

clean:
	@echo "RM		$(OBJDIR)"
	$(VERBOSE) rm -rf "$(OBJDIR)"

mrproper: clean
	@echo "RM		$(BINARY)"
	$(VERBOSE) rm -f "$(BINARY)"

ifneq ($(MAKECMDGOALS),clean)
-include $(DEP_FILES)
endif

.PHONY: all clean mrproper
