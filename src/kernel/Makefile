# Inherit from parent
# mstrict align needed or otherwise it breaks interestingly enough
CFLAGS += -Iinclude -I../libc/include -mstrict-align -O0 -g3 -gdwarf-4
BUILDDIR ?= build

# Source files
CFILES = $(wildcard *.c) $(wildcard drivers/*.c) $(wildcard fs/*.c)
ARCH_CFILES = $(wildcard arch/aarch64/*.c)
ASMFILES = $(wildcard arch/aarch64/*.S)

# Object files - note the change here for ASM objects
COBJS = $(addprefix $(BUILDDIR)/, $(CFILES:.c=.o))
ARCH_COBJS = $(addprefix $(BUILDDIR)/, $(notdir $(ARCH_CFILES:.c=.o)))
# Add a suffix to distinguish ASM objects from C objects with same base name
ASMOBJS = $(addprefix $(BUILDDIR)/, $(notdir $(ASMFILES:.S=_asm.o)))

all: $(COBJS) $(ARCH_COBJS) $(ASMOBJS)

# Create build subdirectories as needed
$(BUILDDIR)/fs $(BUILDDIR)/drivers $(BUILDDIR)/arch/aarch64:
	mkdir -p $@

# Rule for driver files - depends on the directory existing
$(BUILDDIR)/drivers/%.o: drivers/%.c | $(BUILDDIR)/drivers
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for fs files - depends on the directory existing
$(BUILDDIR)/fs/%.o: fs/%.c | $(BUILDDIR)/fs
	$(CC) $(CFLAGS) -c $< -o $@

# Kernel C files
$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Architecture-specific C files
$(BUILDDIR)/%.o: arch/aarch64/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assembly files - updated pattern to match the _asm suffix
$(BUILDDIR)/%_asm.o: arch/aarch64/%.S
	$(CC) $(CFLAGS) -D__ASSEMBLY__ -c $< -o $@

clean:
	# Objects cleaned by parent

.PHONY: all clean