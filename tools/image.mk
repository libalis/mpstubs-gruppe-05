# Generates a bootable ISO image that can be transferred to external media, such as CDs or USB sticks.
# This will install, in addition to your kernel, the bootloader GRUB (https://www.gnu.org/software/grub/).
#
# The target 'gemu-iso' is used to test the image generated with 'iso'.
#
# Assuming that a USB mass-storage devices is connected as, for instance /dev/sdc, the target 'usb-sdc'
# can be used to make your device bootable (requires root access, substitute sdc with the matching device).
# Alternatively, you can burn the .iso file directly to CD.

DD = dd
XORRISO = xorriso
MKISO = grub-mkrescue

ISODIR = $(BUILDDIR)-iso
ISOGRUBCFG = boot/grub/grub.cfg
ISOKERNEL = boot/kernel
ISOINITRD = initrd
GRUBTITLE = $(shell id -un)s $(PROJECT)
GRUBTIMEOUT = 2
GRUBBIN = /usr/lib/grub/i386-pc
ifeq (,$(wildcard $(GRUBBIN)))
	GRUBBIN = /proj/i4stubs/tools/grub-i386-pc
endif

# Default ISO target
iso: $(ISOFILE)

# Create Grub config
$(ISODIR)/$(ISOGRUBCFG):
	@echo "GEN		$@"
	@mkdir -p $(dir $@)
	@/bin/echo -e "set timeout=$(GRUBTIMEOUT)\nset default=0\n\nmenuentry \"$(GRUBTITLE)\" {\n\tmultiboot /$(ISOKERNEL)\n\tmodule /$(ISOINITRD)\n\tboot\n}" > $@

# Strip debug symbols from kernel binary
$(ISODIR)/$(ISOKERNEL): all
	@echo "STRIP		$@"
	@mkdir -p $(dir $@)
	$(VERBOSE) $(STRIP) --strip-debug --strip-unneeded -p -o $@ $(KERNEL)

# copy inital ramdisk
$(ISODIR)/$(ISOINITRD): all
	@echo "CPY		$@"
	@mkdir -p $(dir $@)
	@if [ -s $(INITRD) ] ; then cp -a $(INITRD) $@ ; else touch $@ ; fi

# Pack to ISO
$(ISOFILE): $(ISODIR)/$(ISOKERNEL) $(ISODIR)/$(ISOINITRD) $(ISODIR)/$(ISOGRUBCFG)
	@echo "ISO		$@"
	@which $(XORRISO) >/dev/null || echo "Xorriso cannot be found - if building the ISO fails, this may be the reason!" >&2
	$(VERBOSE) $(MKISO) -d $(GRUBBIN) -o $@ $(ISODIR)

# Run ISO in Qemu
qemu-iso: $(ISOFILE)
	$(QEMU) -cdrom $< -smp $(QEMUCPUS) $(QEMUFLAGS)

# Run ISO in KVM
kvm-iso: $(ISOFILE)
	$(QEMU) -cdrom $< -smp $(QEMUCPUS) $(KVMFLAGS)

# Copy ISO to USB device
usb: $(ISOFILE)
ifeq (,$(USBDEV))
	@echo "The environment variable USBDEV must contain the path to the USB mass-storage device:" >&2
	@lsblk -o TYPE,KNAME,SIZE,MODEL -a -p | grep "^disk" | cut -b 6-
	@exit 1
else
	$(VERBOSE) $(DD) if=$< of=$(USBDEV) bs=4M status=progress && sync
endif

# Shorthand to copy ISO to a specific USB device
usb-%:
	@$(MAKE) USBDEV=/dev/$* usb

# Burn ISO to CD
cd: $(ISOFILE)
ifeq (,$(CDRWDEV))
	@echo "The environment variable CDRWDEV must contain the path to the CD/DVD writer" >&2
	@exit 1
else
	$(VERBOSE) $(XORRISO) -as cdrecord -v dev=$(CDRWDEV) -dao $<
endif

# Shorthand to nurn ISO to specific CD device
cd-%:
	@$(MAKE) CDRWDEV=/dev/$* cd

# The standard target 'clean' removes the whole generated system, the object files, and the dependency files.
clean::
	@echo "RM		$(ISODIR)"
	$(VERBOSE) rm -rf "$(ISODIR)" "$(ISODIR)$(OPTTAG)" "$(ISODIR)$(NOOPTTAG)" "$(ISODIR)$(DBGTAG)" "$(ISODIR)$(VERBOSETAG)"

# Documentation
help::
	@/bin/echo -e "" \
		"	\e[3miso\e[0m      Generates a bootable system image (File: $(ISOFILE))\n\n" \
		"	\e[3mqemu-iso\e[0m Starts the system in QEMU by booting from the virtual CD drive\n\n" \
		"	\e[3mkvm-iso\e[0m  Same as \e[3mqemu-iso\e[0m, but with hardware acceleration\n\n" \
		"	\e[3musb\e[0m      Generates a bootable USB mass-storage device; the environment\n" \
		"	         variable \e[4mUSBDEV\e[0m should point to the USB device\n\n" \
		"	\e[3mcd\e[0m       Generates a bootable CD; the environment variable \e[4mCDRWDEV\e[0m\n" \
		"	         should point to the CD writer\n\n"

# Phony targets
.PHONY: iso qemu-iso kvm-iso cd usb help
