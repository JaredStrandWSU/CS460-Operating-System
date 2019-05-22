/********************************************************************
Copyright 2010-2017 K.C. Wang, <kwang@eecs.wsu.edu>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

/** loader.c: load user mode image file in ELF format *********

1. need ELF header, Program header types
2. use ld script during linking
3. Example of ld script:
/* A simple ld script */

/* for flat binary ******/
/*
OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)
ENTRY(u_entry)
SECTIONS
{
  . = 0x0;
  .text : { *(.text) }
  .data : { *(.data) }
  .bss  : { *(.bss)  }
}
*/

/******** for ELF *******/
 /***********************************************************
OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(u_entry)
SECTIONS
{
  . = 0x0;
  .text : { *(.text) }
  . = 0x8000;
  .data : { *(.data) }
  . = 0xC000;
  .bss  : { *(.bss)  }
}

4. In the ELF file: text begins at VA=0, data at 0x8000, bss at 0xC000.
   The sections shall be loaded at

   0             0x8000   0xC000                          4MB
   --------------------------------------------------------
   |text (RO)    |data    |bss |                  ustack  |   
   --------------------------------------------------------

5. Loading algorithm:
   1. read ELF header 
   2. read program header to find out offset, filesize
   3. load filesize bytes from offset to page frames of proc
   4  read 4096 bytes to a buf;
      memcpy to a page frame 
*************************************************************/
/*****************************************************************
         Currently, all Umode image file sizes are < 4MB, 
 So, only load an image file to the pages of the first pdgir[0].
******************************************************************/
typedef unsigned int u32;
		 typedef unsigned short u16;
		 typedef unsigned char  u8;


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#define PGSIZE 4096
#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian
#define ELF_PROG_LOAD           1
// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4

// ELF file header
struct elfhdr {
  u32 magic;  // must equal ELF_MAGIC
  u32 dummy[3];
  u16 type;
  u16 machine;
  u32 version;
  u32 entry;
  u32 phoffset;
  u32 shoffset;
  u32 flags;
  u16 ehsize;
  u16 phentsize;
  u16 phnum;
  u16 shentsize;
  u16 shnum;
  u16 shstrndx;
};

// ELF program section header
struct proghdr {
  u32 type;
  u32 offset;
  u32 vaddr;
  u32 paddr;
  u32 filesize;
  u32 memsize;
  u32 flags;  // use this to set page's R/W
  u32 align;
};
// ELF Section header
struct shdr {
  u32 name;
  u32 type;
  u32 flags;
  u32 addr;
  u32 offset;
  u32 size;
  u32 link;
  u32 info; 
  u32 align;
  u32 entsize;
};

int main(int argc, char *argv[])
{ 
  int fd, i, n, phnum, pn, ELF;
   u32 size, count, total;
   char *addr;
   u32  *pgtable;
   struct proghdr *aph, *ph;
   struct shdr    *ash, *sh;

   struct elfhdr  *elf;
   char ebuf[1024], shbuf[1024];
   int filesize, memsize;

   if (argc<2)
     exit(1);
   printf("file=%s\n", argv[1]);

   fd = open(argv[1], O_RDWR); // open for READ again
   if (fd < 0){
     printf("open %s failed\n", argv[1]);
     exit(1);
   }

   n = read(fd, ebuf, 256); // read elf header
   elf = (struct elfhdr *)ebuf;
   printf("elf magic=%x phoffset=%x shoffset=%x\n", 
	  elf->magic, elf->phoffset, elf->shoffset);

   printf("shoffset=%x shnum=%x shentsize=%x\n", 
	  elf->shoffset, elf->shnum, elf->shentsize);

   aph = (struct proghdr *)((char *)elf + elf->phoffset);
   ash = (struct shdr *)   ((char *)elf + elf->shoffset);
 
   phnum = elf->phnum;
   printf("phnum=%d ", phnum);

   getchar(); 

      total = 0;
      for (i=1, ph=aph; i <= phnum; ph++, i++){
         if (ph->type != 1) 
             break;
         printf("offset=%d vaddr=%x\n", ph->offset, ph->vaddr);
         printf("Sec%d: ", i);
 
	 filesize = ph->filesize;
         memsize  = ph->memsize;     
	 printf("type=%x offset=%x vaddr=%x paddr=%x fsize=%x msize=%x\n", 
	 ph->type, ph->offset,ph->vaddr,ph->paddr, ph->filesize,ph->memsize);
                 
         getchar(); 
         printf("change filesize to memsize\n");
         ph->filesize = 0x300000;
         ph->memsize  = 0x300000;

         lseek(fd, (long)0, 0);
         write(fd, ebuf, 256);

      }

      lseek(fd, (long)(elf->shoffset), 0);
      read(fd, shbuf, 1024);
      sh = ash = (struct shdr *)shbuf;

      for (i=0; i<elf->shnum; i++){
	//	printf("sh=%x ", sh);
	//        getchar();
 printf("%d: name=%x type=%x addr=%x offset=%x size=%x link=%x, ents=%x\n", 
    i, sh->name, sh->type,sh->addr,sh->offset, sh->size,sh->link,sh->entsize);


	 if (i==3){
	   sh->type = 1;
	   sh->size = 0x300000;
printf("%d: name=%x type=%x addr=%x offset=%x size=%x link=%x, ents=%x\n", 
    i, sh->name, sh->type,sh->addr,sh->offset, sh->size,sh->link,sh->entsize);
	 }
	 getchar();
	 sh++;
      }
     lseek(fd, (long)(elf->shoffset), 0);
     write(fd, shbuf, 1024);

      /*

  u32 name;
  u32 type;
  u32 addr;
  u32 offset;
  u32 size;
  u32 link;
  u32 info; 
  u32 align;
  u32 entsize;
      */         

   close(fd);
}

