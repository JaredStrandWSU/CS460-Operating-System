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

// vid.c file: implement fbuf for the ARM PL110 LCD display
/**************** Reference: ARM PL110 and DUI02241 ********************
Color LCD base address: 0x10120000 - 0x1012FFFF
00    timing0
04    timing1
08    timing2
0C    timing3
10    upperPanelframeBaseAddressRegister // use only ONE panel
14    lowerPanelFrameBaseAddressRegister // some display allows 2 panels
18    interruptMaskClrregister
1C    controlRegister
20    interruptStatusReg
etc
************************************************************************/
#include "font0"

u8 cursor;
int volatile *fb;
unsigned char *font;
int row, col;

int fbuf_init()
{
  int x; int i;
  /**** for SVGA 800X600 these values are in ARM DUI02241 *********
  *(volatile unsigned int *)(0x1000001c) = 0x2CAC; // 800x600
  *(volatile unsigned int *)(0x10120000) = 0x1313A4C4;
  *(volatile unsigned int *)(0x10120004) = 0x0505F6F7;
  *(volatile unsigned int *)(0x10120008) = 0x071F1800;
  *(volatile unsigned int *)(0x10120010) = (1*1024*1024);
  *(volatile unsigned int *)(0x10120018) = 0x82B;
  ***************************************************************/
  fb = (int *)(1*1024*1024); // enough for 800x600 SVGA
  font = fonts0;              // use fonts0 for char bit patterns 

  /********* for VGA 640x480 ************************/
  *(volatile unsigned int *)(0x1000001c) = 0x2C77;    // LCDCLK SYS_OSCCLK
  *(volatile unsigned int *)(0x10120000) = 0x3F1F3F9C;  // time0
  *(volatile unsigned int *)(0x10120004) = 0x090B61DF;    // time1
  *(volatile unsigned int *)(0x10120008) = 0x067F1800;    // time2
  /*
  *(volatile unsigned int *)(0x10120010) = (1*1024*1024); // panelBaseAddress
  */
  *(volatile unsigned int *)(0x10120010) = (1*1024*1024); // BaseAddress
  *(volatile unsigned int *)(0x10120018) = 0x82B;         //control register
  
  /****** at 0x200-0x3FC are LCDpalletes of 128 words ***************
  unsigned int *inten = (unsigned int *)(0x10120200);
  for (i=0; i<128; i++){
       inten[i]=0x8F008F00;
  }
  ******** yet to figure out HOW TO use these palletes *************/
  //  fb = (int *)(1*1024*1024);  // at 1MB area; enough for 800x600 SVGA


  // for (x = 0; x < (800 * 600); ++x) // for one BAND
  /******** these will show 3 bands of BLUE, GREEN, RED ********* 
  for (x = 0; x < (212*480); ++x)
  fb[x] = 0x00FF0000;  //00BBGGRR
  for (x = 212*480+1; x < (424*480); ++x)
  fb[x] = 0x0000FF00;  //00BBGGRR
  for (x = 424*480+1; x < (640*480); ++x)
  fb[x] = 0x000000FF;  //00BBGGRR
  ************* used only during intial testing ****************/

  // for 640x480 VGA mode display
  for (x=0; x<640*480; x++)
    fb[x] = 0x00000000;    // clean screen; all pixels are BLACK
  cursor = 127; // cursor bit map in font0 at 127
}

int clrpix(int x, int y)
{
  int pix = y*640 + x;
  fb[pix] = 0x00000000;
}

int setpix(int x, int y)
{
  int pix = y*640 + x;
  if (color==RED)
     fb[pix] = 0x000000FF;
  if (color==BLUE)
     fb[pix] = 0x00FF0000;
  if (color==GREEN)
     fb[pix] = 0x0000FF00;
  if (color==CYAN)
     fb[pix] = 0x00FFFF00;
  if (color==PURPLE)
     fb[pix] = 0x00FF00FF;
  if (color==YELLOW)
     fb[pix] = 0x0000FFFF;
  if (color==WHITE)
     fb[pix] = 0x00FFFFFF;
}

int dchar(unsigned char c, int x, int y)
{
  int r, bit;
  unsigned char *caddress, byte;

  caddress = font + c*16;
  //  printf("c=%x %c caddr=%x\n", c, c, caddress);

  for (r=0; r<16; r++){
    byte = *(caddress + r);

    for (bit=0; bit<8; bit++){
      if (byte & (1<<bit))
	  setpix(x+bit, y+r);
    }
  }
}

