/****************************************************************************
*                   KCW testing ext2 file system                            *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

#include "type.h"
#include "Level2/myopen.c"
#include "Level2/myclose.c"
#include "Level2/mylseek.c"
#include "Level2/myread.c"
#include "Level2/mywrite.c"
#include "Level2/mycat.c"
#include "Level2/mycp.c"

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
OFT    oft[NOFT];

char gpath[256];
char *name[64]; // assume at most 64 components in pathnames
int  n;
int  fd, dev;
int  nblocks, ninodes, bmap, imap, inode_start;
char pathname[256], parameter[256];

/********************
 * .o files
#include "util.c"
#include "alloc_dealloc.c"
#include "cd_ls_pwd.c"
#include "mkdir.c"
#include "creat.c"
#include "rmdir.c"
#include "link_unlink.c"
*********************/
int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;
  OFT    *oftp;

  printf("init()\n");

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->uid = 0;
    p->cwd = 0;
    p->status = FREE;
    for (j=0; j<NFD; j++)
      p->fd[j] = 0;
  }
  //define OFT oft[NOFT] in main(); initialize them to FREE, only 16 open file tables and 64 fd.
  for (i=0; i<NOFT; i++){
    oftp = &oft[i];
    oftp->mode = -1;
    oftp->mptr = NULL;
    oftp->offset = 0;
    oftp->refCount = 0;
  }
}

// load root INODE and set root pointer to it
int mount_root()
{  
  printf("mount_root()\n");
  root = iget(dev, 2);
}

char *disk = "disk";
int main(int argc, char *argv[ ])
{
  int ino, bytesRead = 0, bytesWrote = 0;
  char buf[BLKSIZE];
  char line[256], cmd[64], path[128];

  if (argc > 1)
    disk = argv[1];

  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);  exit(1);
  }
  dev = fd;

  /********** read super block at 1024 ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system *****************/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  inode_start = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);

  init();  
  mount_root();
  //printf("root refCount = %d\n", root->refCount);
  printf("mydisk mounted on / OK\n");

  printf("creating P0 as running process\n");
  running = &proc[0];
  running->status = READY;
  running->cwd = iget(dev, 2);
  //printf("root refCount = %d\n", root->refCount);

  //printf("hit a key to continue : "); getchar();
  while(1){
    printf("Level 1: [ls|cd|pwd|mkdir|creat|rmdir|link|unlink|symlink|readlink]\n");
    printf("Level 2: [open|close|lseek|read|write|cat|cp|quit]\n");
    printf("input command: ");
    gets(line);
    if (line[0]==0)
      continue;
    pathname[0] = 0;
    parameter[0] = 0;

    sscanf(line, "%s %s %s", cmd, pathname, parameter);
    printf("cmd=%s path=%s param=%s\n", cmd, pathname, parameter);

    if (strcmp(cmd, "ls")==0)
      list_file();

    if (strcmp(cmd, "cd")==0)
      change_dir();

    if (strcmp(cmd, "pwd")==0)
      pwd(running->cwd);

    if (strcmp(cmd, "mkdir")==0)
      make_dir();

    if (strcmp(cmd, "creat")==0)
      creat_file();

    if (strcmp(cmd, "rmdir")==0)
      rmdir();

    if (strcmp(cmd, "link")==0)
      link();

    if (strcmp(cmd, "unlink")==0)
      unlink();

    if (strcmp(cmd, "symlink")==0)
      symlink();

    if (strcmp(cmd, "readlink")==0){
      readlink(line);
      printf("symlink name = %s\n", line);
    }

    //TODO: OPEN : DONE
    if (strcmp(cmd, "open")==0)
      myopen(pathname, parameter);
        
    //TODO: CLOSE : DONE
    if (strcmp(cmd, "close")==0)
      myclose(pathname, parameter);
    
    //TODO: LSEEK
    if (strcmp(cmd, "lseek")==0)
      mylseek(pathname, parameter);

    //TODO: READ : DONE
    if (strcmp(cmd, "read")==0)
    {
      bytesRead = read_file(pathname, parameter);
      printf("Actual number of bytes read: %d\n", bytesRead);
    }
        
    //TODO: WRITE
    if (strcmp(cmd, "write")==0)
    {
      bytesWrote = write_file();
      printf("Number of bytes wrote: %d\n", bytesWrote);
    }

    //TODO: CAT
    if (strcmp(cmd, "cat")==0)
      mycat(pathname, parameter);
        
    //TODO: CP
    if (strcmp(cmd, "cp")==0)
      mycp(pathname, parameter);

    if (strcmp(cmd, "quit")==0)
      quit();
    
    if (strcmp(cmd, "pfd")==0)
      mypfd();
  }
}
 
int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}
