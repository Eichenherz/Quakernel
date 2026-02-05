# Nuke built-in rules and variables.
MAKEFLAGS += -rR
.SUFFIXES:

# Target architecture to build for.
ARCH := riscv64

# Check if the architecture is supported.
ifeq ($(filter $(ARCH), riscv64 ),)
    $(error Architecture $(ARCH) not supported)
endif

# Build modes
BUILD ?= ReleaseWithDebugInfo

ifeq ($(filter $(BUILD),Release ReleaseWithDebugInfo Debug),)
    $(error $(BUILD) not supported)
endif

override IMAGE_NAME := "Quakernel_$(BUILD)_$(ARCH)"

#HOST_LIBS :=

# Compiler flags per build mode
ifeq ($(BUILD),Release)
  CFLAGS := -O3 -DNDEBUG
  CPPFLAGS :=
  LDFLAGS :=
endif

ifeq ($(BUILD),ReleaseWithDebugInfo)
  CFLAGS := -O3 -g
  CPPFLAGS :=
  LDFLAGS :=
endif

ifeq ($(BUILD),Debug)
  CFLAGS := -O0 -g
  CPPFLAGS :=
  LDFLAGS :=
endif

# User controllable toolchain.
TOOLCHAIN ?= llvm

# User controllable linker command.
LD := ld

# Determine CC based on TOOLCHAIN
ifeq ($(TOOLCHAIN),)
    CC := cc
else ifeq ($(TOOLCHAIN),gcc)
    CC := gcc
else ifeq ($(TOOLCHAIN),llvm)
    CC := clang
    LD := ld.lld
else
    # If TOOLCHAIN is something else, assume it's a prefix or full compiler name
    CC := $(TOOLCHAIN)
endif



.PHONY: build-iso
build-iso: $(IMAGE_NAME).iso

.PHONY: build-hdd
build-hdd: $(IMAGE_NAME).hdd

limine/limine:
	$(MAKE) -C limine \
		CC="$(CC)" \
		CFLAGS="$(CFLAGS)" \
		CPPFLAGS="$(CPPFLAGS)" \
		LDFLAGS="$(LDFLAGS)" 
# LIBS="$(HOST_LIBS)"

.PHONY: kernel
kernel:
	$(MAKE) -C . -f kernel/Makefile \
		CC="$(CC)" CFLAGS="$(CFLAGS)" CPPFLAGS="$(CPPFLAGS)" \
		LD="$(LD)" LDFLAGS="$(LDFLAGS)" \
		ARCH="$(ARCH)" BUILD="$(BUILD)"


BOOT_EFI_x86_64 := BOOTX64.EFI
BOOT_EFI_aarch64 := BOOTAA64.EFI
BOOT_EFI_riscv64 := BOOTRISCV64.EFI
BOOT_EFI_loongarch64 := BOOTLOONGARCH64.EFI
BOOT_EFI := $(BOOT_EFI_$(ARCH))

OUTDIR ?= Images

$(IMAGE_NAME).iso: limine/limine kernel
	rm -rf iso_root
	mkdir -p iso_root/boot
	cp -v kernel/bin_$(BUILD)_$(ARCH)/kernel iso_root/boot/
	mkdir -p iso_root/boot/limine
	cp -v limine.conf iso_root/boot/limine/
	cp -v limine/limine-uefi-cd.bin iso_root/boot/limine/
	mkdir -p iso_root/EFI/BOOT
	cp -v limine/$(BOOT_EFI) iso_root/EFI/BOOT/
	mkdir -p $(OUTDIR)
	xorriso -as mkisofs -R -r -J \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(OUTDIR)/$(IMAGE_NAME).iso
	rm -rf iso_root

$(IMAGE_NAME).hdd: limine/limine kernel
	mkdir -p $(OUTDIR)
	rm -f $(OUTDIR)/$(IMAGE_NAME).hdd
	dd if=/dev/zero bs=1M count=0 seek=64 of=$(OUTDIR)/$(IMAGE_NAME).hdd
	PATH=$$PATH:/usr/sbin:/sbin sgdisk $(OUTDIR)/$(IMAGE_NAME).hdd -n 1:2048 -t 1:ef00
	mformat -i $(OUTDIR)/$(IMAGE_NAME).hdd@@1M
	mmd -i $(OUTDIR)/$(IMAGE_NAME).hdd@@1M ::/EFI ::/EFI/BOOT ::/boot ::/boot/limine
	mcopy -i $(OUTDIR)/$(IMAGE_NAME).hdd@@1M bin_$(BUILD)_$(ARCH)/kernel ::/boot
	mcopy -i $(OUTDIR)/$(IMAGE_NAME).hdd@@1M limine.conf ::/boot/limine
	mcopy -i $(OUTDIR)/$(IMAGE_NAME).hdd@@1M limine/$(BOOT_EFI) ::/EFI/BOOT

.PHONY: clean
clean:
	$(MAKE) -C kernel clean
	rm -rf iso_root $(OUTDIR)/$(IMAGE_NAME).iso $(OUTDIR)/$(IMAGE_NAME).hdd

.PHONY: distclean
distclean:
	$(MAKE) -C kernel distclean
	rm -rf iso_root *.iso *.hdd kernel-deps limine