#!/bin/bash
LIBPATH=/usr/lib/gcc/arm-none-eabi/6.3.1/ #find out your lib version number
# 1. What does the following statement do?___________________________________
arm-none-eabi-as -mcpu=arm926ej-s -g ts.s -o ts.o
echo Assembled!
read dummy

# 2. What does the following statement do?___________________________________
arm-none-eabi-gcc -c -mcpu=arm926ej-s -g t.c -o t.o
echo Compiled!
read dummy

# 3. What does the following statement do?___________________________________
arm-none-eabi-ld -T t.ld ts.o t.o -o t.elf
#arm-none-eabi-ld -T t.ld -L $LIBPATH ts.o t.elf -o -lgcc
echo Linked!
read dummy

# 4. What does the following statement do?___________________________________
arm-none-eabi-objcopy -O binary t.elf t.bin
echo Converted to .bin from .elf!
read dummy
rm *.o *.elf

echo ready to go?
read dummy

#qemu-system-arm -M versatilepb -m 128M -kernel t.bin \
#-serial mon:stdio

qemu-system-arm -M realview-pbx-a9 -m 128M -kernel t.bin \
-serial /dev/pts/0 \
-serial /dev/pts/1 \
-serial /dev/pts/2 \
-serial /dev/pts/3

#-serial mon:stdio
#-serial mon:stdio -serial /dev/pts/0 -serial /dev/pts/1 -serial /dev/pts/2 