int undchar(unsigned char c, int x, int y)
{
  int row, bit;
  unsigned char *caddress, byte;

  caddress = font + c*16;
  //  printf("c=%x %c caddr=%x\n", c, c, caddress);

  for (row=0; row<16; row++){
    byte = *(caddress + row);

    for (bit=0; bit<8; bit++){
      if (byte & (1<<bit))
	  clrpix(x+bit, y+row);
    }
  }
}

int dstring(char *s, int x, int y)
{
  while(*s){
    dchar(*s, x, y);
    x+=8;
    s++;
  }
}

int scroll()
{
  int i;
  for (i=0; i<640*480-640*16; i++){
    fb[i] = fb[i+ 640*16];
  } 
}
  
int kpchar(char c, int ro, int co)
{
   int x, y;
   x = co*8;
   y = ro*16;
   //printf("c=%x [%d%d] (%d%d)\n", c, ro,co,x,y);
   dchar(c, x, y);
   
}

int unkpchar(char c, int ro, int co)
{
   int x, y;
   x = co*8;
   y = ro*16;
   //printf("c=%x [%d%d] (%d%d)\n", c, ro,co,x,y);
   undchar(c, x, y);
}

int erasechar()
{ 
  // erase char at (row,col)
  int r, bit, x, y;
  unsigned char *caddress, byte;

  x = col*8;
  y = row*16;
 
  //printf("ERASE: row=%d col=%d x=%d y=%d\n",row,col,x,y);

  for (r=0; r<16; r++){
     for (bit=0; bit<8; bit++){
        clrpix(x+bit, y+r);
    }
  }
} 

int clrcursor()
{
  unkpchar(127, row, col);
}

int putcursor(unsigned char c)
{
  kpchar(c, row, col);
}

int kputc(char c)
{
  clrcursor();
  if (c=='\r'){
    col=0;
    //printf("row=%d col=%d\n", row, col);
    putcursor(cursor);
    return;
  }
  if (c=='\n'){
    row++;
    if (row>=25){
      row = 24;
      scroll();
    }
    //printf("row=%d col=%d\n", row, col);
    putcursor(cursor);
    return;
  }
  if (c=='\b'){
    if (col>0){
      col--;
      erasechar();
      putcursor(cursor);
    }
    return;
  }
  // c is ordinary char
  kpchar(c, row, col);
  col++;
  if (col>=80){
    col = 0;
    row++;
    if (row >= 25){
      row = 24;
      scroll();
    }
  }
  putcursor(cursor); 
  //printf("row=%d col=%d\n", row, col);
}

int kprints(char *s)
{
  while(*s){
    kputc(*s);
    s++;
  }
}

int krpx(u32 x)
{
  char c;
  if (x==0) 
     return;
  c = tab[x % 16];
  krpx(x / 16);
  kputc(c);
}

int kprintx(u32 x)
{
  kputc('0'); kputc('x');
  if (x==0)
    kputc('0');
  else
    krpx(x);
  kputc(' ');
}

int krpu(u32 x)
{
  char c;
  if (x==0) 
     return;
  c = tab[x % 10];
  krpu(x / 10);
  kputc(c);
}

int kprintu(u32 x)
{
  if (x==0){
    kputc('0');
  }
  else
    krpu(x);
  kputc(' ');
}

int kprinti(int x)
{
  if (x<0){
    kputc(' ');
    kputc('-');
    x = -x;
  }
  kprintu((u32)x);
}

int kprintf(char *fmt,...)
{
  int *ip;
  char *cp;
  cp = fmt;
  ip = (int *)&fmt + 1;

  while(*cp){
    if (*cp != '%'){
      kputc(*cp);
      if (*cp=='\n')
	kputc('\r');
      cp++;
      continue;
    }
    cp++;
    switch(*cp){
    case 'c': kputc((char)*ip);      break;
    case 's': kprints((char *)*ip);  break;
    case 'd': kprinti(*ip);          break;
    case 'u': kprintu(*ip);          break;
    case 'x': kprintx(*ip);          break;
    }
    cp++; ip++;
  }
}

int stestring(char *s)
{
  char c;
  while((c=kgetc()) != '\r'){
    *s = c;
    s++;
  }
  *s = 0;
}
