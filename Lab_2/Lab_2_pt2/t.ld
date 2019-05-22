/****************
qemu with -kernel file.bin ==> start to execute from 0x10000 ==>
file.bin must be loaded to 0x10000 ==> . = 0x10000
************/
ENTRY(start)
SECTIONS
{
  . = 0x10000;
  .text : { ts.o *(.text) }
  .data : { *(.data) }
  .bss : { *(.bss) }
  
  .data : { wsu.o }     /* wsu.bmp image file */

  . = ALIGN(8);
  . = . + 0x4000;       /* SVC stack */
  stack_top = .;

}

