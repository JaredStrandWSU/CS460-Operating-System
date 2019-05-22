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
  //Assign memory addresses for your UART Devices
  for (i=0; i<4; i++){
    up = &uart[i];
    //up->base = (char *)(0x101F1000 + i*0x1000);
    up->base = (char *)(0x10009000 + i*0x1000);
    up->n = i;
  }
  //uart[3].base = (char *)(0x10009000); // uart3 at 0x10009000
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
/*
// fuprintf: prints a passed string!
void fuprintf(UART *up, char *fmt, . . .)
{

}
int myprintf(char *fmt, ...)
{
	if (strcmp(fmt, "") != 0)
	{
		int i = 0, *ebp = getebp(), *ip = ebp + 3;
		for (i = 0; fmt[i] != '\0'; i++)
		{
			if (fmt[i] == '%')
			{
				i++;
				if (fmt[i] == 'c')
					printc(*ip);
				else if (fmt[i] == 'd')
					printd(*ip);
				else if (fmt[i] == 'x')
					printx(*ip);
				else if (fmt[i] == 's')
					prints(*ip);
				else if (fmt[i] == '%')
					putchar('%');
				else
					return 0;
				ip++;
			}
			else if (fmt[i] == '/')
			{
				i++;
				if (fmt[i] == 'n')
				{
					putchar('\n');
					putchar('\r');
				}
				i++;
			}
			else
				putchar(fmt[i]);
		}
	}
	return 1;
}

//// Printing Helper Functions
// print an integer
void printd(int x)
{
	if (x == 0)
		putchar('0');
	else if (x < 0)
	{
		putchar('-');
		rpd(-x);
	}
	else
		rpd(x);
	putchar(' ');
}

// print a hex value
void printx(unsigned long x)
{
	if (x == 0)
		putchar('0');
	else
		rpx(x);
	putchar(' ');
}

// print a character
void printc(char c)
{
	putchar(c);
}

// print a string
void prints(char *s)
{
	int i = 0;
	for (i = 0; s[i] != '\0'; i++)
		putchar(s[i]);
	putchar(' ');
}
// prints an unsigned long (non-negative number)
void printu(unsigned long x)
{
	if (x == 0)
		putchar('0');
	else
		rpu(x);
	putchar(' ');
}

//// Printing Recursive Helper Functions
// recursive helper for printu
void rpd(int x)
{
	char c;
	if (x)
	{
		c = table[x % BASE];
		rpu(x / BASE);
		putchar(c);
	}
}

// recursive helper for printu
void rpx(unsigned long x)
{
	char c;
	if (x)
	{
		c = table[x % HEX];
		rpx(x / HEX);
		putchar(c);
	}
}

// recursive helper for printu
void rpu(unsigned long x)
{
	char c;
	if (x)
	{
		c = table[x % BASE];
		rpu(x / BASE);
		putchar(c);
	}
}
*/