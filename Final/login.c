/********** login.c file *************/
//This code runs as the main for a login process.
//This program takes user input and checks the /etc/passwd file
//for user credentials to login. When successful user found, fork
//a child sh process and wait for child to die.
/*
 Algorithm of login
// login.c : Upon entry, argv[0]=login, argv[1]=/dev/ttyX
#include "ucode.c"
int in, out, err; char name[128],password[128]
main(int argc, char *argv[])
{
(1). close file descriptors 0,1 inherited from INIT.
(2). open argv[1] 3 times as in(0), out(1), err(2).
(3). fixtty(argv[1]); // set tty name string in PROC.tty
(4). open /etc/passwd file for READ;
     while(1){
(5).    printf("login:"); gets(name);
        printf("password:"); gets(password);
        for each line in /etc/passwd file do{
            tokenize user account line;
(6).        if (user has a valid account){
(7).            change uid, gid to user's uid, gid; // chuid()
                change cwd to user's home DIR // chdir()
                close opened /etc/passwd file // close()
(8).            exec to program in user account // exec()
            }
        }
        printf("login failed, try again\n");
    }
}
*/
#include "ucode.c"
int in, out, err;

int tokenizeData(char *line, char *tokens[], char delimiter)
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

main(int argc, char *argv[])
{
    char name[128], password[128];
    char buf[1024];
    char *userLines[128];
    char *userTokensArr[128];
    int numLineTokens, numUsers, i, fd;
    int validUsername = 0, validPassword = 0;
//(1). close file descriptors 0,1 inherited from INIT.
    close(0); close(1);

//(2). open argv[1] 3 times as in(0), out(1), err(2).
    in  = open(argv[1], 0);
    out = open(argv[1], 1);
    err = open(argv[1], 2);

//(3). fixtty(argv[1]); // set tty name string in PROC.tty
    fixtty(argv[1]);
    prints("WELCOME TO J-UNIX LOGIN FUNCTION!!!\n\n");
    printf("PID: %d\n", getpid());
//(4). open /etc/passwd file for READ;
    fd = open("/etc/passwd", 0);
    read(fd, buf, 1024); //get size of file somehow. dynamic mem
    printf("buf / user list:\n%s\n", buf);
    //buf containes a string of 1024 bytes, chars, iterator number of users
    numUsers = tokenizeData(buf, userLines, '\n');
    printf("numUsers: %d\n", numUsers);
//(5.)
    while(1){
        while(1)
        {
            prints("login: ");
            gets(name);
            if(name[0] ==0)
                continue;
            else
                break;
        }
        while(1)
        {
            prints("password: ");
            gets(password);
            if(password[0] ==0)
                continue;
            else
                break;
        }

        for (i=0; i<numUsers; i++)
        {
            prints("Checking user input values against /etc/passwd user:\n");
            printf("input name           : %s\n", name);
            printf("input password       : %s\n", password);

            numLineTokens = tokenizeData(userLines[i], userTokensArr, ':');
            printf("/etc/passwd: username : %s\n", userTokensArr[0]);
            printf("/etc/passwd: password : %s\n", userTokensArr[1]);
            //printf("numLineTokens  : %d\n", numLineTokens);

            if(strcmp(userTokensArr[0], name) == 0) //Username == name ?
                validUsername = 1;
            if(strcmp(userTokensArr[1], password) == 0) //Password == input Password?
                validPassword = 1;

            printf("ValidUsername  : %d\n", validUsername);
            printf("ValidPassword  : %d\n", validPassword);

            if(validUsername && validPassword) //successfuly login
            {
                chuid(atoi(userTokensArr[3]), atoi(userTokensArr[2])); //change uid, gid to user's uid, gid; // chuid()
                chdir(userTokensArr[5]); //change cwd to user's home DIR // chdir()
                close(fd); //close opened /etc/passwd file // close()
                printf("Login Was Successful, launching user home program: %s.\n", userTokensArr[6]);
                exec(userTokensArr[6]); // (8). exec to program in user account // exec()
                prints("Logout Was Successful, returning to prompt.\n");
                break;
            }
            validUsername = 0;
            validPassword = 0;
            userTokensArr[0] = 0;
            getc();
        }
        prints("Login Failed, try again.\n");
    }
}