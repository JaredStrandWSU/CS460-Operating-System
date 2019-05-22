#include "ucode.c"
typedef enum { false, true } bool;

int tokenizeString(char *line, char *tokens[], char delimiter)
{
  char *cp = line;
  int index = 0;
  /// Taken from 'token()' in crt0.c
  while (*cp != 0)
  {
    /// Trims whitespace
    while (*cp == ' ')
      *cp++ = 0;
    /// Scans string
    if (*cp != 0)
      tokens[index++] = cp;
    /// Splits lines
    while (*cp != delimiter && *cp != 0)
      cp++;
    if (*cp != 0)
      *cp = 0;
    else
      break;
    cp++;
  }
  return index;
}

int do_redirection(char *cmd, int redirectType)
{
  int i = 0;
  char *file;

  //parse command to see redirection type, return the type if any
  while (!redirectType && cmd[i]) //!0 && char is not '\0', so > 0
  {
    //input redirection '<'
    if (cmd[i] == '<')
    {
      redirectType = 1;
      cmd[i] = '\0'; //set '<' to null
      cmd[i + 1] = '\0'; //set space to null

      //get filename
      file = cmd + i + 2; //beginning of filename string
      
      while(*file == ' ') //clear any whitespace before the start of name
        file++;
      
      close(0);
      open(file, O_RDONLY); //open as stdout to another program, breaks if more spaces
    }
    //output redirection '>'
    else if ( cmd[i] == '>')
    {
      if ( cmd[i+1] == '>') //check if double arrow '>' '>'
      {
        redirectType = 2;
        cmd[i] = '\0'; //set '>' to null
        cmd[i + 1] = '\0'; //set space to null

        //get filename
        file = cmd + i + 2; //beginning of filename string
      
        while(*file == ' ') //clear any whitespace before the start of name
          file++;

        close(1);
        open(file, O_WRONLY|O_APPEND);
      }
      else
      {
        redirectType = 3; // single arrow '>'
        cmd[i] = '\0'; //set '>' to null
        cmd[i + 1] = '\0'; //set space to null

        //get filename
        file = cmd + i + 2; //beginning of filename string
      
        while(*file == ' ') //clear any whitespace before the start of name
          file++;

        close(1);
        open(file, O_WRONLY|O_CREAT);
      }
    }
  }
  return redirectType;
}

int redirect_check(char *command)
{
  int index = 0;
  int redirectType = 0;
  printf("Entering Redirect Check!\n");
  
  //parse command to see redirection type, return the type if any
  while (!redirectType && command[index]) //!0 && char is not '\0', so > 0
  {
    //input redirection '<'
    if (command[index] == '<')
    {
      redirectType = 1;
      printf("Redirect found and is : '<'\n");
    }

    //output redirection '>'
    else if ( command[index] == '>')
    {
      if ( command[index+1] == '>') //check if double arrow '>' '>'
      {
        redirectType = 2;
        printf("Redirect found and is : '>>'\n");
      }
      else
      {
        redirectType = 3; // single arrow '>'
        printf("Redirect found and is : '>'\n");
      }
    }
    index++;
  }
  return redirectType; //0 if none
}


void do_command(char *cmd)
{
  int redirectType = 0;
  prints("In do_command\n");
  printf("cmd value: %s\n", cmd);
  //check for redirects and then exec properly
  prints("About to call redirect checker to return the type of redirect.\n");
  redirectType = redirect_check(cmd);
  if(redirectType) // if 0(false) then  skip doing redirection
  {
    prints("We need to do some redirection!\n");
    do_redirection(cmd, redirectType);
    prints("redirection complete!\n");
  }
  else
  {
    prints("No redirect found.\n");
  }
  
  printf("about to exec with cmd: %s\n", cmd);
  exec(cmd); //run command
}

//ALGORITHM
int main(int argc, char *argv[ ])
{
  char cmdLine[128], temp[128];
  char *cmdLineTokensArr[128];
  int numCMDLineTokens = 0, i = 0, pid = 0, status = 0;
  int lpd[2];
  bool pipeFound = false;

  prints("\n      *** WELCOME TO J-UNIX SHELL ***      \n");
  while(1)
  {
    //get a command line string
    prints("#sh: ");
    gets(cmdLine); prints("\n");
    if (cmdLine[0] == 0)
      continue;
    
    printf("cmdLine: %s\n", cmdLine);
    
    for (i=0; i<128;i++)
      temp[i] = 0;

    for (i=0; i < 128; i++)
    {
      if(cmdLine[i] != '\0')
        temp[i] = cmdLine[i];
      else
      {
        temp[i] = '\0';
        break;
      }
    }

    printf("cmdLine before tokenize: %s\n", cmdLine);
    printf("temp before    tokenize: %s\n", cmdLine);

    numCMDLineTokens = tokenizeString(temp, cmdLineTokensArr, ' ');
    printf("cmdLine after tokenize: %s\n", cmdLine);

//commands begin
/*

*/
    //do the cmd directly if a non binary exec command
    if (strcmp(cmdLineTokensArr[0], "cd") == 0)
    {
      if(cmdLineTokensArr[1] == 0)
        {
          continue;
        } 
        else
        {
          chdir(cmdLineTokensArr[1]); continue; 
        }
    }

    if (strcmp(cmdLineTokensArr[0], "logout") == 0)
    { prints("Logging out...\n"); exit(0); }

////////////////commands end////////////////
    //for binary executable commands
    pid = fork();

    if (pid < 0)
    {
      prints("NO PROCS AVAILABLE: FORK FAILED!");
      exit(0); //just exit
    }

    if(pid) //parent running
    {
      pid = wait(&status);
      continue;
    }
    else
    {
      //child needs to handle the command since it's an exec
      //but we need to check for pipes and redirects.

      //create pipe descriptor
      //if needed
      //do pipe and pass in pipe descriptor
      //forked child process runs this line
      //checks if pipes in command, if not, runs single command
      i = 0;
      while (cmdLineTokensArr[i] != 0)
      {
        if (*cmdLineTokensArr[i] == '|')
        {
          pipeFound = true;
          break;
        }
        i++;
      }
      
      if(pipeFound == false) //no pipe
      {
        //no pipes in cmd
        do_command(cmdLine);
      }
      else
      {
        //PIPES FOUND
        //implement last

      }
      
      //do_pipe(cmdLine, 0);
    }
  }
}

