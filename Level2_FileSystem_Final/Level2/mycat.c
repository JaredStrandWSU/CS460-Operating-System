void mycat()
{
/*
    cat filename:

   char mybuf[1024], dummy = 0;  // a null char at end of mybuf[ ]
   int n;

    int fd = open filename for READ;
    while( n = read(fd, mybuf[1024], 1024)){
       mybuf[n] = 0;             // as a null terminated string
       // printf("%s", mybuf);   <=== THIS works under Linux but not good
       spit out chars from mybuf[ ] but handle \n properly;
   } 
    close(fd);
*/
    return;
}