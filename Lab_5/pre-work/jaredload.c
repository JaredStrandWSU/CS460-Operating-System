int bsector;
int bmap, imap, iblk, blk, offset;
char buf[1024], buf1[1024], buf2[1024];


int search(INODE *ip, char *name)
{
  int i, j; 
  char c, *cp;
  //char *newname;
  DIR  *dp; 
  /*
  if (name[0] == '/')
  {
    printf("Path is Absolute, look in /bin directory.\n");
    //path is absolute, cut off /bin/
    j = 5;
    i = 0;
    while(name[j] != '\0')
    {
      newname[i] = name[j];
      j++; i++;
    }
    newname[i+1] = '\0';
    printf("newname string: %s\n", newname);
  }
  */
 printf("search for %s: ", name);
  for (i=0; i<12; i++){
    if ( ip->i_block[i] ){
      //printf("i_block[%d] = %d\n", i, ip->i_block[i]);
      getblock(ip->i_block[i], buf2);
      dp = (DIR *)buf2;
      cp = buf2;
      while (cp < &buf2[1024]){
        c = dp->name[dp->name_len];  // save last byte

        dp->name[dp->name_len] = 0;
        //printf("About to compare two string values.\n");
        //printf("dp->name: %s\n", dp->name);
        //printf("name: %s\n"); 
        //printf("searching for name: %s\ncomparing against dp->name: %s\n", name, dp->name); 
        if ( strcmp(dp->name, name) == 0 ){
          printf("found %s\n", name); 
          return(dp->inode);
        }

        dp->name[dp->name_len] = c; // restore that last byte
        cp += dp->rec_len;
        dp = (DIR *)cp;
      }
    }
  }
  printf("search failed\n");
}

load(char *filename, PROC *p)
{ 
  int    i, me, blk, iblk, count;
  char   *cp, *name[2], *addr;
  u32    *up;
  GD     *gp;
  INODE  *ip;
  DIR    *dp;

  name[0] = "bin";
  //added for fork
  if (filename[0] != 'u')
  {
    name[1] = "u1";
  }
  else
  {
    name[1] = filename;
  }
    
  //name[1] = filename;
  printf("value of filename: %s\n", filename);
  addr = (char *)(0x800000 + (p->pid - 1)*0x100000);
  printf("loading %s: ", filename);
  
  /* read blk#2 to get group descriptor 0 */
  getblock(2, buf1);
  gp = (GD *)buf1;
  iblk = (u16)gp->bg_inode_table;

  printf("iblk=%d ", iblk);
  
  getblock(iblk, buf1);       // read first inode block block
  ip = (INODE *)buf1 + 1;   // ip->root inode #2

  /* search for system name */
  for (i=0; i<2; i++){
      me = search(ip, name[i]) - 1;
      if (me < 0) 
	      return 0;
      getblock(iblk+(me/8), buf1);    // read block inode of me
      ip = (INODE *)buf1 + (me % 8);
  }

  /* read indirect block into b2 */
  if (ip->i_block[12])         // only if has indirect blocks 
     getblock(ip->i_block[12], buf2);

  count = 0;

  for (i=0; i<12; i++){
    if (ip->i_block[i] == 0)
      break;
      //printf("location=%x count=%d\n", location, count);
      getblock(ip->i_block[i], addr);
      kputc('*');
      addr += 1024;
      count += 1024;
  }

  if (ip->i_block[12]){ // only if file has indirect blocks
     up = (u32 *)buf2;      
     while(*up){
       getblock(*up, addr); 
       kputc('.');
       addr += 1024;
       up++; count += 1024;
     }
  }
  //  printf("\n");
  printf(" %d bytes loaded\n", count); 
}  

