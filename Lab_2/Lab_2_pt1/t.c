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

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

/*
UART0 base address: 0x101f1000;
UART1 base address: 0x101f2000;
UART2 base address: 0x101f3000;
UART3 base address: 0x10009000;

// flag register at 0x18
//  7    6    5    4    3    2   1   0
// TXFE RXFF TXFF RXFE BUSY
// TX FULL : 0x20
// TX empty: 0x80
// RX FULL : 0x40
// RX empty: 0x10
// BUSY=1 :  0x08
*/

int N;
int v[] = {1,2,3,4,5,6,7,8,9,10};
int sum;

//char *tab = "0123456789ABCDEF";

#include "string.c"
#include "uart.c"

UART *up;

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

int main()
{
  int i;
  int size = sizeof(int);
  char string[32]; 
  char line[128]; 

  N = 10;

  uart_init();

// 1.
  // up = &uart[0];
  //uprints(up, "Enter lines from UART terminal, enter quit to exit\n\r");
// 3. 
/*
  //print from each 
  while(1){
    for (i=0; i<4; i++){
      up = &uart[i];
      uprints(up, "enter a line from this UART : ");
      ugets(up, string);
      if (strcmp(string, "quit")==0)
        break;
      uprints(up, "    ECHO : "); uprints(up, string); uprints(up, "\n\r");
    }
    if (strcmp(string, "quit")==0)
      break;
  }
*/
 // 4.
  for (i=0; i<4; i++){
    up = &uart[i];
    fuprintf(up, "%s", "enter a line from this UART : ");
    ugets(up, string);
    fuprintf(up, "    ECHO %s\n", string);
  }
  
  up = &uart[0];
  uprints(up, "Enter lines from UART terminal, enter quit to exit\n\r");

  while(1){
    ugets(up, string);
    uprints(up, "    ");
    if (strcmp(string, "quit")==0)
       break;
    uprints(up, string);  uprints(up, "\n\r");
  }

  uprints(up, "Compute sum of array\n\r");
  sum = 0;
  for (i=0; i<N; i++)
    sum += v[i];
  fuprintf(up, "sum = %d\n", sum);
  uputc(up, (sum/10)+'0'); 
  uputc(up, (sum%10)+'0');
  uprints(up, "\n\rEND OF RUN\n\r");

}