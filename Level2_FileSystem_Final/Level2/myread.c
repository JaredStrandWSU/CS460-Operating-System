extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern OFT    oft[NOFT];

int myread(int fd, char *buf, int nbytes)
{
    int count = 0;  // Counter
    int *ip;
    int avil = 0;   // Number of bytes still available in file.
    int lbk = 0;    // Logical block number of the file to start at based on the offset
    int blk = 0;    // Physical block based on logical INODE block
    int indirectPhyblk = 0; //physical double indirect block number based on lbk number
    int indirectOffset = 0;
    int startByte = 0; //Starting byte to find after finding the starting block based on offset
    char *cq = buf; // cq points at buf[ ]
    int remain = 0;
    char blkreadbuf[1024]; // Used to hold the phsyical block read from get_block()
    
    OFT *oftp = running->fd[fd]; //used to hold the file descriptor we are working on, shorthand.

    MINODE *mip = running->fd[fd]->mptr;

    avil = mip->INODE.i_size - oftp->offset; // number of bytes still available in file.

    while (nbytes && avil){

    //   Compute LOGICAL BLOCK number lbk and startByte in that block from offset;

            lbk       = oftp->offset / BLKSIZE; //integer division trick
            startByte = oftp->offset % BLKSIZE; //start byte of the start block
     
       // I only show how to read DIRECT BLOCKS. YOU do INDIRECT and D_INDIRECT
 
       if (lbk < 12){                     // lbk is a direct block
            printf("lbk < 12, Reading blocks.\n");
            blk = mip->INODE.i_block[lbk]; // map LOGICAL lbk to PHYSICAL blk
       }
       else if (lbk >= 12 && lbk < 256 + 12) { 
            //  indirect blocks, 12 byte offset for direct blocks
            printf("lbk >= 12, Reading indirect blocks.\n");
			//indirect blocks
			//They are located at iblock 12, get block 12 address to calc physical block
			get_block(mip->dev, mip->INODE.i_block[12], blkreadbuf);

            //12th block address in blkreadbuf, add the logical block we are at in the file
            //then offset by 12 since direct blocks alredy can read first 12,
            //address of block 12 + ( (12 to 255) - 12) = address[12] + (0) to (255) = pyshical i_block to read from
			ip = (int *)blkreadbuf + lbk - 12;
            blk = *ip;
       }
       else{ 
            //  double-indirect blocks, 256 byte offset for double-direct blocks
            printf("lbk >= 256, Reading double-indirect blocks.\n");
			//double indirect blocks
			//They are located at iblock 13
			get_block(mip->dev, mip->INODE.i_block[13], blkreadbuf); //get the 13th block for dbl indirect

			indirectPhyblk = (lbk - 256 - 12) / 256; //physical indirect block number based on logical block number minus direct and indirect
			indirectOffset = (lbk - 256 - 12) % 256; //Physcial indrect block start byte number minus direct and indirect

			ip = (int *)blkreadbuf + indirectPhyblk; //get the address of the block to read dbl indirect from

			get_block(mip->dev, *ip, blkreadbuf);  //get the physical double indirect block

			ip = (int *)blkreadbuf + indirectOffset; //calculate the double indirect block number based on offset
            blk = *ip;
       }
       printf("logical block value (lbk): %d\n", lbk);
       printf("physical value      (blk): %d\n", blk);

       //get the data block into readbuf[BLKSIZE] 
       get_block(mip->dev, blk, blkreadbuf);

       //copy from startByte to buf[ ], at most remain bytes in this block
       char *cp = blkreadbuf + startByte;   
       remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]

       while (remain > 0){
            *cq++ = *cp++;             // copy byte from readbuf[] into buf[]
             oftp->offset++;           // advance offset 
             count++;                  // inc count as number of bytes read
             avil--; nbytes--;  remain--;
             if (nbytes <= 0 || avil <= 0) 
                 break;
       }
 
       // if one data block is not enough, loop back to OUTER while for more ...

   }
   //while(buf != '\0')
   //{
   //    printf("%c", buf++);
   //}
   printf("myread: read %d char from file descriptor %d\n", count, fd);
   printf("remaining bytes: %d\n", remain);
   printf("nbytes to read : %d\n", nbytes);
   return count;   // count is the actual number of bytes read
}

int read_file()
{
    /*
  Preparations: 
    ASSUME: file is opened for RD or RW;
    ask for a fd  and  nbytes to read;
    verify that fd is indeed opened for RD or RW;
    return(myread(fd, buf, nbytes));
    */
    char fileDescriptorString[30], nbytesString[30];
    char *buf;
    int fdNum = -1, nbytes = -1;

    mypfd();
    printf("Please enter a file descriptor number (open for RD or RD mode)--->\nfdNum = ");
    gets(fileDescriptorString);
    fdNum = atoi(fileDescriptorString);
    printf("Please enter n bytes to read --->\nnbytes = ");
    gets(nbytesString);
    nbytes = atoi(nbytesString);
    if(running->fd[fdNum]->mode == 0 || running->fd[fdNum]->mode == 2)
    {
        //file is open for RD or RW mode.
        return (myread(fdNum, buf, nbytes));
    }

    printf("file not open for RD or RQ mode!\n");
    return -1;

}