# Common include Makefile

# Hide commands
VERBOSE = @
# Prefix for toolchain binaries
PREFIX ?=
# Project name
PROJECT ?= "MPStuBS"
empty:=
space:= $(empty) $(emtpy)
comma:= ,

help::
	@/bin/echo -e "\n" \
		"\e[1mMAKEFILE for the teaching operating system $(PROJECT)\e[0m\n" \
		"--------------------------------------------------\n\n" \
		"Executing '\e[4mmake\e[0m' will compile the operating system from source.\n"

# Get current directory path
CURRENT_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

# Include Makefile scripts
include $(CURRENT_DIR)/build.mk
include $(CURRENT_DIR)/qemu.mk
include $(CURRENT_DIR)/image.mk
include $(CURRENT_DIR)/linter.mk
include $(CURRENT_DIR)/remote.mk

# Disable buitlin rules
MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-builtin-variables

# Disable buitlin suffixes
.SUFFIXES:
