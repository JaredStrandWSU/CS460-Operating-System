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

/* crt0.c : main0(s) called from u.s, where s = oigianl command string
            tokenlize s into char *argv[ ] and call main(argc, argv).
 
    token() breaks up a string into argc of tokens, pointed by argv[]
*/
/*  #include "uinclude.h" */
//#include "ucode.c"

int argc;
char *argv[32];

void token(char *line)
{
  char *cp;
  cp = line;
  argc = 0;
  
  while (*cp != 0){
       while (*cp == ' ') *cp++ = 0;        
       if (*cp != 0)
           argv[argc++] = cp;         
       while (*cp != ' ' && *cp != 0) cp++;                  
       if (*cp != 0)   
           *cp = 0;                   
       else 
            break; 
       cp++;
  }
  argv[argc] = 0;
}

void showarg(int argc, char *argv[ ])
{
  int i;
  printf("argc=%d ", argc);
  for (i=0; i<argc; i++)
    //printf("argv[%d]=%s ", i, argv[i]);
    printf("%s ", argv[i]);
  prints("\n");
}
// BEFORE: r0 was trashed in goUmode(), so had to rely on r1->string
// NOW: r0 is NOT trashed in goUmode() ==> should be r0 alone
void main0(char *s)
{
  if (s){
  //   printf("s=%s\n", s);
     token(s);
  }
  // showarg(argc, argv);
  main(argc, argv);
}


