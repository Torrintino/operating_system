#!/usr/bin/make
.SUFFIXES:
SRC = bootloader.c kernel.c
TAR = bootloader.bin
PCK = lab-1.zip

CFLAGS = -m32 -c -g -Os -march=i686 -ffreestanding -Wall -Werror
LFLAGS = -m elf_i386 -static -Tlinker.ld -nostdlib --nmagic

all: $(TAR)

%.o: %.c
	$(CC) $(CFLAGS)  $^ -o $@

disk_load.o: disk_load.s
	gcc -m32 -c -march=i686 -Os disk_load.s

bootloader.elf: bootloader.o disk_load.o
	$(LD) $(LFLAGS) -o $@ $^

kernel.elf: kernel.o
	$(LD) -m elf_i386 -static -nostdlib --nmagic -o kernel.elf kernel.o

kernel.bin: kernel.elf
	objcopy -O binary kernel.elf kernel.bin

bootloader.bin: kernel.bin bootloader.elf
	objcopy -O binary bootloader.elf bootloader.bin
	cat bootloader.bin kernel.bin > os.bin
	dd if=/dev/zero bs=1 count=1024 >> os.bin
	mv os.bin bootloader.bin

run: bootloader.bin
	qemu-system-x86_64 -drive format=raw,file=bootloader.bin

pack:
	zip $(PCK) Makefile *.c *.h *.s

clean:
	$(RM) $(RMFILES) $(TAR) $(PCK)
