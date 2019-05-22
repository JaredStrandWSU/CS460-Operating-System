




#include "type.h"
#include "string.c"

PROC proc[NPROC];      // NPROC PROCs
PROC *freeList;        // freeList of PROCs 
PROC *readyQueue;      // priority queue of READY procs
PROC *running;         // current running proc pointer
TIMER *tp[4]; // 4 TIMER structure pointers

PROC *sleepList;       // list of SLEEP procs
int procsize = sizeof(PROC);

#define printf kprintf
#define gets kgets
#define NULL 0

#include "timer.c"
#include "kbd.c"
#include "vid.c"
#include "exceptions.c"

#include "queue.c"
#include "wait.c"      // include wait.c file

/*******************************************************
  kfork() creates a child process; returns child pid.
  When scheduled to run, child PROC resumes to body();
********************************************************/
int body(), tswitch(), do_sleep(), do_wakeup(), do_exit(), do_switch();
int do_kfork();
int scheduler();
void timer_handler();

int kprintf(char *fmt, ...);

void copy_vectors(void) {
    extern u32 vectors_start;
    extern u32 vectors_end;
    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;
    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}

void IRQ_handler()
{
    int vicstatus, sicstatus;
    int ustatus, kstatus;

    // read VIC SIV status registers to find out which interrupt
    vicstatus = VIC_STATUS;
    sicstatus = SIC_STATUS;  
    if (vicstatus & 0x80000000){ // SIC interrupts=bit_31=>KBD at bit 3 
       if (sicstatus & 0x08){
          kbd_handler();
       }
    // VIC status BITs: timer0,1=4, uart0=13, uart1=14
    if (vicstatus & (1<<4)){ // bit4=1:timer0,1
        if (*(tp[0]->base+TVALUE)==0) // timer 0
            timer_handler(0);
    }
}

char *status[ ] = {"FREE", "READY", "SLEEP", "ZOMBIE"};
    
int body()   // process body function
{
  int c;
  char cmd[64];
  printf("proc %d starts from body()\n", running->pid);
  while(1){
    printf("***************************************\n");
    printf("proc %d running: parent=%d\n", running->pid,running->ppid);
    printChildList(running->child);
    printList("readyQueue", readyQueue);
    printSleepList(sleepList);
    menu();
    printf("enter a command : ");
    gets(cmd);
    //we can add a new timer here :) create_timer();
    if (strcmp(cmd, "ps")==0)
      do_ps();
    if (strcmp(cmd, "fork")==0)
      do_kfork();
    if (strcmp(cmd, "switch")==0)
      do_switch();
    if (strcmp(cmd, "exit")==0)
      do_exit();
    if (strcmp(cmd, "sleep")==0)
      do_sleep();
    if (strcmp(cmd, "wakeup")==0)
      do_wakeup();
    if (strcmp(cmd, "wait")==0)
      do_wait();
  }
}

int kfork()
{
  int i;
  PROC *p = dequeue(&freeList);
  if (p==0){
    kprintf("kfork failed\n");
    return -1;
  }
  p->ppid = running->pid;
  p->parent = running;
  p->status = READY;
  p->priority = 1;
  
  //My code
  if(running->child)
  {
    PROC *temp = running->child;
    while(temp->sibling)
    {
      //child has a sibling
      temp = temp->sibling;
    }
    temp->sibling = p;
  }
  else
  {
    running->child = p;
    printf("I am the first born child!\n");
  }
// set kstack to resume to body
//  HIGH    -1  -2  -3  -4  -5 -6 -7 -8 -9 -10 -11 -12 -13 -14
//        ------------------------------------------------------
// kstack=| lr,r12,r11,r10,r9,r8,r7,r6,r5, r4, r3, r2, r1, r0
//        -------------------------------------------------|----
//	                                              proc.ksp
  for (i=1; i<15; i++)
    p->kstack[SSIZE-i] = 0;        // zero out kstack

  p->kstack[SSIZE-1] = (int)body;  // saved lr -> body()
  p->ksp = &(p->kstack[SSIZE-14]); // saved ksp -> -14 entry in kstack
 
  enqueue(&readyQueue, p);
  return p->pid;
}

/******************** DO FUNCTIONS ********************/
int do_ps()
{
  int i;
  PROC *p;
  printf("PID  PPID  status\n");
  printf("---  ----  ------\n");
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    printf(" %d    %d    ", p->pid, p->ppid);
    if (p == running)
      printf("RUNNING\n");
    else
      printf("%s\n", status[p->status]);
  }
}

