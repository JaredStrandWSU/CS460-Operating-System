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

#include "defines.h"
#include "vid.c"
#include "uart.c"
int color;
char *tab = "0123456789ABCDEF";

UART uart[4];

extern char _binary_wsu_bmp_start;

int color;
UART *up;

int main()
{
   char c, *p;
   int mode;
   uart_init();
   up = &uart[0];

   mode = 0;
   fbuf_init(mode);

   p = &_binary_wsu_bmp_start;
   show_bmp(p, 0, 0); 

   while(1){
     fuprintf(up, "enter a key from this UART : ");
     ugetc(up);
     p = &_binary_wsu_bmp_start;
     show_bmp(p, 0, 0);
   }
   while(1);   // loop here  
}
