# Nuke built-in rules and variables.
MAKEFLAGS += -rR
.SUFFIXES:

# This is the name that our final executable will have.
# Change as needed.
override OUTPUT := kernel

# User controllable flags. Passed from the main makefile.
ARCH ?=
BUILD ?= 

CC ?=
CFLAGS ?= 
CPPFLAGS ?=
LD ?=
LDFLAGS ?=

ifeq ($(ARCH),x86_64)
    NASMFLAGS := -F dwarf -g
endif

# Check if CC is Clang.
override CC_IS_CLANG := $(shell ! $(CC) --version 2>/dev/null | grep -q '^Target: '; echo $$?)

# Internal C flags that should not be changed by the user. 
override CFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu23 \
    -nostdinc \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-PIC \
    -ffunction-sections \
    -fdata-sections \
    -pipe

# Internal C preprocessor flags that should not be changed by the user. # freestnd-c-hdrs/include 
override CPPFLAGS := \
    -I third_party/libfdt/src \
    -I limine \
    -I kernel/src \
    -isystem third_party/freestnd_c_hdrs/include \
    -isystem nostdinc \
    -isystem kernel/src/lib
    $(CPPFLAGS) \
    -DLIMINE_API_REVISION=3 \
    -MMD \
    -MP

ifeq ($(ARCH),x86_64)
    # Internal nasm flags that should not be changed by the user.
    override NASMFLAGS += \
        -Wall
endif

# Architecture specific internal flags.
ifeq ($(ARCH),x86_64)
    ifeq ($(CC_IS_CLANG),1)
        override CC += \
            -target x86_64-unknown-none-elf
    endif
    override CFLAGS += \
        -m64 \
        -march=x86-64 \
        -mabi=sysv \
        -mno-80387 \
        -mno-mmx \
        -mno-sse \
        -mno-sse2 \
        -mno-red-zone \
        -mcmodel=kernel
    override LDFLAGS += \
        -m elf_x86_64
    override NASMFLAGS := \
        -f elf64 \
        $(NASMFLAGS)
endif
ifeq ($(ARCH),aarch64)
    ifeq ($(CC_IS_CLANG),1)
        override CC += \
            -target aarch64-unknown-none-elf
    endif
    override CFLAGS += \
        -mcpu=generic \
        -march=armv8-a+nofp+nosimd \
        -mgeneral-regs-only
    override LDFLAGS += \
        -m aarch64elf
endif
ifeq ($(ARCH),riscv64)
    ifeq ($(CC_IS_CLANG),1)
        override CC += \
            -target riscv64-unknown-none-elf
        override CFLAGS += \
            -march=rv64imac
    else
        override CFLAGS += \
            -march=rv64imac_zicsr_zifencei
    endif
    override CFLAGS += \
        -mabi=lp64 \
        -mno-relax
    override LDFLAGS += \
        -m elf64lriscv \
        --no-relax
endif
ifeq ($(ARCH),loongarch64)
    ifeq ($(CC_IS_CLANG),1)
        override CC += \
            -target loongarch64-unknown-none-elf
    endif
    override CFLAGS += \
        -march=loongarch64 \
        -mabi=lp64s \
        -mfpu=none \
        -msimd=none \
        -mno-relax
    override LDFLAGS += \
        -m elf64loongarch \
        --no-relax
endif

# Internal linker flags that should not be changed by the user.
override LDFLAGS += \
    -nostdlib \
    -static \
    -z max-page-size=0x1000 \
    --gc-sections \
    -T linker_scripts/$(ARCH).lds

# Use "find" to glob all *.c, *.S, and *.asm files in the tree and obtain the
# object and header dependency file names.
override THIRD_PARTY := $(shell find third_party -type f -name '*.c' | grep -v '^third_party/cc_runtime/')
override NOSTDINC := $(shell find nostdinc -type f -name '*.c' | LC_ALL=C sort)
override CC_RUNTIME := $(shell find -L third_party/cc_runtime -type f -name '*.c' | LC_ALL=C sort)
override KERNEL_SRC := $(shell find kernel/src -type f -name '*.c' ! -path 'kernel/src/arch/*')
override ARCH_SRCS := $(shell find kernel/src/arch/$(ARCH) -type f -name '*.c' | LC_ALL=C sort)

override CFILES := $(NOSTDINC) $(THIRD_PARTY) $(CC_RUNTIME) $(KERNEL_SRC) $(ARCH_SRCS)

override ASFILES := $(filter %.S,$(SRCFILES))
override ASMFILES := $(shell find kernel/src -name '*.S' ! -path "src/kernel/arch/*/*")
override ASMFILES += $(shell find kernel/src/arch/$(ARCH) -name '*.S')

ifeq ($(ARCH),x86_64)
override NASMFILES := $(filter %.asm,$(SRCFILES))
endif
override OBJ := $(addprefix obj_$(BUILD)_$(ARCH)/,$(CFILES:.c=.c.o) $(ASFILES:.S=.S.o))
ifeq ($(ARCH),x86_64)
override OBJ += $(addprefix obj_$(BUILD)_$(ARCH)/,$(NASMFILES:.asm=.asm.o))
endif
override HEADER_DEPS := $(addprefix obj_$(BUILD)_$(ARCH)/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))

# Default target. This must come first, before header dependencies.
.PHONY: all
all: bin_$(BUILD)_$(ARCH)/$(OUTPUT)

# Include header dependencies.
-include $(HEADER_DEPS)

# Link rules for the final executable.
bin_$(BUILD)_$(ARCH)/$(OUTPUT): Makefile kernel/linker_scripts/$(ARCH).lds $(OBJ)
	mkdir -p "$$(dirname $@)"
	$(LD) $(OBJ) $(LDFLAGS) -o $@

# Compilation rules for *.c files.
obj_$(BUILD)_$(ARCH)/%.c.o: %.c Makefile
	mkdir -p "$$(dirname $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Compilation rules for *.S files.
obj_$(BUILD)_$(ARCH)/%.S.o: %.S Makefile
	mkdir -p "$$(dirname $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

ifeq ($(ARCH),x86_64)
# Compilation rules for *.asm (nasm) files.
obj_$(BUILD)_$(ARCH)/%.asm.o: %.asm Makefile
	mkdir -p "$$(dirname $@)"
	nasm $(NASMFLAGS) $< -o $@
endif

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf bin_$(BUILD)_$(ARCH) obj_$(BUILD)_$(ARCH)

# Remove everything built and generated including downloaded dependencies.
.PHONY: distclean
distclean:
	rm -rf bin_* obj_* 