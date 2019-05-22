int tswitch();

int ksleep(int event)
{
  int sr = int_off();
  printf("proc %d going to sleep on event=%d\n", running->pid, event);

  running->event = event;
  running->status = SLEEP;
  //dequeue(&readyQueue);
  enqueue(&sleepList, running);
  printSleepList(sleepList);
  printf("Calling tswitch()\n");
  tswitch();
  int_on(sr);
}

int kwakeup(int event)
{
  PROC *temp, *p;
  temp = 0;
  int sr = int_off();
  
  printf("Value of Wakeup event searching for: %d\n", event);
  printSleepList(sleepList);

  while (p = dequeue(&sleepList)){
    if (p->event == event) {
      printf("wakeup %d\n", p->pid);
      p->status = READY;
      enqueue(&readyQueue, p);
    }
    else {
      enqueue(&temp, p);
    }
  }
  sleepList = temp;
  printSleepList(sleepList);
  int_on(sr);
}

void sendToOrphanage(void)
{
  //add me to granpas children list
  PROC *p1 = &proc[1]; //grab P1
  PROC *temp = 0;
  //P1 has no kids
  if(p1->child == 0)
  {
    p1->child = running->child;
  }
  else
  {
    printf("P1 ADOPTS ALL THE KIDS!\n");
    temp = p1->child;
    while(temp->sibling != 0)
    {
      temp = temp->sibling;
    }
    temp->sibling = running->child;
  }
}

int kexit(int exitCode)
{
  int i, wakeupP1 = 0;
  PROC *p = 0; //Temp proc pointer
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
  running->child = 0;
  /* wakeup parent and also P1 if necessary */
  kwakeup(running->parent); // parent sleeps on its PROC address
  if (wakeupP1)
    kwakeup(&proc[1]);
  tswitch(); // give up CPU
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
          printList(freeList);
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