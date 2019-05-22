/*********** type.h file ************/
#define NPROC    9          // number of PROCs
#define SSIZE 1024          // stack size = 4KB

// PROC status
#define FREE     0          
#define READY    1
#define SLEEP    2
#define ZOMBIE   3

typedef struct proc{
    struct proc *next;      // next proc pointer       
    int  *ksp;              // saved sp: at byte offset 4
  
    int   pid;              // process ID
    int   ppid;             // parent process pid 
    int   status;           // PROC status=FREE|READY, etc. 
    int   priority;         // scheduling priority

    int   kstack[1024];     // process execution stack
    int   exitCode;
    int   event;            //Process sleep event
    
    struct proc *child;     // child process of current node
    struct proc *parent;    // pointer to parent process
    struct proc *sibling;   // linked list of proc pointers w/ same parent
}PROC;
