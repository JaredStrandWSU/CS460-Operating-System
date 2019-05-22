/********** init.c file *************/
//program that runs as the image for P1, forked by P0, replaces current init.c somehow

#include "ucode.c"

int pid, child, status;
int stdin, stdout;
//

int main(int argc, char *argv[ ])
{
    int i = 0;
    // open /dev/tty0 as read and write in order to display messages on terminal
    stdin = open("/dev/tty0", 0);
    stdout = open("/dev/tty0", 1);

//    stdin_ttyS0 = open("/dev/ttyS0", 0);
//    stdout_ttyS0 = open("/dev/ttyS1", 1);

    //was having logic issues with having children running from multiple
    //ttys

    prints("Welcome to J-UNIX init.c Function!!!\n");
    prints("Fork a child login processes!\n");

    child = fork();
    //fork extra login children here for multiple ttys

    if (child) //did not fail to return fork child pid > 0, running as, child will run with child = 0;
    {
        parent();
    }
    else
    {
        exec("login /dev/tty0"); //new child runoff replaces its image with login
        //multiple execs based on PID of forked child
    }
}

int parent()
{
    while(1)
    {
        prints("Parent waiting for dead child\n");

        pid = wait(&status); //pid == dead child, wait for child to die

        if(pid == child) //dead child 
        {
            //a child login process has died, bury and create another
            child = fork();
            prints("Child PID\n");
            if(!child) 
            {
                exec("login /dev/tty0");
                //refork child based on cleanup PID, so always forking on tty
            }
        }
        else
        {
            //bury child and reloop to wait again. Non-login proc died
        }
        
    }
}