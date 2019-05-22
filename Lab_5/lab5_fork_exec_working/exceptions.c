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

// interrupts.c file

int kprintf(char *fmt, ...);

/* all other handlers are infinite loops */

void __attribute__((interrupt)) undef_handler(void)
{
  kprintf("undef exception\n");
  while(1);
}

//void __attribute__((interrupt)) swi_handler(void) { for(;;); }

void __attribute__((interrupt)) prefetch_abort_handler(void) 
{ 
  kprintf("prefetch exception\n"); 
  while(1);
}

void DATA_handler(void) 
{
  u32 fault_status, fault_addr, domain, status;
  int spsr = get_spsr();
  int oldcolor = color;
  color = RED;
  kprintf("data_abort exception in ");
  if ((spsr & 0x1F)==0x13)
    kprintf("SVC mode\n");
  if ((spsr & 0x1F)==0x10)
    kprintf("USER mode\n");

  fault_status = get_fault_status();
  fault_addr   = get_fault_addr();
  // fault_status = 7654 3210
  //                doma status
  domain = (fault_status & 0xF0) >> 4;
  status = fault_status & 0xF;
  kprintf("domain=%x status=%x addr=%x\n", domain, status, fault_addr);
  //kprintf("data_abort handler return\n");
  color = oldcolor;
}
void __attribute__((interrupt)) fiq_handler(void) { for(;;); }


