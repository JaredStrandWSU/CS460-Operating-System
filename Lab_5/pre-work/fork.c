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
// In kernel Mode

int body(), goUmode();

PROC *kfork(char *filename)
{
  int i; 
  int *ptable, pentry;
  char *addr;
  /*
  char *cp, *cq;

  char line[8];
  int usize1, usize;
  int *ustacktop, *usp;
  u32 BA, Btop, Busp;
  */
  PROC *p = dequeue(&freeList);
  if (p==0){
    kprintf("kfork failed\n");
    return (PROC *)0;
  }
  p->ppid = running->pid;
  p->parent = running;
  p->parent = running;
  p->status = READY;
  p->priority = 1;
  p->cpsr = (int *)0x10; //Permissions mode 0x10 or 0x01
  
 // build p's pgtable 
  p->pgdir = (int *)(0x600000 + (p->pid - 1)*0x4000);
  ptable = p->pgdir;
  // initialize pgtable
  for (i=0; i<4096; i++)
    ptable[i] = 0;
  pentry = 0x412;
  for (i=0; i<258; i++){
    ptable[i] = pentry;
    pentry += 0x100000;
  }
  // ptable entry flag=|AP0|doma|1|CB10|=110|0001|1|1110|=0xC3E or 0xC32       
  //ptable[2048] = 0x800000 + (p->pid - 1)*0x100000|0xC3E;
  ptable[2048] = 0x800000 + (p->pid - 1)*0x100000|0xC32;
  
  p->cpsr = (int *)0x10;    // previous mode was Umode
  
  // set kstack to resume to goUmode, then to VA=0 in Umode image
  for (i=1; i<29; i++)  // all 28 cells = 0
    p->kstack[SSIZE-i] = 0;

  p->kstack[SSIZE-15] = (int)goUmode;  // in dec reg=address ORDER !!!
  p->ksp = &(p->kstack[SSIZE-28]);

  // kstack must contain a resume frame FOLLOWed by a goUmode frame
  //  ksp  
  //  -|-----------------------------------------
  //  r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 fp ip pc|
  //  -------------------------------------------
  //  28 27 26 25 24 23 22 21 20 19 18  17 16 15
  //  
  //   usp      NOTE: r0 is NOT saved in svc_entry()
  // -|-----goUmode--------------------------------
  //  r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 ufp uip upc|
  //-------------------------------------------------
  //  14 13 12 11 10 9  8  7  6  5  4   3    2   1
  /********************

  // to go Umode, must set new PROC's Umode cpsr to Umode=10000
  // this was done in ts.s dring init the mode stacks ==> 
  // user mode's cspr was set to IF=00, mode=10000  

  ***********************/
  // must load filename to Umode image area at 7MB+(pid-1)*1MB
  addr = (char *)(0x700000 + (p->pid)*0x100000);

  load(filename, p);
  
  // must fix Umode ustack for it to goUmode: how did the PROC come to Kmode?
  // by swi # from VA=0 in Umode => at that time all CPU regs are 0
  // we are in Kmode, p's ustack is at its Uimage (8mb+(pid-1)*1mb) high end
  // from PROC's point of view, it's a VA at 1MB (from its VA=0)
  // but we in Kmode must access p's Ustack directly

  /***** this sets each proc's ustack differently, thinking each in 8MB+
  ustacktop = (int *)(0x800000+(p->pid)*0x100000 + 0x100000);
  TRY to set it to OFFSET 1MB in its section; regardless of pid
  **********************************************************************/
  //p->usp = (int *)(0x80100000);
  p->usp = (int *)VA(0x100000);

  //  p->kstack[SSIZE-1] = (int)0x80000000;
  p->kstack[SSIZE-1] = VA(0);
  // -|-----goUmode-------------------------------------------------
  //  r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 ufp uip upc|string       |
  //----------------------------------------------------------------
  //  14 13 12 11 10 9  8  7  6  5  4   3    2   1 |             |

  enqueue(&readyQueue, p);

  kprintf("proc %d kforked a child %d: ", running->pid, p->pid); 
  printQ(readyQueue);

  return p;
}

