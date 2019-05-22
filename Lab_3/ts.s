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

.text
.code 32
.global reset
.global vectors_start, vectors_end
.global lock, unlock, int_off, int_on

reset:                 // entry point set in t.ld	
reset_handler:
  LDR sp, =svc_stack   // set SVC stack
  BL copy_vectors      // copy vector table to 0

  MRS r0, cpsr         // go into IRQ mode
  BIC r1, r0, #0x1F
  ORR r1, r1, #0x12
  MSR cpsr, r1
  LDR sp, =irq_stack   // set IRQ stack

  BIC r0, r0, #0x80    // mask in IRQ interrupt I-bit in CPSR
  MSR cpsr, r0         // back to SVC mode

  BL main              // call main() in C
  B .

.align 4
irq_handler:

  sub	lr, lr, #4
  stmfd	sp!, {r0-r10, fp, ip, lr}

  bl	IRQ_handler  

  ldmfd	sp!, {r0-r10, fp, ip, pc}^

lock:
	mrs r0, cpsr
	orr r0, r0, #0x80 // set CPSR I-bit to 1
	msr cpsr, r0
	mov pc, lr
unlock:
	mrs r0, cpsr
	BIC r0, r0, #0x80 // clear CPSR B-bit to 0 
	msr cpsr, r0
	mov pc, lr
	
int_off:                 // int cpsr = int_off()
  MRS r1, cpsr
  MOV r0, r1
  ORR r1, r1, #0x80      // set I-bit to 1
  MSR cpsr, r1           // load into CPSR => IRQ masked out
  mov pc, lr	

int_on:                  // int_off(cpsr)
  MSR cpsr, r0           // r0 = original CPSR: load into CPSR
  mov pc, lr	          


vectors_start:
  LDR PC, reset_handler_addr
  LDR PC, undef_handler_addr
  LDR PC, swi_handler_addr
  LDR PC, prefetch_abort_handler_addr
  LDR PC, data_abort_handler_addr
  B .
  LDR PC, irq_handler_addr
  LDR PC, fiq_handler_addr

reset_handler_addr:          .word reset_handler
undef_handler_addr:          .word undef_handler
swi_handler_addr:            .word swi_handler
prefetch_abort_handler_addr: .word prefetch_abort_handler
data_abort_handler_addr:     .word data_abort_handler
irq_handler_addr:            .word irq_handler
fiq_handler_addr:            .word fiq_handler

vectors_end:

.end
