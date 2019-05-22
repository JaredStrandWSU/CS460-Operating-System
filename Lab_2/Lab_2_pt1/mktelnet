arm-none-eabi-as -mcpu=arm926ej-s -g ts.s -o ts.o
arm-none-eabi-gcc -c -mcpu=arm926ej-s -g t.c -o t.o
arm-none-eabi-ld -T t.ld ts.o t.o -o t.elf
arm-none-eabi-objcopy -O binary t.elf t.bin
echo ready to go?

read dummy

# with a uart driver to support ugetc()/uputc(), connect by telent:
# Open another pseudo terminal, enter
# telnet localhost 1234
echo ------------------------------------------------------ 
echo Open another terminal and enter: telnet localhost 1234
echo then enter inputs from the telnet terminal
echo ------------------------------------------------------
qemu-system-arm -M versatilepb -m 128M -kernel t.bin -nographic \
-serial telnet:localhost:1234,server