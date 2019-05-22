/*********** t.c file of A Multitasking System *********/
#include "type.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PROC proc[NPROC];      // NPROC PROCs
PROC *freeList;        // freeList of PROCs
PROC *sleepList;       // list of sleepy tired PROCs
PROC *readyQueue;      // priority queue of READY procs
PROC *running;         // current running proc pointer
#include "queue.c"     // include queue.c file

/*******************************************************
  kfork() creates a child process; returns child pid.
  When scheduled to run, child PROC resumes to body();
********************************************************/
int body();
int tswitch();

//SYSTEM COMMANDS

int kfork()
{
  int  i;
  PROC *p = dequeue(&freeList);
  if (!p){
     printf("no more proc\n");
     return(-1);
  }
  /* initialize the new proc and its stack */
  p->status = READY;
  p->priority = 1;       // ALL PROCs priority=1,except P0
  p->ppid = running->pid;
  p->parent = running;

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

  /************ new task initial stack contents ************
   kstack contains: |retPC|eax|ebx|ecx|edx|ebp|esi|edi|eflag|
                      -1   -2  -3  -4  -5  -6  -7  -8   -9
  **********************************************************/
  for (i=1; i<10; i++)               // zero out kstack cells
      p->kstack[SSIZE - i] = 0;
  p->kstack[SSIZE-1] = (int)body;    // retPC -> body()
  p->ksp = &(p->kstack[SSIZE - 9]);  // PROC.ksp -> saved eflag 
  enqueue(&readyQueue, p);           // enter p into readyQueue
  return p->pid;
}

int kexit(int exitCode)
{
  int i, wakeupP1 = 0;
  PROC *p = NULL; //Temp proc pointer
  if (running->pid==1){ 
    printf("other procs still exist, P1 can't die yet\n");
    return -1;
  }
  /* send children (dead or alive) to P1's orphanage */
  for (i = 1; i < NPROC; i++){
    p = &proc[i]; //set temp to proc[i] to see if a child of running proc
    if (p->status != FREE && p->ppid == running->pid){
      p->ppid = 1;
      p->parent = &proc[1];
      wakeupP1++;
    }
  }
  /* record exitValue and become a ZOMBIE */
  running->exitCode = exitCode;
  running->status = ZOMBIE;
  //if you are child your parent die you go to orphanage
  if(running->child)
    sendToOrphanage();
  //remove link to children
  running->child = NULL;
  /* wakeup parent and also P1 if necessary */
  kwakeup(running->parent); // parent sleeps on its PROC address
  if (wakeupP1)
    kwakeup(&proc[1]);
  tswitch(); // give up CPU
}

void sendToOrphanage(void)
{
  //add me to granpas children list
  PROC *p1 = &proc[1]; //grab P1
  PROC *temp = NULL;
  //P1 has no kids
  if(p1->child == NULL)
  {
    p1->child = running->child;
  }
  else
  {
    printf("P1 ADOPTS ALL THE KIDS!\n");
    temp = p1->child;
    while(temp->sibling != NULL)
    {
      temp = temp->sibling;
    }
    temp->sibling = running->child;
  }
  
}

int kwait(int *status){
  PROC *p; 
  int i, hasChild = 0;
  while(1){ // search PROCs for a child
    for (i=1; i<NPROC; i++){ // exclude P0
      p = &proc[i];
      if (p->status != FREE && p->ppid == running->pid){
        hasChild = 1; // has child flag
        if (p->status == ZOMBIE){ // lay the dead child to rest
        //printf("p: %d\n", p);
          printf("Found zombie child\n");
          *status = p->exitCode; // collect its exitCode 
          p->status = FREE; // free its PROC
          enqueue(&freeList, p); // ->Make sure we don't lose childPtr
          printf("Enqueueing into freelist\n");
          printList("freelist", freeList);
          return(p->pid); // return its pid
        }
      }
    }
    if (!hasChild)
    {
      printf("No child found && proc 1 not die\n");
      return -1; // no child, return ERROR
    }
    ksleep(running); // still has kids alive: sleep on PROC address
  }
  
}

void removeFromFamily(PROC *parent, PROC *p)
{

}

int ksleep(int event)
{
  printf("proc %d going to sleep on event=%d\n", running->pid, event);

  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  printList("sleepList", sleepList);
  tswitch();
}

