# Targets for running and debugging in Qemu/KVM

QEMUCPUS ?= 4
INITRD ?= /dev/null
QEMUSERIAL ?= pty
QEMUFLAGS = -k en-us -serial $(QEMUSERIAL) -d guest_errors -m 2048
# According to qemu(1): "Creates a backend using PulseAudio. This backend is
# available on most systems." So we use pa as audiodev.
QEMUFLAGS += -audiodev pa,id=stubsad -machine pcspk-audiodev=stubsad
KVMFLAGS = -enable-kvm -cpu host $(QEMUFLAGS)
DBGFLAGS = -no-shutdown -no-reboot -monitor vc
DBGKERNEL ?= $(KERNEL64)
DBGARCH ?= i386:x86-64
QEMU ?= qemu-system-x86_64
QEMUKERNEL := -kernel $(KERNEL) -initrd $(INITRD)
GDB = $(PREFIX)gdb
GDBFLAG = --eval-command="source tools/gdb/stubs.py"

# Switch to curses if no graphical output is available
ifeq ($(DISPLAY),)
	QEMUFLAGS += -display curses
else
	# check if qemu has the 'show-tabs' option
	QEMU_CAP_TABS := $(shell $(QEMU) -display gtk,show-tabs=on -h 2>/dev/null 1>/dev/null && echo 'show-tabs=on')
	QEMUFLAGS += -display $(subst $(space),$(comma),gtk $(QEMU_CAP_TABS))
endif

# Run the kernel in Qemu
qemu: all
	$(QEMU) $(QEMUKERNEL) -smp $(QEMUCPUS) $(QEMUFLAGS)

# qemu must wait for the debugger to attach, or a 'continue'-command provided via the monitor
%-wait: QEMUFLAGS += -S $(DBGFLAGS)
DBGFLAGS = -no-shutdown -no-reboot -qmp unix:qmp.sock,server=on,wait=off -monitor vc
%-wait: QEMUFLAGS += -S -chardev socket,path=dbg.sock,server=on,wait=off,id=gdb0 -gdb chardev:gdb0 $(DBGFLAGS)
kvm-wait: kvm
qemu-wait: qemu

# Execute Qemu with activated GDB stub and directly connect GDB to the spawned Qemu.
qemu-gdb: all
	$(GDB) $(GDBFLAG) $(DBGKERNEL) \
		-ex "set arch $(DBGARCH)" \
		-ex "target remote | exec $(QEMU) -gdb stdio $(QEMUKERNEL) -smp $(QEMUCPUS) -S $(QEMUFLAGS) $(DBGFLAGS)"

gdb: all
	$(GDB) $(GDBFLAG) $(DBGKERNEL) \
		-ex "set arch $(DBGARCH)" \
		-ex "target remote dbg.sock"

# Runs StuBS in Qemu with with hardware accelerations (KVM support) enabled
# The started emulator provides several virtual CPUs that execute in parallel.
kvm: all
	$(QEMU) $(QEMUKERNEL) -smp $(QEMUCPUS) $(KVMFLAGS)

# Executes Qemu with KVM suppot with activated GDB stub
# and directly connect GDB to the spawned Qemu.
# Please note: Software breakpoints may not work before the stubs kernel
# has switched to long mode -- so we use a hardware breakpoint to stop
# at `kernel_init` (the C++ entry point)
kvm-gdb: all
	$(GDB) $(GDBFLAG) $(DBGKERNEL) \
		-ex "set arch $(DBGARCH)" \
		-ex "target remote | exec $(QEMU) -gdb stdio $(QEMUKERNEL) -smp $(QEMUCPUS) -S $(KVMFLAGS) $(DBGFLAGS)" \
		-ex "hbreak kernel_init" \
		-ex "continue"

# Help for Qemu targets
help::
	@/bin/echo -e "" \
		"	\e[3mqemu\e[0m     Starts $(PROJECT) in QEMU\n" \
		"	         Due to the internal design of QEMU, some things (especially\n" \
		"	         race conditions) might behave different compared to hardware!\n\n" \
		"	\e[3mqemu-gdb\e[0m Starts $(PROJECT) in QEMU with internal GDB stub and attaches\n" \
		"	         it to a GDB session allowing step-by-step debugging\n\n" \
		"	\e[3mqemu-wait\e[0m Starts $(PROJECT) with QEMU waiting for an debugger to attach\n\n" \
		"	\e[3mkvm\e[0m      Starts $(PROJECT) in KVM, a hardware-accelerated virtual machine\n\n" \
		"	\e[3mkvm-gdb\e[0m  Same as \e[3mqemu-gdb\e[0m, but with hardware acceleration\n\n" \
		"	\e[3mkvm-wait\e[0m Same as \e[3mqemu-wait\e[0m, but with hardware acceleration\n\n" \
		"	\e[3mgdb\e[0m Attaches the debugger to an already waiting QEMU (\e[3mqemu-wait\e[0m|\e[3mkvm-wait\e[0m)\n"

# Phony targets
.PHONY: qemu qemu-wait kvm kvm-wait gdb qemu-gdb kvm-gdb help
