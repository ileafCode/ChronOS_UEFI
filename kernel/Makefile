
OSNAME = CustomOS

GNUEFI = ../gnu-efi
OVMFDIR = ../OVMFbin
LDS = kernel.ld
CC = x86_64-elf-gcc
ASMC = nasm
LD = x86_64-elf-ld

CFLAGS = -ffreestanding -I include -O0 -mno-sse -mno-sse2 -mno-mmx -mno-red-zone
LDFLAGS = -T $(LDS) -static -Bsymbolic -nostdlib -z max-page-size=0x1000

SRCDIR := src
OBJDIR := lib
BUILDDIR = bin
BOOTEFI := $(GNUEFI)/x86_64/bootloader/main.efi

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRC = $(call rwildcard,$(SRCDIR),*.c)
ASMSRC = $(call rwildcard,$(SRCDIR),*.asm)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
OBJS += $(patsubst $(SRCDIR)/%.asm, $(OBJDIR)/%_asm.o, $(ASMSRC))
DIRS = $(wildcard $(SRCDIR)/*)

kernel: $(OBJS) link

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@ echo !==== COMPILING $^
	@ mkdir -p $(@D)
	$(CC) $(CFLAGS) -g -c $^ -o $@

$(OBJDIR)/%_asm.o: $(SRCDIR)/%.asm
	@ echo !==== COMPILING $^
	@ mkdir -p $(@D)
	$(ASMC) $(ASMFLAGS) -g $^ -f elf64 -o $@
	
link:
	@ echo !==== LINKING
	$(LD) $(LDFLAGS) -o $(BUILDDIR)/kernel.elf $(OBJS)

setup:
	@mkdir $(BUILDDIR)
	@mkdir $(SRCDIR)
	@mkdir $(OBJDIR)

buildimg:
	dd if=/dev/zero of=$(BUILDDIR)/$(OSNAME).img bs=512 count=65536
	mformat -i $(BUILDDIR)/$(OSNAME).img -h 64 -t 64 -n 32 -c 1 ::
	mmd -i $(BUILDDIR)/$(OSNAME).img ::/EFI
	mmd -i $(BUILDDIR)/$(OSNAME).img ::/EFI/BOOT
	mcopy -i $(BUILDDIR)/$(OSNAME).img $(BOOTEFI) ::/EFI/BOOT
	mcopy -i $(BUILDDIR)/$(OSNAME).img startup.nsh ::
	mcopy -i $(BUILDDIR)/$(OSNAME).img $(BUILDDIR)/kernel.elf ::
	mcopy -i $(BUILDDIR)/$(OSNAME).img $(BUILDDIR)/font.psf ::
	mcopy -i $(BUILDDIR)/$(OSNAME).img $(BUILDDIR)/functions.txt ::
	mcopy -i $(BUILDDIR)/$(OSNAME).img ../user_programs/bin/test ::
	mcopy -i $(BUILDDIR)/$(OSNAME).img ../user_programs/bin/init ::

run:
	sudo qemu-system-x86_64 -drive file=$(BUILDDIR)/$(OSNAME).img \
	-audiodev dbus,id=coreaudiodev \
	-netdev vmnet-bridged,id=net0,ifname=en0 \
	-device edu \
	-device e1000,netdev=net0 \
	-device AC97,audiodev=coreaudiodev \
	-m 512M -cpu qemu64 -machine q35 \
	-drive if=pflash,format=raw,unit=0,file="$(OVMFDIR)/ovmf_code.fd",readonly=on \
	-drive if=pflash,format=raw,unit=1,file="$(OVMFDIR)/ovmf_vars.fd",readonly=on \
	-debugcon stdio
