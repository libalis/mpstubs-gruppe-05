# use the i4 Infrastructure

SOLUTIONDIR = /proj/i4stubs/solution

SOLUTIONPREFIX = musterloesung-m

SOLUTIONTMPDIR = .solution

NETBOOTDIR = /proj/i4stubs/student
# Will use either the username from the ssh configuration or, by default, `whoami`
NETBOOTSSH ?= cipterm0.cip.cs.fau.de

HALLOFFAMESRC = /proj/i4stubs/halloffame/halloffame.iso
HALLOFFAMEISO = $(SOLUTIONTMPDIR)/halloffame.iso

# Fetch an ISO including all hall of fame images
$(HALLOFFAMEISO):
	$(VERBOSE) echo "Get Hall-Of-Fame ISO" ; \
	mkdir -p $(SOLUTIONTMPDIR) ; \
	if [ -f  "$(HALLOFFAMESRC)" ] ; then \
		cp $(HALLOFFAMESRC) $@ ; \
	else \
		echo "via SSH $(NETBOOTSSH)" ; \
		scp $(NETBOOTSSH):$(HALLOFFAMESRC) $@ ; \
	fi

# 'halloffame' starts an ISO including all hall of fame images
# with 4 cores and KVM virtualization
halloffame: $(HALLOFFAMEISO)
	$(QEMU) -cdrom $< -smp $(QEMUCPUS) $(KVMFLAGS) ; \

# 'halloffame-old' starts an ISO including all hall of fame images
# in compatibility mode (single core, software emulation)
halloffame-old: $(HALLOFFAMEISO)
	$(QEMU) -cdrom $< ; \

# The target 'netboot' copies the resulting StuBS kernel to the base directory
# of our tftp server. The tftp server enables the text systems to boot the image
# via pxelinux.
netboot: all
	$(VERBOSE) initrd="$(INITRD)" ; \
	if [ ! -s "$$initrd" ] ; then \
		initrd="$(BUILDDIR)/fake-initrd" ; \
		echo "(none)" > "$$initrd" ; \
	fi ; \
	if [ -d  "$(NETBOOTDIR)" ] ; then \
		echo "CPY		$(NETBOOTDIR)/$(shell id -run)/kernel" ; \
		install -m 644 $(KERNEL) $(NETBOOTDIR)/$(shell id -run)/kernel ; \
		install -m 644 "$$initrd" $(NETBOOTDIR)/$(shell id -run)/initrd.img ; \
	else \
		echo "via SSH $(NETBOOTSSH)" ; \
		echo "SSH		$(NETBOOTSSH)@$(NETBOOTDIR)/$(shell id -run)/kernel" ; \
		tar --mode="u=rw,og=r" --transform='flags=r;s|$(KERNEL)|kernel|' --transform="flags=r;s|$$initrd|initrd.img|" -cz $(KERNEL) "$$initrd" | \
		ssh "$(NETBOOTSSH)" "cat - | tar -xvzp -C $(NETBOOTDIR)/\`id -run\`/" ; \
	fi

# 'solution-1' starts our solution for exercise 1 using KVM.
# Of course, other exercise numbers can be supplied, too.
solution-%:
	$(VERBOSE) echo "Solution for $(PROJECT) assignment $*" ; \
	if [ -d  "$(SOLUTIONDIR)" ] ; then \
		$(QEMU) -kernel $(SOLUTIONDIR)/$(SOLUTIONPREFIX)$*.elf -initrd $(SOLUTIONDIR)/$(SOLUTIONPREFIX)$*.rd -smp $(QEMUCPUS) $(KVMFLAGS) ; \
	else \
		echo "via SSH $(NETBOOTSSH)" ; \
		mkdir -p $(SOLUTIONTMPDIR) ; \
		scp $(NETBOOTSSH):$(SOLUTIONDIR)/$(SOLUTIONPREFIX)$*.{elf,rd} $(SOLUTIONTMPDIR) && \
		$(QEMU) -kernel $(SOLUTIONTMPDIR)/$(SOLUTIONPREFIX)$*.elf -initrd $(SOLUTIONTMPDIR)/$(SOLUTIONPREFIX)$*.rd -smp $(QEMUCPUS) $(KVMFLAGS) ; \
	fi

# The standard target 'clean' removes the temporary solution directory
clean::
	@echo "RM		$(SOLUTIONTMPDIR)"
	$(VERBOSE) rm -rf "$(SOLUTIONTMPDIR)"

# Documentation
help::
	@/bin/echo -e "" \
		"	\e[3mnetboot\e[0m  Copies $(PROJECT) to the network share, allowing the test systems\n" \
		"	         to boot your system\n\n\n" \
		"Apart from the above targets that run your implementation, our solution can\n" \
		"be run in KVM (at least when called in the CIP) using the target\n\n" \
		"	\e[3msolution-\e[2;3mexercise\e[0m\n\n" \
		"where \e[2;3mexercise\e[0m is the number of the exercise whose solution should be executed.\n\n"

# Phony targets
.PHONY: netboot help
