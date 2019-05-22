/************ uio.c file: must implement printf() in U space **********/
#define printf uprintf
char *tab = "0123456789ABCDEF";

int uputc(char);

int ugetline(char *line)
{
  char c, *cp = line;
  while( (c = ugetc()) != '\r'){
    //uputc('u'); uputc(c);
    *cp = c;
    cp++;
  }
  *cp = 0;
}
 
int uprints(char *s)
{
  while(*s){
    uputc(*s);
    s++;
  }
}

int urpx(u32 x)
{
  char c;
  if (x==0) 
     return;
  c = tab[x % 16];
  urpx(x / 16);
  uputc(c);
}

int uprintx(u32 x)
{
  uputc('0'); uputc('x');
  if (x==0)
    uputc('0');
  else
    urpx(x);
  uputc(' ');
}

int urpu(u32 x)
{
  char c;
  if (x==0) 
     return;
  c = tab[x % 10];
  urpu(x / 10);
  uputc(c);
}

int uprintu(u32 x)
{
  if (x==0){
    uputc('0');
  }
  else
    urpu(x);
  uputc(' ');
}

int uprinti(int x)
{
  if (x<0){
    uputc(' ');
    uputc('-');
    x = -x;
  }
  uprintu((u32)x);
}

int uprintf(char *fmt,...)
{
  int *ip;
  char *cp;
  cp = fmt;
  ip = (int *)&fmt + 1;

  while(*cp){
    if (*cp != '%'){
      uputc(*cp);
      if (*cp=='\n')
	uputc('\r');
      cp++;
      continue;
    }
    cp++;
    switch(*cp){
    case 'c': uputc((char)*ip);      break;
    case 's': uprints((char *)*ip);  break;
    case 'd': uprinti(*ip);          break;
    case 'u': uprintu((u32)*ip);          break;
    case 'x': uprintx((u32)*ip);          break;
    }
    cp++; ip++;
  }
}