int do_kfork()
{
   int child = kfork();
   if (child < 0)
      printf("kfork failed\n");
   else{
      printf("proc %d kforked a child = %d\n", running->pid, child); 
      printList("readyQueue", readyQueue);
   }
   return child;
}

int do_switch()
{
   tswitch();
}

int do_exit()
{
  kexit(running->pid);  // exit with own PID value 
}

int do_sleep()
{
  
  int event;
  printf("enter an event value to sleep on : ");
  event = geti();
  ksleep(event);
}

int do_wakeup()
{
  int event;
  printf("enter an event value to wakeup with : ");
  event = geti();
  kwakeup(event);
}

int do_wait()
{
  int status, result;
  PROC *tempParent, *tempChild;
  //running->status;
  do{
    //either return zombie pid or sleep
    result = kwait(&status);
    if(result == -1)
      break;
    if(result) //positive result found a pid
    {
      //found zombie child under running proc
      //remove from family tree

      tempParent = running;
      tempChild = running->child;
      if(tempChild->pid == result)
      {
        //zombie node is first child
        running->child = running->sibling;
      }
      else
      {
        while(tempChild->sibling->pid != result)
        {
          tempChild = tempChild->sibling;
        }
      }
      //zombie child is tempChild->sibling, cut p out of linkage
      tempChild->sibling = tempChild->sibling->sibling;
    }
  }
  while(result);
  //error no children
  result = kwait(&status);


  printf("Exit value of pid %d: %d\n",running->pid, status);
}

/******************** MAIN FUNCTION ********************/
int main()
{ 
   int i; 
   char line[128]; 
   u8 kbdstatus, key, scode;
   KBD *kp = &kbd;
   color = WHITE;
   row = col = 0; 

   fbuf_init();
   kprintf("Welcome to Wanix in ARM\n");
   kbd_init();
   
   /* enable SIC interrupts */
   VIC_INTENABLE |= (1<<31); // SIC to VIC's IRQ31
   /* enable KBD IRQ */
   SIC_INTENABLE = (1<<3); // KBD int=bit3 on SIC
   SIC_ENSET = (1<<3);  // KBD int=3 on SIC
   *(kp->base+KCNTL) = 0x12;
   VIC_INTENABLE = 0;
   VIC_INTENABLE |= (1<<4); // timer0,1 at VIC.bit4
   //VIC_INTENABLE |= (1<<5); // timer2,3 at VIC.bit5
   timer_init();
   timer_start(0);
   
   
   init();

   printQ(readyQueue);
   kfork();   // kfork P1 into readyQueue  

   unlock();
   while(1){
     if (readyQueue)
        tswitch();
   }
}

/*********** scheduler *************/
int scheduler()
{ 
  printf("proc %d in scheduler()\n", running->pid);
  if (running->status == READY)
     enqueue(&readyQueue, running);
  printList("readyQueue", readyQueue);
  running = dequeue(&readyQueue);
  printf("next running = %d\n", running->pid);  
}

/******************** HELPER FUNCTIONS ********************/
void printChildList(PROC *child)
{
  printf("MY CHILD LIST: ");
  PROC *c;
  if(child == NULL)
  {
    printf("NO CHILDREN");
  }
  else
  {
    c = child;
    while(c != NULL)
    {
      printf("[%d ", c->pid);
      if(c->status == 1)
        printf("READY]->");
      if(c->status == 2)
        printf("SLEEP]->");
      if(c->status == 3)
        printf("ZOMBIE]->");
      c = c->sibling;
    }
    printf("NULL");
  }
  printf("\n");
}

// initialize the MT system; create P0 as initial running process
int init() 
{
  int i;
  PROC *p;
  for (i=0; i<NPROC; i++){ // initialize PROCs
    p = &proc[i];
    p->pid = i;            // PID = 0 to NPROC-1  
    p->status = FREE;
    p->priority = 0;      
    p->next = p+1;
  }
  proc[NPROC-1].next = 0;  
  freeList = &proc[0];     // all PROCs in freeList     
  readyQueue = 0;          // readyQueue = empty

  sleepList = 0;           // sleepList = empty
  
  // create P0 as the initial running process
  p = running = dequeue(&freeList); // use proc[0] 
  p->status = READY;
  p->priority = 0;
  p->ppid = 0;             // P0 is its own parent
  
  printList("freeList", freeList);
  printf("init complete: P0 running\n"); 
}

int menu()
{
  printf("***************************************\n");
  printf(" ps fork switch exit sleep wakeup wait \n");
  printf("***************************************\n");
}
