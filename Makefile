# Kernel Makefile
# try `make help` for more information

# Default target
.DEFAULT_GOAL = all

# Path to the files for the initial ramdisk (for Assignment 7)
INITRD_DIR ?= initrd/
INITRD_TOOL ?= fs/tool/fstool
INITRD_DEP =
# 1MB free space
INITRD_FREE ?= 1048576

# Kernel source files
LINKER_SCRIPT = compiler/sections.ld
CRTI_SOURCE = compiler/crti.asm
CRTN_SOURCE = compiler/crtn.asm
CC_SOURCES = $(shell find * -name "*.cc" -a ! -name '.*' -a ! -path 'test*' -a ! -path 'fs/tool/*')
ASM_SOURCES = $(shell find * -name "*.asm"  -a ! -name '.*')

# Target files
KERNEL = $(BUILDDIR)/system
KERNEL64 = $(KERNEL)64
ISOFILE = $(BUILDDIR)/stubs.iso

# Include global variables and standard recipes
include tools/common.mk

# Initial Ramdisk
ifneq ($(wildcard $(INITRD_DIR)*),)
INITRD = $(BUILDDIR)/initrd.img
INITRD_DEP += $(shell find $(INITRD_DIR) -type f )
# Additional dependency for kernel
$(KERNEL): $(INITRD)
endif

all: $(KERNEL)

# Linking the system image
# We use the C++ compiler (which calls the actual linker)
$(KERNEL64): $(CRTI_OBJECT) $(CRTN_OBJECT) $(ASM_OBJECTS) $(CC_OBJECTS) $(LINKER_SCRIPT) $(MAKEFILE_LIST)
	@echo "LD		$@"
	@mkdir -p $(@D)
	$(VERBOSE) $(CXX) $(CXXFLAGS) -Wl,-T $(LINKER_SCRIPT) -o $@ $(CRTI_OBJECT) $(CRTBEGIN_OBJECT) $(ASM_OBJECTS) $(CC_OBJECTS) $(LDFLAGS) $(LIBGCC) $(CRTEND_OBJECT) $(CRTN_OBJECT)

# The kernel must be a 32bit elf for multiboot compliance
$(KERNEL): $(KERNEL64)
	$(VERBOSE) $(OBJCOPY) -I elf64-x86-64 -O elf32-i386 $< $@

# Tool for editing a Minix v3 file system image (Assignment 7)
$(INITRD_TOOL): $(shell test -d $(dir $(INITRD_TOOL)) && find $(dir $(INITRD_TOOL)) -name "*.cc" -or -name '*.h')
	@echo "Make		$@"
	$(VERBOSE) MAKEFLAGS="" $(MAKE) -C $(dir $(INITRD_TOOL))

# Initial Ramdisk with Minix v3 file system
$(INITRD): $(INITRD_TOOL) $(INITRD_DEP)
	@echo "INITRD		$@"
	$(VERBOSE) dd if=/dev/zero of=$@ bs=$(shell du -s $(INITRD_DIR) | cut -f1 | xargs expr $(INITRD_FREE) + ) count=1
	$(VERBOSE) /sbin/mkfs.minix -3 $@  # optional --inodes <number>
	$(VERBOSE) ./$(INITRD_TOOL) put "$(INITRD_DIR)" $@
