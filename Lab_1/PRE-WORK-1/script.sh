#!/bin/bash
#Use BCC to generate a one-segment binary executable a.out WITHOUT header
as86 -o bs.o bs.s
bcc -c -ansi t.c
ld86 -d bs.o t.o /usr/lib/bcc/libc.a
ls -l a.out
#Dump a.out to a VIRTUAL FD disk
dd if=a.out of=mtximage bs=1024 count=1 conv=notrunc
rm *.o
#Boot up QEMU from virtual FD disk
qemu-system-i386 -fda mtximage -no-fd-bootchk
echo done
