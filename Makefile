K=kernel
U=user

# Common kernel sources (exclude architecture-specific directories and files with arch-specific versions)
K_SRC_COMMON := $(filter-out $(wildcard $K/*/rv/*.c) $(wildcard $K/*/la/*.c) $(wildcard $K/*/*/rv/*.c) $(wildcard $K/*/*/la/*.c) $K/trap/trap.c, $(wildcard $K/*/*.c) $(wildcard $K/*/*/*.c) $(wildcard $K/*/*/*/*.c))
K_ASM_COMMON := $(filter-out $(wildcard $K/*/rv/*.S) $(wildcard $K/*/la/*.S) $(wildcard $K/*/*/rv/*.S) $(wildcard $K/*/*/la/*.S), $(wildcard $K/*/*.S) $(wildcard $K/*/*/*.S) $(wildcard $K/*/*/*/*.S))

# RISC-V specific sources
K_SRC_RV := $(wildcard $K/*/rv/*.c) $(wildcard $K/*/*/rv/*.c)
K_ASM_RV := $(wildcard $K/*/rv/*.S) $(wildcard $K/*/*/rv/*.S)

# LoongArch specific sources  
K_SRC_LA := $(wildcard $K/*/la/*.c) $(wildcard $K/*/*/la/*.c)
K_ASM_LA := $(wildcard $K/*/la/*.S) $(wildcard $K/*/*/la/*.S)

# RISC-V specific objects
OBJS_RV = \
  boot/rv/entry.o \
  boot/start-rv.o \
  boot/main-rv.o \
  boot/rv/initcode.o \
  $(K_SRC_COMMON:.c=-rv.o) \
  $(K_ASM_COMMON:.S=-rv.o) \
  $(K_SRC_RV:.c=-rv.o) \
  $(K_ASM_RV:.S=-rv.o)

# LoongArch specific objects  
OBJS_LA = \
  boot/la/entry.o \
  boot/main-la.o \
  boot/la/initcode.o \
  $(K_SRC_COMMON:.c=-la.o) \
  $(K_ASM_COMMON:.S=-la.o) \
  $(K_SRC_LA:.c=-la.o) \
  $(K_ASM_LA:.S=-la.o)

# RISC-V specific objects for sh variant
OBJS_RV_SH = \
  boot/rv/entry.o \
  boot/start-rv.o \
  boot/main-rv.o \
  boot/rv/initcode-sh.o \
  $(K_SRC_COMMON:.c=-rv.o) \
  $(K_ASM_COMMON:.S=-rv.o) \
  $(K_SRC_RV:.c=-rv.o) \
  $(K_ASM_RV:.S=-rv.o)

# LoongArch specific objects for sh variant  
OBJS_LA_SH = \
  boot/la/entry.o \
  boot/main-la.o \
  boot/la/initcode-sh.o \
  $(K_SRC_COMMON:.c=-la.o) \
  $(K_ASM_COMMON:.S=-la.o) \
  $(K_SRC_LA:.c=-la.o) \
  $(K_ASM_LA:.S=-la.o)

# RISC-V toolchain
TOOLPREFIX_RV = riscv64-linux-gnu-
QEMU_RV = qemu-system-riscv64

CC_RV = $(TOOLPREFIX_RV)gcc
AS_RV = $(TOOLPREFIX_RV)gas
LD_RV = $(TOOLPREFIX_RV)ld
OBJCOPY_RV = $(TOOLPREFIX_RV)objcopy
OBJDUMP_RV = $(TOOLPREFIX_RV)objdump

# LoongArch toolchain
TOOLPREFIX_LA = loongarch64-linux-gnu-
QEMU_LA = qemu-system-loongarch64

CC_LA = $(TOOLPREFIX_LA)gcc
AS_LA = $(TOOLPREFIX_LA)gas
LD_LA = $(TOOLPREFIX_LA)ld
OBJCOPY_LA = $(TOOLPREFIX_LA)objcopy
OBJDUMP_LA = $(TOOLPREFIX_LA)objdump

# Common CFLAGS
CFLAGS_COMMON = -Wall -Werror -O -fno-omit-frame-pointer -ggdb -gdwarf-2
CFLAGS_COMMON += -MD
CFLAGS_COMMON += -ffreestanding -fno-common -nostdlib
CFLAGS_COMMON += -fno-builtin-strncpy -fno-builtin-strncmp -fno-builtin-strlen -fno-builtin-memset
CFLAGS_COMMON += -fno-builtin-memmove -fno-builtin-memcmp -fno-builtin-log -fno-builtin-bzero
CFLAGS_COMMON += -fno-builtin-strchr -fno-builtin-exit -fno-builtin-malloc -fno-builtin-putc
CFLAGS_COMMON += -fno-builtin-free
CFLAGS_COMMON += -fno-builtin-memcpy -Wno-main -Wno-unused-variable
CFLAGS_COMMON += -fno-builtin-printf -fno-builtin-fprintf -fno-builtin-vprintf
CFLAGS_COMMON += -I./include -I./include/fs/ext4 
CFLAGS_COMMON += -D QEMU

# RISC-V specific CFLAGS
CFLAGS_RV = $(CFLAGS_COMMON)
CFLAGS_RV += -mcmodel=medany -mno-relax
CFLAGS_RV += -D RISCV

# LoongArch specific CFLAGS  
CFLAGS_LA = $(CFLAGS_COMMON)
CFLAGS_LA += -D LOONGARCH
CFLAGS_LA += -Wno-unused-function

# Add stack protector flags if available
CFLAGS_RV += $(shell $(CC_RV) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
CFLAGS_LA += $(shell $(CC_LA) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

# Disable PIE when possible (for Ubuntu 16.10 toolchain)
ifneq ($(shell $(CC_RV) -dumpspecs 2>/dev/null | grep -e '[^f]no-pie'),)
CFLAGS_RV += -fno-pie -no-pie
endif
ifneq ($(shell $(CC_RV) -dumpspecs 2>/dev/null | grep -e '[^f]nopie'),)
CFLAGS_RV += -fno-pie -nopie
endif

ifneq ($(shell $(CC_LA) -dumpspecs 2>/dev/null | grep -e '[^f]no-pie'),)
CFLAGS_LA += -fno-pie -no-pie
endif
ifneq ($(shell $(CC_LA) -dumpspecs 2>/dev/null | grep -e '[^f]nopie'),)
CFLAGS_LA += -fno-pie -nopie
endif

# Additional flags for toolchains
CFLAGS_RV += -static -nostartfiles -fno-pic
CFLAGS_LA += -static -nostartfiles -fno-pic
CFLAGS_LA += -Wno-shift-overflow
LDFLAGS = -z max-page-size=4096
LDFLAGS_RV = $(LDFLAGS) -static -nostdlib
LDFLAGS_LA = $(LDFLAGS) -static -nostdlib

ULIB_C_SOURCES = $U/ulib.c $U/printf.c $U/umalloc.c
ULIB_S_SOURCES_RV = $U/usys-rv.S
ULIB_S_SOURCES_LA = $U/usys-la.S
ULIB_O_FILES_RV = $(patsubst %.c,%-rv.o,$(ULIB_C_SOURCES)) $(patsubst %.S,%.o,$(ULIB_S_SOURCES_RV))
ULIB_O_FILES_LA = $(patsubst %.c,%-la.o,$(ULIB_C_SOURCES)) $(patsubst %.S,%.o,$(ULIB_S_SOURCES_LA))

# RISC-V kernel build
kernel-rv: $U/initcode-rv $(OBJS_RV) $K/rv/kernel.ld
	$(LD_RV) $(LDFLAGS_RV) -T $K/rv/kernel.ld -o kernel-rv $(OBJS_RV) 
	$(OBJDUMP_RV) -S kernel-rv > kernel-rv.asm
	$(OBJDUMP_RV) -t kernel-rv | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel-rv.sym

# LoongArch kernel build
kernel-la: $U/initcode-la $(OBJS_LA) $K/la/kernel.ld
	$(LD_LA) $(LDFLAGS_LA) -T $K/la/kernel.ld -o kernel-la $(OBJS_LA) 
	$(OBJDUMP_LA) -S kernel-la > kernel-la.asm
	$(OBJDUMP_LA) -t kernel-la | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel-la.sym

# RISC-V kernel build for sh variant
kernel-rv-sh: $U/initcode-rv-sh $(OBJS_RV_SH) $K/rv/kernel.ld
	$(LD_RV) $(LDFLAGS_RV) -T $K/rv/kernel.ld -o kernel-rv-sh $(OBJS_RV_SH) 
	$(OBJDUMP_RV) -S kernel-rv-sh > kernel-rv-sh.asm
	$(OBJDUMP_RV) -t kernel-rv-sh | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel-rv-sh.sym

# LoongArch kernel build for sh variant
kernel-la-sh: $U/initcode-la-sh $(OBJS_LA_SH) $K/la/kernel.ld
	$(LD_LA) $(LDFLAGS_LA) -T $K/la/kernel.ld -o kernel-la-sh $(OBJS_LA_SH) 
	$(OBJDUMP_LA) -S kernel-la-sh > kernel-la-sh.asm
	$(OBJDUMP_LA) -t kernel-la-sh | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel-la-sh.sym

# RISC-V initcode build
$U/initcode-rv: $U/initcode.S $U/init-rv.c $(ULIB_C_SOURCES) $(ULIB_S_SOURCES_RV)
	@echo "--- Building RISC-V initcode and its dependencies ---"
	# Step 1: Compile all necessary C source files
	$(CC_RV) $(CFLAGS_RV) -c -o $U/initcode-rv.o $U/initcode.S
	$(CC_RV) $(CFLAGS_RV) -c -o $U/init-rv.o $U/init-rv.c
	$(CC_RV) $(CFLAGS_RV) -c -o $U/ulib-rv.o $U/ulib.c
	$(CC_RV) $(CFLAGS_RV) -c -o $U/printf-rv.o $U/printf.c
	$(CC_RV) $(CFLAGS_RV) -c -o $U/umalloc-rv.o $U/umalloc.c
	# Step 2: Generate and compile usys.S
	perl $U/usys-rv.pl > $U/usys-rv.S
	$(CC_RV) $(CFLAGS_RV) -c -o $U/usys-rv.o $U/usys-rv.S
	# Step 3: Link everything together
	@echo "--- Linking RISC-V initcode ---"
	$(LD_RV) $(LDFLAGS_RV) -N -e start -Ttext 0 -o $U/initcode-rv.out $U/initcode-rv.o $U/init-rv.o $(ULIB_O_FILES_RV)
	$(OBJCOPY_RV) -S -O binary $U/initcode-rv.out $@
	$(OBJDUMP_RV) -S $U/initcode-rv.out > $U/initcode-rv.asm
	rm -f $U/initcode-rv.out

# LoongArch initcode build
$U/initcode-la: $U/initcode-la.S $U/init-la.c $(ULIB_C_SOURCES) $(ULIB_S_SOURCES_LA)
	@echo "--- Building LoongArch initcode and its dependencies ---"
	# Step 1: Compile all necessary C source files
	$(CC_LA) $(CFLAGS_LA) -c -o $U/initcode-la.o $U/initcode-la.S
	$(CC_LA) $(CFLAGS_LA) -c -o $U/init-la.o $U/init-la.c
	$(CC_LA) $(CFLAGS_LA) -c -o $U/ulib-la.o $U/ulib.c
	$(CC_LA) $(CFLAGS_LA) -c -o $U/printf-la.o $U/printf.c
	$(CC_LA) $(CFLAGS_LA) -c -o $U/umalloc-la.o $U/umalloc.c
	# Step 2: Generate and compile usys.S
	perl $U/usys-la.pl > $U/usys-la.S
	$(CC_LA) $(CFLAGS_LA) -c -o $U/usys-la.o $U/usys-la.S
	# Step 3: Link everything together
	@echo "--- Linking LoongArch initcode ---"
	$(LD_LA) $(LDFLAGS_LA) -N -e start -Ttext 0 -o $U/initcode-la.out $U/initcode-la.o $U/init-la.o $(ULIB_O_FILES_LA)
	$(OBJCOPY_LA) -S -O binary $U/initcode-la.out $@
	$(OBJDUMP_LA) -S $U/initcode-la.out > $U/initcode-la.asm
	rm -f $U/initcode-la.out

# RISC-V initcode build for sh variant (using init-sh.c)
$U/initcode-rv-sh: $U/initcode.S $U/init-sh.c $(ULIB_C_SOURCES) $(ULIB_S_SOURCES_RV)
	@echo "--- Building RISC-V initcode-sh and its dependencies ---"
	# Step 1: Compile all necessary C source files
	$(CC_RV) $(CFLAGS_RV) -c -o $U/initcode-rv-sh.o $U/initcode.S
	$(CC_RV) $(CFLAGS_RV) -c -o $U/init-sh-rv.o $U/init-sh.c
	$(CC_RV) $(CFLAGS_RV) -c -o $U/ulib-rv.o $U/ulib.c
	$(CC_RV) $(CFLAGS_RV) -c -o $U/printf-rv.o $U/printf.c
	$(CC_RV) $(CFLAGS_RV) -c -o $U/umalloc-rv.o $U/umalloc.c
	# Step 2: Generate and compile usys.S (re-using existing usys-rv.S/o)
	# Note: usys-rv.S is already handled by $U/usys-rv.S target
	$(CC_RV) $(CFLAGS_RV) -c -o $U/usys-rv.o $U/usys-rv.S
	# Step 3: Link everything together
	@echo "--- Linking RISC-V initcode-sh ---"
	$(LD_RV) $(LDFLAGS_RV) -N -e start -Ttext 0 -o $U/initcode-rv-sh.out $U/initcode-rv-sh.o $U/init-sh-rv.o $(ULIB_O_FILES_RV)
	$(OBJCOPY_RV) -S -O binary $U/initcode-rv-sh.out $@
	$(OBJDUMP_RV) -S $U/initcode-rv-sh.out > $U/initcode-rv-sh.asm
	rm -f $U/initcode-rv-sh.out

# LoongArch initcode build for sh variant (using init-sh.c)
$U/initcode-la-sh: $U/initcode-la.S $U/init-sh.c $(ULIB_C_SOURCES) $(ULIB_S_SOURCES_LA)
	@echo "--- Building LoongArch initcode-sh and its dependencies ---"
	# Step 1: Compile all necessary C source files
	$(CC_LA) $(CFLAGS_LA) -c -o $U/initcode-la-sh.o $U/initcode-la.S
	$(CC_LA) $(CFLAGS_LA) -c -o $U/init-sh-la.o $U/init-sh.c
	$(CC_LA) $(CFLAGS_LA) -c -o $U/ulib-la.o $U/ulib.c
	$(CC_LA) $(CFLAGS_LA) -c -o $U/printf-la.o $U/printf.c
	$(CC_LA) $(CFLAGS_LA) -c -o $U/umalloc-la.o $U/umalloc.c
	# Step 2: Generate and compile usys.S (re-using existing usys-la.S/o)
	# Note: usys-la.S is already handled by $U/usys-la.S target
	$(CC_LA) $(CFLAGS_LA) -c -o $U/usys-la.o $U/usys-la.S
	# Step 3: Link everything together
	@echo "--- Linking LoongArch initcode-sh ---"
	$(LD_LA) $(LDFLAGS_LA) -N -e start -Ttext 0 -o $U/initcode-la-sh.out $U/initcode-la-sh.o $U/init-sh-la.o $(ULIB_O_FILES_LA)
	$(OBJCOPY_LA) -S -O binary $U/initcode-la-sh.out $@
	$(OBJDUMP_LA) -S $U/initcode-la-sh.out > $U/initcode-la-sh.asm
	rm -f $U/initcode-la-sh.out

tags: $(OBJS_RV) $(OBJS_LA) _init
	etags *.S *.c

ULIB_RV = $U/ulib-rv.o $U/usys-rv.o $U/printf-rv.o $U/umalloc-rv.o
ULIB_LA = $U/ulib-la.o $U/usys-la.o $U/printf-la.o $U/umalloc-la.o

_%-rv: %-rv.o $(ULIB_RV)
	$(LD_RV) $(LDFLAGS_RV) -T $U/user.ld -o $@ $^
	$(OBJDUMP_RV) -S $@ > $*-rv.asm
	$(OBJDUMP_RV) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $*-rv.sym

_%-la: %-la.o $(ULIB_LA)
	$(LD_LA) $(LDFLAGS_LA) -T $U/user-la.ld -o $@ $^
	$(OBJDUMP_LA) -S $@ > $*-la.asm
	$(OBJDUMP_LA) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $*-la.sym

$U/usys-rv.S : $U/usys-rv.pl
	perl $U/usys-rv.pl > $U/usys-rv.S

$U/usys-rv.o : $U/usys-rv.S
	$(CC_RV) $(CFLAGS_RV) -c -o $U/usys-rv.o $U/usys-rv.S

$U/usys-la.S : $U/usys-la.pl
	perl $U/usys-la.pl > $U/usys-la.S

$U/usys-la.o : $U/usys-la.S
	$(CC_LA) $(CFLAGS_LA) -c -o $U/usys-la.o $U/usys-la.S

$U/_forktest-rv: $U/forktest-rv.o $(ULIB_RV)
	# forktest has less library code linked in - needs to be small
	# in order to be able to max out the proc table.
	$(LD_RV) $(LDFLAGS_RV) -N -e main -Ttext 0 -o $U/_forktest-rv $U/forktest-rv.o $(ULIB_RV)
	$(OBJDUMP_RV) -S $U/_forktest-rv > $U/forktest-rv.asm

$U/_forktest-la: $U/forktest-la.o $(ULIB_LA)
	# forktest has less library code linked in - needs to be small
	# in order to be able to max out the proc table.
	$(LD_LA) $(LDFLAGS_LA) -N -e main -Ttext 0 -o $U/_forktest-la $U/forktest-la.o $(ULIB_LA)
	$(OBJDUMP_LA) -S $U/_forktest-la > $U/forktest-la.asm

# Prevent deletion of intermediate files, e.g. cat.o, after first build, so
# that disk image changes after first build are persistent until clean.  More
# details:
# http://www.gnu.org/software/make/manual/html_node/Chained-Rules.html


# Embedded glibc dynamic linker (for PT_INTERP support)


.PRECIOUS: %-rv.o %-la.o

# List of user programs to be included in the filesystem image
UPROGS_RV=\
	$U/_cat-rv\
	$U/_echo-rv\
	$U/_forktest-rv\
	$U/_grep-rv\
	$U/_kill-rv\
	$U/_mkdir-rv\
	$U/_rm-rv\
	$U/_sh-rv\
	$U/_wc-rv\
	$U/_ls-rv\
	$U/_zombie-rv\
	$U/_sigtest-rv\
	$U/_pptest-rv\
	$U/_simple-rv\
	$U/_futex-rv\

UPROGS_LA=\
	$U/_cat-la\
	$U/_echo-la\
	$U/_forktest-la\
	$U/_grep-la\
	$U/_kill-la\
	$U/_mkdir-la\
	$U/_rm-la\
	$U/_sh-la\
	$U/_wc-la\
	$U/_ls-la\
	$U/_zombie-la\
	$U/_sigtest-la\
	$U/_pptest-la\
	$U/_simple-la\
	$U/_futex-la\
#
# === MODIFIED SECTION: Building the ext4 Filesystem Image ===
#
# This section creates an ext4-formatted disk image `fs.img`
# and copies the user programs into it.
# This requires privileges and standard Linux tools (dd, mkfs.ext4, mount).
#

#
# === MODIFIED SECTION: Building the ext4 Filesystem Image (v2) ===
#
# This version incorporates best-practice parameters from your other Makefile
# to ensure compatibility with the lwext4 library in the kernel.
#

# Define a temporary mount point
MNT_DIR := fs_mnt
# Define image size in Megabytes (feel free to adjust)
IMG_SIZE_MB := 256

fs-rv.img: $(UPROGS_RV) README
	@echo "--- Creating filesystem image (fs.img) ---"
	# Clean up any previous mounts or directories to be safe
	@-sudo umount $(MNT_DIR) 2>/dev/null || true
	@rm -rf $(MNT_DIR)
	# 1. Create a zero-filled image file
	@echo "Creating a $(IMG_SIZE_MB)MB disk image..."
	@dd if=/dev/zero of=fs.img bs=1M count=$(IMG_SIZE_MB)
	# 2. Format the image with an ext4 filesystem using compatible options
	@echo "Formatting with ext4 (disabling metadata_csum for lwext4 compatibility)..."
	@mkfs.ext4 -O ^metadata_csum -F -b 4096 -L rootfs fs.img
	# 3. Create a mount point and mount the image
	@echo "Mounting the image..."
	@mkdir -p $(MNT_DIR)
	@sudo mount fs.img $(MNT_DIR)
	# 4. Copy README and all user programs into the image's root
	@echo "Copying user programs..."
	@sudo cp README $(MNT_DIR)/
	@for prog in $(UPROGS_RV); do \
		sudo cp $$prog $(MNT_DIR)/`basename $$prog | sed 's/^_//' | sed 's/-rv$$//'`; \
	done
	# 5. Unmount the image
	@echo "Unmounting the image..."
	@sudo umount $(MNT_DIR)
	# 6. Clean up the temporary mount point directory
	@rm -rf $(MNT_DIR)
	@echo "--- Filesystem image created successfully ---"


fs-la.img: $(UPROGS_LA) README
	@echo "--- Creating filesystem image (fs.img) ---"
	# Clean up any previous mounts or directories to be safe
	@-sudo umount $(MNT_DIR) 2>/dev/null || true
	@rm -rf $(MNT_DIR)
	# 1. Create a zero-filled image file
	@echo "Creating a $(IMG_SIZE_MB)MB disk image..."
	@dd if=/dev/zero of=fs.img bs=1M count=$(IMG_SIZE_MB)
	# 2. Format the image with an ext4 filesystem using compatible options
	@echo "Formatting with ext4 (disabling metadata_csum for lwext4 compatibility)..."
	@mkfs.ext4 -O ^metadata_csum -F -b 4096 -L rootfs fs.img
	# 3. Create a mount point and mount the image
	@echo "Mounting the image..."
	@mkdir -p $(MNT_DIR)
	@sudo mount fs.img $(MNT_DIR)
	# 4. Copy README and all user programs into the image's root
	@echo "Copying user programs..."
	@sudo cp README $(MNT_DIR)/
	@for prog in $(UPROGS_LA); do \
		sudo cp $$prog $(MNT_DIR)/`basename $$prog | sed 's/^_//' | sed 's/-la$$//'`; \
	done
	# 5. Unmount the image
	@echo "Unmounting the image..."
	@sudo umount $(MNT_DIR)
	# 6. Clean up the temporary mount point directory
	@rm -rf $(MNT_DIR)
	@echo "--- Filesystem image created successfully ---"


-include kernel/*.d user/*.d

# RISC-V object file compilation rules
%-rv.o: %.c
	$(CC_RV) $(CFLAGS_RV) -c -o $@ $<

%-rv.o: %.S
	$(CC_RV) $(CFLAGS_RV) -c -o $@ $<

boot/main-rv.o: boot/main.c
	$(CC_RV) $(CFLAGS_RV) -c -o $@ $<

boot/start-rv.o: boot/rv/start.c
	$(CC_RV) $(CFLAGS_RV) -c -o $@ $<

boot/rv/entry.o: boot/rv/entry.S
	$(CC_RV) $(CFLAGS_RV) -c -o $@ $<

# LoongArch object file compilation rules  
%-la.o: %.c
	$(CC_LA) $(CFLAGS_LA) -c -o $@ $<

%-la.o: %.S
	$(CC_LA) $(CFLAGS_LA) -c -o $@ $<

boot/main-la.o: boot/main.c
	$(CC_LA) $(CFLAGS_LA) -c -o $@ $<

boot/la/entry.o: boot/la/entry.S
	$(CC_LA) $(CFLAGS_LA) -c -o $@ $<

boot/rv/initcode.o: boot/rv/initcode.S $U/initcode-rv
	$(CC_RV) $(CFLAGS_RV) -c -o $@ $<

boot/la/initcode.o: boot/la/initcode.S $U/initcode-la
	$(CC_LA) $(CFLAGS_LA) -c -o $@ $<

# New initcode object files for sh variant
boot/rv/initcode-sh.o: boot/rv/initcode-sh.S $U/initcode-rv-sh
	$(CC_RV) $(CFLAGS_RV) -c -o $@ $< 

boot/la/initcode-sh.o: boot/la/initcode-sh.S $U/initcode-la-sh
	$(CC_LA) $(CFLAGS_LA) -c -o $@ $<

all: kernel-rv kernel-la

clean: 
	rm -f *.tex *.dvi *.idx *.aux *.log *.ind *.ilg \
	*/*.o */*/*.o */*/*/*.o */*.d */*/*.d */*/*/*.d */*.asm */*.sym \
	*/*-rv.o */*-la.o */*/*-rv.o */*/*-la.o \
	*/*/*/*-rv.o */*/*/*-la.o \
	$U/initcode-rv $U/initcode-la $U/initcode-rv.out $U/initcode-la.out \
	$U/initcode-rv-sh $U/initcode-la-sh $U/initcode-rv-sh.out $U/initcode-la-sh.out \
	 kernel-rv kernel-la \
	kernel-rv.asm kernel-la.asm \
	kernel-rv.sym kernel-la.sym \
	kernel-rv-sh kernel-la-sh kernel-rv-sh.asm kernel-la-sh.asm \
	kernel-rv-sh.sym kernel-la-sh.sym \
	 fs.img mkfs/mkfs .gdbinit \
        $U/usys-rv.S $U/usys-la.S \
	 include/ramdisk_img.h \
	$(UPROGS_RV) $(UPROGS_LA)

# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 26000)
# QEMU's gdb stub command line changed in 0.11
QEMUGDB_RV = $(shell if $(QEMU_RV) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)
QEMUGDB_LA = $(shell if $(QEMU_LA) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)

ifndef CPUS
CPUS := 1
endif

# RISC-V QEMU options
QEMUOPTS_RV = -machine virt -bios default -kernel kernel-rv -m 1G -smp $(CPUS) -nographic
QEMUOPTS_RV += -drive file=sdcard-rv.img,if=none,format=raw,id=x0
QEMUOPTS_RV += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
QEMUOPTS_RV += -rtc base=utc
QEMUOPTS_RV += -no-reboot

# LoongArch QEMU options
QEMUOPTS_LA = -kernel kernel-la -m 1G -nographic -smp 1
QEMUOPTS_LA += -drive file=sdcard-la.img,if=none,format=raw,id=x0
QEMUOPTS_LA += -device virtio-blk-pci,drive=x0
QEMUOPTS_LA += -no-reboot -device virtio-net-pci,netdev=net0 -netdev user,id=net0
QEMUOPTS_LA += -rtc base=utc

# RISC-V QEMU options for sh variant (using fs.img)
QEMUOPTS_SH_RV = -machine virt -bios default -kernel kernel-rv-sh -m 1G -smp $(CPUS) -nographic
QEMUOPTS_SH_RV += -drive file=fs.img,if=none,format=raw,id=x0
QEMUOPTS_SH_RV += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
QEMUOPTS_SH_RV += -rtc base=utc

# LoongArch QEMU options for sh variant (using fs.img)
QEMUOPTS_SH_LA = -kernel kernel-la-sh -m 1G -nographic -smp 1
QEMUOPTS_SH_LA += -drive file=fs.img,if=none,format=raw,id=x0
QEMUOPTS_SH_LA += -device virtio-blk-pci,drive=x0
QEMUOPTS_SH_LA += -no-reboot -device virtio-net-pci,netdev=net0 -netdev user,id=net0
QEMUOPTS_SH_LA += -rtc base=utc

qemu: kernel-rv sdcard-rv.img
	$(QEMU_RV) $(QEMUOPTS_RV)

qemu-la: kernel-la sdcard-la.img
	$(QEMU_LA) $(QEMUOPTS_LA)

# New qemu targets for sh variant
qemu-sh: kernel-rv-sh fs-rv.img
	$(QEMU_RV) $(QEMUOPTS_SH_RV)

qemu-la-sh: kernel-la-sh fs-la.img
	$(QEMU_LA) $(QEMUOPTS_SH_LA)

.gdbinit: .gdbinit.tmpl-riscv
	sed "s/:1234/:$(GDBPORT)/" < $^ > $@

qemu-gdb: kernel-rv .gdbinit
	@echo "*** Now run 'gdb' in another window." 1>&2
	$(QEMU_RV) $(QEMUOPTS_RV) -S $(QEMUGDB_RV)

qemu-gdb-la: kernel-la .gdbinit
	@echo "*** Now run 'gdb' in another window." 1>&2
	$(QEMU_LA) $(QEMUOPTS_LA) -S $(QEMUGDB_LA)

# === 测评相关目标 ===

# 清理测评产生的镜像文件
clean-test:
	rm -f sdcard-*.img

clean-testlog:
	rm -f os_serial_out_*.txt

# 一键测评：先清理残留 img，再构建并运行测评
# 使用方式: make test
# 如需指定架构: make test-rv  或  make test-la
test: clean-test kernel-rv sdcard-rv.img
	docker run --rm \
		-v $(PWD):/coursegrader/submit \
		-v /home/zhangshuoyu/oscomp-testdata:/coursegrader/testdata \
		-v /home/zhangshuoyu/autotest-for-oskernel:/cg \
		-v /home/zhangshuoyu/oscomp-testdata:/mnt/cghook/ \
		zhouzhouyi/os-contest:20260510 python3 /cg/kernel.zip

test-la: clean-test kernel-la sdcard-la.img
	docker run --rm \
		-v $(PWD):/coursegrader/submit \
		-v /home/zhangshuoyu/oscomp-testdata:/coursegrader/testdata \
		-v /home/zhangshuoyu/autotest-for-oskernel:/cg \
		-v /home/zhangshuoyu/oscomp-testdata:/mnt/cghook/ \
		zhouzhouyi/os-contest:20260510 python3 /cg/kernel.zip

.PHONY: all qemu qemu-la qemu-gdb qemu-gdb-la clean tags qemu-sh qemu-la-sh clean-test test test-la
