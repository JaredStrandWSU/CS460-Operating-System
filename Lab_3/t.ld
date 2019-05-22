/****************
qemu with -kernel file.bin ==> start to execute from 0x10000 ==>
file.bin must be loaded to 0x10000 ==> . = 0x10000
************/
ENTRY(reset)
SECTIONS
{
  . = 0x10000;
  .text : { ts.o  }
  .data : { *(.data) }
  .bss : { *(.bss) }
  . = ALIGN(8);
  . = . + 0x4000; /* 4kB of SVC stack space */
  svc_stack = .;
  . = . + 0x4000; /* 4kB of IRQ stack space */
  irq_stack = .;
}