//UMODE FORK IDENTICAL PROC CHILD
int kufork()
{
  int i;
  char *PA, *CA;
  int *ptable, pentry;
  PROC *p = dequeue(&freeList);
  

   // build p's pgtable 
  p->pgdir = (int *)(0x600000 + (p->pid - 1)*0x4000);
  ptable = p->pgdir;
  // initialize pgtable
  for (i=0; i<4096; i++)
    ptable[i] = 0;
  pentry = 0x412;
  for (i=0; i<258; i++){
    ptable[i] = pentry;
    pentry += 0x100000;
  }
  // ptable entry flag=|AP0|doma|1|CB10|=110|0001|1|1110|=0xC3E or 0xC32       
  //ptable[2048] = 0x800000 + (p->pid - 1)*0x100000|0xC3E;
  ptable[2048] = 0x800000 + (p->pid - 1)*0x100000|0xC32;


  if (p==0){ printf("fork failed\n"); return -1; }
  p->ppid = running->pid;
  p->parent = running;
  p->status = READY;
  p->priority = 1;
 
  PA = (char *)(running->pgdir[2048] & 0xFFFF0000); // parent Umode PA
  CA = (char *)(p->pgdir[2048] & 0xFFFF0000); // child Umode PA
  
  // Copy contents of src[] to dest[] 
  //for (int i=0; i<1000; i++) 
  //  CA[i] = PA[i]; 
  //mymemcpy(CA, PA, 0x100000); // copy 1MB Umode image
  memcpy(CA, PA, 0x100000);
  for (i=1; i <= 14; i++){ // copy bottom 14 entries of kstack
    p->kstack[SSIZE-i] = running->kstack[SSIZE-i];
  }
  p->kstack[SSIZE - 14] = 0; // child return pid = 0
  p->kstack[SSIZE-15] = (int)goUmode; // child resumes to goUmode
  p->ksp = &(p->kstack[SSIZE-28]); // child saved ksp
  p->usp = running->usp; // same usp as parent
  p->cpsr = running->cpsr; // same spsr as parent //DOUBLE CHECK IF BUGGY ucpsr?
  enqueue(&readyQueue, p);
  printf("Reached inside kufork()\n");
  return p->pid;
}

/* HELPER FUNCTIONS */
void mystrcat(char *destination, const char *source)
{
// return if no memory is allocated to the destination
	if (destination == NULL)
		return;

	// copy the C-string pointed by source into the array
	// pointed by destination
	while (*source != '\0')
	{
		*destination = *source;
		destination++;
		source++;
	}

	// include the terminating null character
	*destination = '\0';
}

char *mystrcpy(char *destination, const char *source)
{
  while ((*destination++ = *source++) != '\0');
}

//Replace current image wi Cmd line args in new process with new Umode Image
kexec(char* cmdline)
{
  /*
    ALGORITHM
  1. fetch cmdline from Umode space;
  2. tokenize cmdline to get cmd filename;
  3. check cmd file exists and is executable; return −1 if fails;
  4. load cmd file into process Umode image area;
  5. copy cmdline to high end of usatck, e.g. to x = high end−128;
  6. reinitialize syscall kstack frame to return to VA = 0 in Umode;
  7. return x;
  */
  int i, upa, usp;
  char *cp, kline[128], file[32], filename[32];
  PROC *p = running;
  mystrcpy(kline, cmdline); // fetch cmdline into kernel space
  // get first token of kline as filename
  cp = kline; i = 0;
  while(*cp != ' ' && *cp != '\0'){
    filename[i] = *cp;
    i++;
    cp++;
  }
  filename[i] = 0;
  file[0] = 0;
  //if (filename[0] != '/') // if filename relative
  //  mystrcpy(file, "/bin/"); // prefix with /bin/
  //mystrcat(file, filename);
  upa = p->pgdir[2048] & 0xFFFF0000; // PA of Umode image
  // loader return 0 if file non-exist or non-executable
  if (!load(filename, p))
    return -1;
  // copy cmdline to high end of Ustack in Umode image
  usp = upa + 0x100000 - 128; // assume cmdline len < 128
  mystrcpy((char *)usp, kline);
  p->usp = (int *)VA(0x100000 - 128);
  // fix syscall frame in kstack to return to VA=0 of new image
  for (i=2; i<14; i++) // clear Umode regs r1-r12
    p->kstack[SSIZE-i] = 0;
  p->kstack[SSIZE-1] = (int)VA(0); // return uLR = VA(0)
  return (int)p->usp; // will replace saved r0 in kstack
}