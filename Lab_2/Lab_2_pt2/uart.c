/********************************************************************
Copyright 2010-2017 K.C. Wang, <kwang@eecs.wsu.edu>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/
#define DR   0x00
#define FR   0x18

#define RXFE 0x10
#define TXFF 0x20

typedef struct uart{
  char *base;
  int n;
}UART;

UART uart[4];

int uart_init()
{
  int i; UART *up;

  for (i=0; i<4; i++){
    up = &uart[i];
    up->base = (char *)(0x101F1000 + i*0x1000);
    up->n = i;
  }
  uart[3].base = (char *)(0x10009000); // uart3 at 0x10009000
}

int ugetc(UART *up)
{
  while (*(up->base + FR) & RXFE);
  return *(up->base + DR);
}

int uputc(UART *up, char c)
{
  while(*(up->base + FR) & TXFF);
  *(up->base + DR) = c;
}

int ugets(UART *up, char *s)
{
  while ((*s = (char)ugetc(up)) != '\r'){
    uputc(up, *s);
    s++;
  }
 *s = 0;
}

int uprints(UART *up, char *s)
{
  while(*s)
    uputc(up, *s++);
}

/** WRITE YOUR uprintf(UART *up, char *fmt, . . .) for formatted print **/
int urpx(UART *up, int x) //recursively print hex from 360
{
  char c;
  char *tab = "0123456789ABCDEF";//table for ints and hex values
  if (x) {
    c = tab[x % 16];
    urpx(up, x/16);
  }
  uputc(up, c);
}

int uprintx(UART *up, int x)
{
  uputc(up, '0'); uputc(up, 'x'); //put '0x'
  if (x == 0)
    uputc(up, '0');
  else
    urpx(up, x);
  uputc(up, ' ');
}

int urpu(UART *up, int x) //recursive print unsigned from 360 using table
{
  char c;
  char *tab = "0123456789ABCDEF";//table for ints and hex values
  if(x)
  {
    c = tab[x % 10];
    urpu(up, x/10);
  }
  uputc(up, c);
}

int uprintu(UART *up, int x)
{
  if (x == 0) //handle zero case
    uputc(up, '0');
  else
    urpu(up, x);
  uputc(up, ' '); //print space
}

int uprinti(UART *up, int x)
{
  if (x < 0)    //check if negative, if so, print a negativ sign, then send unsigned
  {
    uputc(up, '-');
    x = -x;
  }
  uprintu(up, x); //send to unsigned function
}

void fuprintf(UART *up, char *fmt, ... ){
  int *ip;  //points at the params
  char *cp; //points at the input string with the %'s
  cp = fmt; //set to temp
  ip = (int *)&fmt + 1; //set to point at the params

  while(*cp) //while we have an input string
  {
    if (*cp != '%') //check if the current char is the %
    {
      uputc(up, *cp); //if not a % function just print to uart screen
      if(*cp == '\n') //check if a newline char 
        uputc(up, '\r'); //if it's newline push a \r char
      cp++;  //incrment the string pointer
      continue; //reset loop
    }
    cp++;   //scroll past the %
    switch (*cp) {  //choose which printf command to use
      case 'c': uputc(up, (char)*ip);   //pass the char pointer
                break;
      case 's': uprints(up, (char *)*ip); //pass the pointer to the first char in string
                break;
      case 'd': uprinti(up, *ip); //pass dereferenced integer
                break;
      case 'u': uprintu(up, *ip); //pass dereferenced unsigned integer
                break;
      case 'x': uprintx(up, *ip); //pass dereferenced unsigned hexadecimal integer
                break;
    }
    cp++; ip++; //increment pointers
  }
}