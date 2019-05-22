extern char pathname[256];
extern char parameter[256];
extern PROC *running;
extern int dev;

int mywrite(int fd, char buf[ ], int nbytes)
{
   int lbk, blk, startByte, i, indirect_blk_number, indirect_offset, remain;
   int *inodePointer;
   char ibuf[1024]; //to hold indirect block addresses
   char wbuf[1024];
   char *cp, *cq = buf;
   OFT *oftp = running->fd[fd];
   MINODE *mip = running->fd[fd]->mptr;
   
   while (nbytes > 0 ){

   //  compute LOGICAL BLOCK (lbk) and the startByte in that lbk:

      lbk       = oftp->offset / BLKSIZE;
      startByte = oftp->offset % BLKSIZE;

    // I only show how to write DIRECT data blocks, you figure out how to 
    // write indirect and double-indirect blocks.

      if (lbk < 12){                         // direct block
         if (mip->INODE.i_block[lbk] == 0){   // if no data block yet
            mip->INODE.i_block[lbk] = balloc(mip->dev);// MUST ALLOCATE a block
         }
         blk = mip->INODE.i_block[lbk];      // blk should be a disk block now
      }
      else if (lbk >= 12 && lbk < 256 + 12){ // INDIRECT blocks 
         // HELP INFO:
         if (mip->INODE.i_block[12] == 0){
            
            //allocate a block for it; allocate 256 block pointers
            //zero out the block on disk !!!!
            mip->INODE.i_block[12] = balloc(mip->dev);// MUST ALLOCATE a block
            get_block(mip->dev, mip->INODE.i_block[12], ibuf);
            for (i=0; i < BLKSIZE; i++)
            {
               ibuf[i] = 0;
            }
            put_block(mip->dev, mip->INODE.i_block[12], ibuf);
         }

         get_block(dev, mip->INODE.i_block[12], ibuf); //get the block of pointers
         inodePointer = (int*)ibuf + lbk - 12; //point to block for writing to int * cast
         blk = *inodePointer; //blk now contains data at the physical block


			//if data block does not exist yet, have to allocate
			if(blk == 0)
			{
				*inodePointer = balloc(mip->dev); //allocate a disk block
				blk = *inodePointer; //assign its content to blk
         }
     }
     else{
         // double indirect blocks
         //if first indirection block doesnt exist we need to allocate it
			if(mip->INODE.i_block[13] == 0)
			{
            //allocate block for dbl indirect pointers
				mip->INODE.i_block[13] = balloc(mip->dev);
				//fill it with 0's!
				get_block(mip->dev, mip->INODE.i_block[13], ibuf);
				for(i = 0; i < BLKSIZE; i++)
					ibuf[i] = 0;
				put_block(mip->dev, mip->INODE.i_block[13], ibuf);
			}

         //Get the first indirect block of pointers into ibuf
			get_block(mip->dev, mip->INODE.i_block[13], ibuf);

         //calulate the 1st indirect block number
			indirect_blk_number = (lbk - 256 - 12) / 256; //690 - 268 / 256 = 1.64 -> 1 pointer
			indirect_offset     = (lbk - 256 - 12) % 256; //dbl indirect block offset number index 166

			inodePointer = (int *)ibuf + indirect_blk_number; //points to block containing dbl indirect pointers
			blk = *inodePointer; //contains block of pointers to blocks of pointers

			//if no block yet, have to allocate it
			if(!blk)
			{
				inodePointer = balloc(mip->dev);
				blk = *inodePointer;

				get_block(mip->dev, blk, ibuf);
				for(i = 0; i < BLKSIZE; i++)
					ibuf[i] = 0;
				put_block(mip->dev, blk, ibuf);
			}

         //guaranteed first pointer block allocated so now get it.
			get_block(mip->dev, blk, ibuf);

         //Needs to be offset x4??
			inodePointer = (int*)ibuf + indirect_offset; //indirect block address, plus secondary offset
			blk = *inodePointer;

			if(!blk)
			{
				*inodePointer = balloc(mip->dev);
				blk = *inodePointer;
				put_block(mip->dev, blk, ibuf);
         }
     }
      printf("blk value: %d\nMade it past blocks\n", blk);
      printf("issue here with get_block\n");
     // all cases come to here : write to the data block
      printf("mip->dev value: %d, blk value: %d, wbuf value: %s\n", mip->dev, blk, wbuf);
     get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]
      printf("mip->dev value: %d, blk value: %d, wbuf value: %s\n", mip->dev, blk, wbuf);
     *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
     remain = BLKSIZE - startByte;     // number of BYTEs remain in this block
   
         printf("right before while loop\n");
     while (remain > 0){               // write as much as remain allows  
     printf("inside while loop\n");
     printf("remain = %d\n", remain);
           *cp++ = *cq++;              // cq points at buf[ ]
           nbytes--; remain--;         // dec counts
           oftp->offset++;             // advance offset
           if (oftp->offset > mip->INODE.i_size)  // especially for RW|APPEND mode
               mip->INODE.i_size++;    // inc file size (if offset > fileSize)
           if (nbytes <= 0) break;     // if already nbytes, break
     }
     put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk
     
     // loop back to outer while to write more .... until nbytes are written
  }

  mip->dirty = 1;       // mark mip dirty for iput() 
  printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);           
  return nbytes;
  
}

int write_file()
{
   /*
     1. Preprations:
     ask for a fd   and   a text string to write;

  2. verify fd is indeed opened for WR or RW or APPEND mode

  3. copy the text string into a buf[] and get its length as nbytes.

     return(mywrite(fd, buf, nbytes));
*/

   char fileDescriptorString[30];
   char buf[256];
   int fdNum = -1;
   int nbytes;

   mypfd();
   printf("Please enter a file descriptor that's open for W, RW, or APPEND mode --->\nfdNum = ");
   gets(fileDescriptorString);
   fdNum = atoi(fileDescriptorString);
   printf("Please enter a string to write to the file --->\nString = ");
   gets(buf);
   nbytes = strlen(buf);

   if(running->fd[fdNum]->mode == 1 || running->fd[fdNum]->mode == 2 || running->fd[fdNum]->mode == 3) //W, RW, or APPEND mode
   {
      //file is open for W, RW, or APPEND mode.
      return (mywrite(fdNum, buf, nbytes));
   }
}