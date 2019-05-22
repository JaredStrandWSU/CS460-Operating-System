/****************
qemu with -kernel file.bin ==> start to execute from 0x10000 ==>
file.bin must be loaded to 0x10000 ==> . = 0x10000
************/
ENTRY(u_entry)
SECTIONS
{
  . = 0x0;
  .text : {
     us.o
     *(.text) 
  }
  .data : { *(.data) }
  .bss  : { *(.bss)  }
}


