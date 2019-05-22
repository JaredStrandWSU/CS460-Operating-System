extern MINODE minode[NMINODE];
extern PROC   proc[NPROC], *running;

int mylseek(int fd, int position)
{

    //change OFT entry's offset to position but make sure NOT to over run either end
    //of the file.
    int originalPosition = running->fd[fd]->offset;
    if(position > running->fd[fd]->mptr->INODE.i_size || position < 0)
        return -1; //error
    running->fd[fd]->offset = position;
  return originalPosition;
}

//size of inode some x
//move to position y