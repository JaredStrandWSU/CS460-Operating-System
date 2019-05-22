extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern OFT    oft[NOFT];

extern char gpath[256];
extern char *name[64]; // assume at most 64 components in pathnames
extern int  n;
extern int  fd, dev;
extern int  nblocks, ninodes, bmap, imap, inode_start;
extern char pathname[256], parameter[256];

void mypfd()
{
    // print running's fd[ ]-> oft contenst to show what are opened
    int j = 0;
    printf(" fd     mode    offset    INODE\n");
    printf("----    ----    ------   --------\n");
    for (j=0; j<NFD; j++)
    {
        if(running->fd[j] != NULL)
        {
            printf(" %d     ", j);
            printf(" %d      ", running->fd[j]->mode);
            printf(" %d        ", running->fd[j]->offset);
            printf(" [dev=%d, ino=%d]\n", running->fd[j]->mptr->dev, running->fd[j]->mptr->ino);
        }
    }
}

int truncate(MINODE *mip)
{
    /* 
    1. release mip->INODE's data blocks;
        a file may have 12 direct blocks, 256 indirect blocks and 256*256
        double indirect data blocks. release them all.
    2. update INODE's time field

    3. set INODE's size to 0 and mark Minode[ ] dirty
    */
    printf("TRUNCATE UNDER CONSTRUCTION!\n");
    return 0;
}

int myopen() 
{
    int i,j = 0;
    int ino = 0;
    int mode = -1, offset = 0;
    MINODE *mip;
    INODE* ip;
    OFT* oftp = NULL;
    char stringMode[30];

    if(pathname[0] == 0)
    {
        printf("No path specified.\n");
        printf("Input path:\n");
        gets(pathname);
    }
    else
    {
        //pathname already given
    }

    if(parameter[0]==0)
    {
        //no mode given
        printf("No mode specified.\n");
        printf("Input mode formatted as: 0|1|2|3 for R|W|RW|APPEND\n");
        //sscanf("%d", &mode);
        gets(stringMode);
        mode = atoi(stringMode);
        printf("mode inputted: %d\n", mode);
    }
    else
    {
        mode = atoi(parameter);
    }

    //get pathname's inumber:
    if (pathname[0]=='/')
    {
        dev = root->dev;          // root INODE's dev
    }
    else
    {
        dev = running->cwd->dev; //cwd dev
    } 

    //get ino
    ino = getino(pathname); 

    //check if ino exists for pathname
    if (ino == 0)
    {
        printf("No ino found for pathname sorry.");
        return -1;
    }
    else
    {
        printf("ino found and it's value is: %d\n", ino);
    }

    //get its Minode pointer
    mip = iget(dev, ino); 

    //get file inode pointer from mip 
    ip = &mip->INODE;

    //Check if inode is a regular file type from it's i_mode field
    if (!S_ISREG(ip->i_mode)) //S_ISREG returns false if not a reg file
    {
        printf("Not a regular file!\n");
        iput(mip); //replace the mip 
        return -1;
    }
    else
    {
        //Check what i_imode field prints
        printf("i_mode: %d\n", ip->i_mode);
        //if its anything but R mode, iput and leave
    }
    
    //check to make sure available file descriptor in proc
    for (j=0; j < NFD; j++)
	{
		if (running->fd[j] == NULL)
			break; //found available one

		if (j == NFD -1)
		{
			printf("Out of available file descriptors!\n");
			iput(mip);
			return -1;
		}
    }

    //allocate/find a free OFT and populate it, connect it to file descriptor
    // use oftp
    for (i=0; i<NOFT; i++){

        oftp = &oft[i];
        
        //check if open already and not read mode
        //mode !- 0, can't open multiple for other non-read mode
        //refcount > 0 - not first open
        //pointing to same file
        //current mode is open
        
        if(mode != 0 && oftp->refCount > 0 && oftp->mptr == mip && oftp->mode != 0)
        {
            printf("File already in use");
            iput(mip);
            return -1;
        }

        //if oftp not in use yet.
        if(oftp->refCount == 0) //single ref support
        {
            //allocated a free OFT to use, called oftp, now populate it with data
            switch(mode){
                case 0 :
                    oftp->offset = 0;     // R: offset = 0
                    break;
                case 1 :
                    truncate(mip);        // W: truncate file to 0 size
                    oftp->offset = 0;
                    break;
                case 2 :
                    oftp->offset = 0;     // RW: do NOT truncate file
                    break;
                case 3 :
                    oftp->offset =  ip->i_size;  // APPEND mode
                    break;
                default: printf("invalid mode\n");
                    return -1;
            }
            
            oftp->mode = mode;
            oftp->mptr = mip;
            oftp->refCount = oftp->refCount + 1;
            running->fd[j] = oftp;
            break;
        }

        if(i == (NOFT - 1))
        {
            //No free open file tables, return error.
            printf("maximum oft's in use!\n");
            iput(mip);
            return -1;
        }
    }

    //Set the time and dirty
    ip->i_atime = time(0L);
    mip->dirty = 1;

    return i; //index of new file descriptor
}