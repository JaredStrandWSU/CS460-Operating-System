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

int kgetpid()
{
  return running->pid;
}

int kgetppid()
{
  return running->ppid;
}

char *pstatus[]={"FREE   ","READY  ","SLEEP  ","BLOCK  ","ZOMBIE", " RUN  "};
int kps()
{
  int i; PROC *p; 
  for (i=0; i<NPROC; i++){
     p = &proc[i];
     kprintf("proc[%d]: pid=%d ppid=%d", i, p->pid, p->ppid);
     if (p==running)
       printf("%s ", pstatus[5]);
     else
       printf("%s", pstatus[p->status]);
     printf("name=%s\n", p->name);
  }
}

int kchname(char *s)
{ 
  kprintf("kchname: name=%s\n", s);
  strcpy(running->name, s);
  return 123;
}

int ktswitch()
{
  tswitch();
}

int kgetPA()
{
  return running->pgdir[2048] & 0xFFFF0000;
}

// called from svc_entry in ts.s
int svc_handler(int a, int b, int c, int d)
{
  int r; 

  switch(a){
     case 0: r = kgetpid();          break;
     case 1: r = kgetppid();         break;
     case 2: r = kps();              break;
     case 3: r = kchname((char *)b); break;
     case 4: r = ktswitch();         break;
     case 5: r = do_wait();          break; //calls do_wait -> kwait
     case 6: r = kexit(b);           break; //calls kexit()
     case 7: r = kfork((char *)b);   break; //calls kfork with filename of main program
     case 8: r = ksleep(b);          break; //calls sleep on event b
     case 9: r = kwakeup(b);         break; //calls wakeup on event b
     case 10: r = kufork();	     break; //calls fork to create identical child proc
     case 11: r = kexec(b);	     break; //calls exec on the supplied CMD

     case 90: r = kgetc() & 0x7F;    break; // & 0x7F
     case 91: r = kputc(b);          break;
     case 92: r = kgetPA();          break;
     default: printf("invalid syscole %d\n", a);
  }

  return r;  // return to goUmode in ts.s; replace r0 in Kstack with r
}

