typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;

#include "string.c"
#include "uio.c"

int ubody(char *name)
{
  int pid, ppid;
  char line[64];
  u32 mode,  *up;

  mode = getcsr();
  mode = mode & 0x1F;
  printf("CPU mode=%x\n", mode);
  pid = getpid();
  ppid = getppid();

  while(1){
    printf("This is file: %s\n", name);
    printf("This is process #%d in Umode at %x parent=%d\n", pid, getPA(),ppid);
    umenu();
    printf("input a command : ");
    ugetline(line); 
    uprintf("\n"); 
 
    if (strcmp(line, "getpid")==0)
       ugetpid();
    if (strcmp(line, "getppid")==0)
       ugetppid();
    if (strcmp(line, "kfork")==0)
       ukfork();
    if (strcmp(line, "ps")==0)
       ups();
    if (strcmp(line, "chname")==0)
       uchname();
    if (strcmp(line, "switch")==0)
       uswitch();
    if (strcmp(line, "sleep")==0)
       usleep();
    if (strcmp(line, "wakeup")==0)
       uwakeup();       
    if (strcmp(line, "wait")==0)
       uwait();
    if (strcmp(line, "exit")==0)
       uexit();
    if (strcmp(line, "fork")==0)
       ufork();
    if (strcmp(line, "exec")==0)
       uexec();
    if (strcmp(line, "open")==0)
       uopen();
    if (strcmp(line, "close")==0)
       uclose();
  }
}

int umenu()
{
  uprintf("----------------------------------------------------------------------------------\n");
  uprintf("getpid getppid kfork ps chname switch sleep wakeup wait exit fork exec open close \n");
  uprintf("----------------------------------------------------------------------------------\n");
}

int getpid()
{
  int pid;
  pid = syscall(0,0,0,0);
  return pid;
}    

int getppid()
{ 
  return syscall(1,0,0,0);
}

int ugetpid()
{
  int pid = getpid();
  uprintf("pid = %d\n", pid);
}

int ugetppid()
{
  int ppid = getppid();
  uprintf("ppid = %d\n", ppid);
}

int ups()
{
  return syscall(2,0,0,0);
}

int uchname()
{
  char s[32];
  uprintf("Input a name string : ");
  ugetline(s);
  uprintf("\n");
  return syscall(3,s,0,0);
}

int uswitch()
{
  return syscall(4,0,0,0);
}

int ukfork()
{
//Forks a child process with usermode image of path specified. calls kfork
  char filename[48];

  //get filename
  uprintf("Input a filename : ");
  ugetline(filename);
  uprintf("\n");

  return syscall(7,filename,0,0);
}

int ufork()
{
//forks a new child process with identical user image and switches to it.
  //calls fork
  int pid;

  uprintf("Executing Fork() to create identical Umode image, Current Process stats below:\n");
  ugetppid();
  ugetpid();
  pid = syscall(10,0,0,0); //calls kufork() in kernel mode
  uprintf("Fork() called, Returned to Usermode as New Child Process stats below:\n");
  ugetppid();
  ugetpid();
/* future shell implemenation
  if (pid)
  {
    pid = wait(&status); //might be broken
  }
  else {
    uexec();
  }
     case 10: r = kufork();	     break; //calls fork to create identical child proc
     case 11: r = kexec(b);	     break; //calls exec on the supplied CMD
*/

}

int uexec()
{
//changes the current user mode image based on CMD argc and argv
  char CMD[48];

  //get command to exec
  uprintf("Input a CMD : ");
  ugetline(CMD);
  uprintf("\n");

//create syscall for kexec, pass in CMD string to be tokenized
  uprintf("Executing Exec() to create a new Umode image off the current process, will make it fork a child in future when I run the shell program instead\n");
  return syscall(11,CMD,0,0);
}

int usleep()
{
  int sleepVal = 0;

  //get value to sleep on
  uprintf("input a value to sleep on : ");
  sleepVal = geti();
  //uprintf("Your input value is : %d\n", sleepVal);
  return syscall(8,sleepVal,0,0);
}

int uwakeup()
{
  int wakeupVal = 0;

  //get value to sleep on
  uprintf("input a value to wakeup on : ");
  wakeupVal = geti(); 
  //uprintf("Your input value is : %d\n", wakeupVal);
  return syscall(9,wakeupVal,0,0);
}

int uwait()
{
  return syscall(5,0,0,0);
}

int uexit()
{
  int exitCode = 0;

  //get exit code from user
  uprintf("input an exit code : ");
  exitCode = geti(); 
  uprintf("\n");

  return syscall(6,exitCode,0,0);
}


int ugetc()
{
  return syscall(90,0,0,0);
}

int uputc(char c)
{
  return syscall(91,c,0,0);
}

int getPA()
{
  return syscall(92,0,0,0);
}
