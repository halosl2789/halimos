#!/bin/bash

nasm -f bin bootsector.asm -o bootsector.bin

nasm -f elf32 boot.asm -o boot.o

gcc -m32 -ffreestanding -fno-stack-protector -fno-pic -fno-PIE -c kernel.c -o kernel.o


ld -m elf_i386 -T linker.ld -o kernel.bin boot.o kernel.o

cat bootsector.bin kernel.bin > halimos.img

if [ -f kernel.bin ]; then
    echo "SUCCESS: HalimOS build completed "
else
    echo "ERROR: Kernel.bin was not created. Check linker errors "
fi