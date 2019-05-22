/*******************************************************
*                      t.c file                        *
*******************************************************/
typedef unsigned char  u8; //Shorthand notation
typedef unsigned short u16;//Shorthand notation
typedef unsigned long  u32;//Shorthand notation

#define TRK 18
#define CYL 36
#define BLK 1024

#include "ext2.h" //Contains File descriptor structures

typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;
GD    *gp;	//Pointer to Group Descriptor Struct
INODE *ip;	//Pointer to INODE
DIR   *dp;	//Pointer to Directory

char buf1[BLK], buf2[BLK];
int color = 0x0A;
u8 ino;	//Inode number


int prints(char *s)
{
	while(*s)
	{
		putc(*s++);
	}
}

int gets(char *s) 
{
  // write YOUR code
	while ((*s=getc()) != '\r')
		putc(*s++);
	*s = 0;
}

int getblk(u16 blk, char *buf)
{
  // readfd( (2*blk)/CYL, ( (2*blk)%CYL)/TRK, ((2*blk)%CYL)%TRK, buf);
  readfd( blk/18, ((blk)%18)/9, ( ((blk)%18)%9)<<1, buf);
}

u16 search(INODE *ip, char *name)
{
	int i; char c; DIR *dp;
	for (i=0; i <12; i++) {	//Assume a DIR has at most 12 direct blocks
		if ((u16)ip->i_block[i]){ //Direct block 0->12
			getblk((u16)ip->i_block[i], buf2);	//Get the block and place in buf2
			dp = (DIR *)buf2;	//Cast direct block as Directory strucutre
			while ((char *)dp < &buf2[BLK]) {	//While there is a lowe pointer than the Direct bock
				c = dp->name[dp->name_len];	//save last byte
				dp->name[dp->name_len] = 0; //make name into string
				prints(dp->name); putc(' ');//Show dp->name string
				if ( strcmp(dp->name, name) == 0 ) {
					prints("\n\r");
					return((u16)dp->inode);
				}
				dp->name[dp->name_len] = c; 	//restore last byte
				dp = (char *)dp + dp->rec_len;// ???
			}
		}
	}
	error(); //Jumps to error if can't find the filename
}

main()
{ 
  u16		i, ino, blk, iblk, ; // iblk: 2 byte block struct for bitmap
  u32		*up;
	char	c, temp[64];
	char	*name[2], filename[64];

	name[0] = "boot"; name[1] = filename;
	prints("bootname: ");
	gets(filename);
  //prints("reading block# 2 (GD)\n\r");
  getblk(2, buf1);	//Grabs Block number 2 from the mtx image stores 1024 bytes block

// 1. WRITE YOUR CODE to get iblk = bg_inode_table block number
  //prints("inode_block="); putc(iblk+'0'); prints("\n\r"); 
	gp = (GD *)buf1;	//Cast block #2 as group descriptor struct to access table
	iblk = (u16)gp->bg_inode_table; //Cast inode table as 16 bit since BIOS 2 byte max,
	getblk(iblk, buf1);	//Read first inode block
	ip = (INODE *)buf1 + 1;	//ip->root inode #2
	
	for (i=0; i<2; i++){
		ino = search(ip, name[i]) - 1; //Get the ino for the root inode
		
		if (ino < 0) 
			error();					 //If search() returned 0
		getblk(iblk+(ino/8), buf1);		 //Read inode block of ino
		ip = (INODE *)buf1 + (ino % 8);// ??? Each block mod%8???
	}
	
	if ((u16)ip->i_block[12]) //Read indirect block into buf2, if exists
		getblk((u16)ip->i_block[12], buf2); //Reading block into buf2
	
	setes(0x1000);	//Set ES register to loading segment - why?
	
	for (i=0; i<12; i++){
		getblk((u16)ip->i_block[i], 0); // ???
		inces(); putc('*');	//Show a * for each direct block loaded - inces() ?
	}
	if ((u16)ip->i_block[12]){	//Load indirect blocks, if any
		up = (u32 *)buf2;					// ???
		while(*up){								// ???
			getblk((u16)*up, 0); 	// ???
			up++;
			inces(); putc('.'); 		// Show a . for each indirect block loaded
		}
	}
	prints("Ready to go?"); getc();



// 2. WRITE YOUR CODE to get root inode
  //prints("read inodes begin block to get root inode\n\r");

// 3. WRITE YOUR CODE to step through the data block of root inode
  //prints("read data block of root DIR\n\r");
   
// 4. print file names in the root directory /
}  