int kwakeup(int event)
{
  PROC *temp, *p;
  temp = 0;
  printList("sleepList", sleepList);

  while (p = dequeue(&sleepList)){
    if (p->event == event){
      printf("wakeup %d\n", p->pid);
      p->status = READY;
      enqueue(&readyQueue, p);
    }
    else{
      enqueue(&temp, p);
    }
  }
  sleepList = temp;
  printList("sleepList", sleepList);
}
/*
int ksleep(int event)
{
  //int sr = int_off(); //disable interrupts so we don't have a case of never waking up
  printf("Disabling INTERRUPT\n");
  printf("proc %d going to sleep on event=%d\n", running->pid, event);

  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  printList("sleepList", sleepList);
  tswitch();

  printf("RE-ENABLE INTERRUPT\n");
  //int_on(sr);
}

int kwakeup(int event)
{
  PROC *temp, *p;
  temp = 0;
  printf("DISABLE INTERRUPTS\n");
  //int sr = int_off();
  
  printList("sleepList", sleepList);

  while (p = dequeue(&sleepList))
  {
    if (p->event == event){
      printf("wakeup %d\n", p->pid);
      p->status = READY;
      enqueue(&readyQueue, p);
    }
    else
    {
      enqueue(&temp, p);
    }
  }
  sleepList = temp;
  printList("sleepList", sleepList);
  printf("\nRE-ENABLE INTERRUPTS\n");
  //int_on(sr);
}
*/
//DO STATEMENTS + helper functions below

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
  kexit(running->pid); 
}

int do_jesus()
{
  int i;
  PROC *p;
  printf("Jesus performs miracles\n");
  for (i=1; i<NPROC; i++){
    p = &proc[i];
    if (p->status == ZOMBIE){
      p->status = READY;
      enqueue(&readyQueue, p);
      printf("raised a ZOMBIE %d to live again\n", p->pid);
    }
  }
  printList("readyQueue", readyQueue);
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

int do_sleep()
{
  int event;
  char str1[50];
  printf("enter an event value to sleep on : ");
  //event = geti(); //ARM
  scanf("%s", str1);
  event = atoi(str1);
  ksleep(event);
}

int do_wakeup()
{
  int event;
  char str1[50];
  printf("enter an event value to wakeup with : ");
  //event = geti(); //ARM
  scanf("%s", str1);
  event = atoi(str1);
  kwakeup(event);
}

int geti()
{
  //gets an integer value from the user input
  //call gets
  //translate result of gets into integer
}

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
int menu()
{
  printf("*****************************************************\n");
  printf(" ps  fork  switch  exit  jesus  wait  sleep  wakeup  \n");
  printf("*****************************************************\n");
}

char *status[ ] = {"FREE", "READY", "SLEEP", "ZOMBIE"};
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
    
int body()   // process body function
{
  int c;
  char cmd[64];
  printf("proc %d starts from body()\n", running->pid);
  while(1){
    printf("***************************************\n");
    printf("proc %d running: parent=%d\n", running->pid,running->ppid);
    printList("readyQueue", readyQueue);
    printChildList(running->child);
    menu();
    printf("enter a command : ");
    fgets(cmd, 64, stdin);
    cmd[strlen(cmd)-1] = 0;

    if (strcmp(cmd, "ps")==0)
      do_ps();
    if (strcmp(cmd, "fork")==0)
      do_kfork();
    if (strcmp(cmd, "switch")==0)
      do_switch();
    if (strcmp(cmd, "exit")==0)
      do_exit();
    if (strcmp(cmd, "jesus")==0)
      do_jesus();
    if (strcmp(cmd, "wait")==0)
      do_wait();
    if (strcmp(cmd, "sleep")==0)
      do_sleep();
    if (strcmp(cmd, "wakeup")==0)
      do_wakeup();
  }
}
// initialize the MT system; create P0 as initial running process
int init() 
{
  int i;
  PROC *p;
  for (i=0; i<NPROC; i++){ // initialize PROCs to freeList
    p = &proc[i];
    p->pid = i;            // PID = 0 to NPROC-1  
    p->status = FREE;
    p->priority = 0;      
    p->next = p+1;
  }
  proc[NPROC-1].next = 0;  
  freeList = &proc[0];     // all PROCs in freeList     
  readyQueue = 0;          // readyQueue = empty

  // create P0 as the initial running process
  p = running = dequeue(&freeList); // use proc[0] 
  p->status = READY;
  p->priority = 0;         // P0 has the lowest priority 0
  p->ppid = 0;             // P0 is its own parent
  printList("freeList", freeList);
  printf("init complete: P0 running\n"); 
}

/*************** main() function ***************/
int main()
{
   printf("Welcome to the MT Multitasking System\n");
   init();    // initialize system; create and run P0
   kfork();   // kfork P1 into readyQueue  
   while(1){
     printf("P0: switch process\n");
     while (readyQueue == 0); // loop if readyQueue empty
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