/* commands to exec if pipes handled
    if (strcmp(cmdLineTokensArr[0], "ls") == 0)
    { exec(cmdLine); continue; }
    if (strcmp(cmdLineTokensArr[0], "cat") == 0)
    { exec(cmdLine); continue; }
    if (strcmp(cmdLineTokensArr[0], "cp") == 0)
    { exec(cmdLine); continue; }
    if (strcmp(cmdLineTokensArr[0], "grep") == 0)
    { exec(cmdLine); continue; }
    if (strcmp(cmdLineTokensArr[0], "more") == 0)
    { exec(cmdLine); continue; }
    if (strcmp(cmdLineTokensArr[0], "l2u") == 0)
    { exec(cmdLine); continue; }
*/

/*
int main(int argc, char *argv[ ])
{

(3). then (in sh) it loops forever (until "logout"):
      {
          prompts for a command line, e.g. cmdLine="cat filename"
          if (cmd == "logout") 
            syscall to die;

          // if just ONE cmd:  
          pid = fork();
          if (pid==0)
              exec(cmdLine);
          else
              pid = wait(&status);
      } 

  char cmdLine[128];
  char *inputArr[128];
  int inputTokens = 0, i = 0;
  prints("\n      *** WELCOME TO J-UNIX SHELL ***      \n");
  while(1)
  {
    prints("#sh: ");
    gets(cmdLine); prints("\n");
    if (cmdLine[0] == 0)
      continue;
    inputTokens = tokenizeString(cmdLine, inputArr, ' ');

    printf("cmd: %s\n", inputArr[0]);
    for(i=0; i < (inputTokens - 1); i++)
    {
      printf("arg[%d]: %s", i, inputArr[i+1]);
    }
    
    prints("\nDo a cool command!\n");
    //exec("ls");
    prints("\n      *** EXITING J-UNIX SHELL ***      \n");
    exit(0); //exit back to login terminal
  }
}
*/
/*
int do_pipe(char *cmdLine, int *pd)
{
  int hasPipe = 0;
  int lpd[2];
  char *head;
  char *tail;
  if(pd)
  {
    close(pd[0]);
    dup2(pd[1], 1);
    close(pd[1]);
  }

  //divide cmdLine into head and tail by rightmost pipe symbol
  hasPipe = scan(cmdLine, head, tail);
  if(hasPipe) {
      //create a pipe lpd; left pipe descriptor
      pid = fork();
      if(pid) { //Parent
        //as READER on lpd;
          close(lpd[1]);
          dup2(lpd[0],0);
          close(lpd[0]);
        do_command(tail);
      }
      else {
        do_pipe(head, lpd);
      }
  }
  else {
    do_command(cmdLine);
  }
}
*/
//Do command
/*
int do_command(char *cmdLine)
{
  char *head;
  char *tail;
  //scan cmdLine for I/O redirection symbols;
  scan(cmdLine, head, tail);
  
  //do I/O redirections;
  //head = cmdLine before redirections
  //exec(head);
}
*/

    /*
      if (redirectType)
      {
        //get the file
        printf("RedirectType: %d, command: %s\n", redirectType, command);
        printf("command[index-1] before: %c\n", command[index-1]);
        command[index - 1] = 0; //end the command string

        printf("command[index-1] after: %c\n", command[index -1]);
        command[index++] = 0;
        command[index++] = 0;

        if(command[index] == ' ')
        {
          command[index++] = 0;
        }
        //copy the 
        strcpy(rdFile, command+index); //
*/

//Scan for pipes
/*
int scan(char *cmdLine, char *head, char *tail)
{
  //divide cmdLine into head and tail by rightmost | symbol
  //cmdLine = cmd1 | cmd2 | ... | cmdn-1 | cmdn
  //   |<------- head ------>| tail |; return 1;
  //cmdLine = cmd1 ==> head=cmd1, tail=null; return 0;
}
*/