extern PROC   *running;

extern char gpath[256];
extern char *name[64]; // assume at most 64 components in pathnames
extern char pathname[256], parameter[256];

void myclose()
{
    int fd = 0;
    char stringfd[256];

    printf("Enter a file descriptor number to close: \n");
    mypfd();
    printf("fd #");
    gets(stringfd);
    fd = atoi(stringfd);
    
    //1. verify fd is within range.
    if(fd > NFD - 1 || fd < 0)
    {
        printf("fd not in range.\n");
        return;
    }

    //2. verify running->fd[fd] is pointing at a OFT entry
    if(running->fd[fd] == NULL)
    {
        printf("not pointing at an OFT entry.\n");
        return;
    }

    //3. The following code segments should be fairly obvious:
    OFT *oftp = running->fd[fd];
    running->fd[fd] = 0;
    oftp->refCount--;
    if (oftp->refCount > 0) // no need to iput, still open
        return;

    // last user of this OFT entry ==> dispose of the Minode[]
    MINODE *mip = oftp->mptr;
    iput(mip);

    return; 
}