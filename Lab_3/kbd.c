
#include "keymap2"

/********************************************************************************
//0    1    2    3    4    5    6    7     8    9    A    B    C    D    E    F
char ltab[] = {
  0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,  'q', '1',  0,    0,   0,  'z', 's', 'a', 'w', '2',  0,
  0,  'c', 'x', 'd', 'e', '4', '3',  0,    0,  ' ', 'v', 'f', 't', 'r', '5',  0,
  0,  'n', 'b', 'h', 'g', 'y', '6',  0,    0,   0,  'm', 'j', 'u', '7', '8',  0,
  0,  ',', 'k', 'i', 'o', '0', '9',  0,    0,  '.', '/', 'l', ';', 'p', '-',  0,
  0,   0,  '\'', 0,  '[', '=',  0,   0,    0,   0, '\r', ']',  0, '\\',  0,   0,
  0,   0,   0,   0,   0,   0,  '\b', 0,    0,   0,   0,   0,   0,   0,   0,   0
};

char utab[] = {
  0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,  'Q', '!',  0,    0,   0,  'Z', 'S', 'A', 'W', '@',  0,
  0,  'C', 'X', 'D', 'E', '$', '#',  0,    0,  ' ', 'V', 'F', 'T', 'R', '%',  0,
  0,  'N', 'B', 'H', 'G', 'Y', '^',  0,    0,   0,  'M', 'J', 'U', '&', '*',  0,
  0,  '<', 'K', 'I', 'O', ')', '(',  0,    0,  '>', '?', 'L', ':', 'P', '_',  0,
  0,   0,  '"',  0,  '{', '+',  0,   0,    0,   0,  '\r','}',  0,  '|',  0,   0,
  0,   0,   0,   0,   0,   0,  '\b', 0,    0,   0,   0,   0,   0,   0,   0,   0
};
**********************************************************************************/
// KBD registers from base address
#define KCNTL 0x00    
#define KSTAT 0x04
#define KDATA 0x08
#define KCLK  0x0C
#define KISTA 0x10

typedef struct kbd{
  char *base;
  char buf[128];
  int head, tail, data, room;
}KBD;

KBD kbd;

int kputc(char);

int shifted = 0; 
int release = 0;
int control = 0;
int i = 0;

int kbd_init()
{
  KBD *kp = &kbd;
  kp->base = (char *)0x10006000;
  *(kp->base + KCNTL) = 0x11;    // bit4=Enable   bit0=INT on
  *(kp->base + KCLK)  = 8;       // KBD internal clock setting 
  kp->head = kp->tail = 0;
  kp->data = 0; kp->room = 128;

  release = 0;  // key release flag

  shifted = 0;  // shift key down flag
  control = 0;  // control key down flag
}

// kbd_handler() for scan code set 2

/*******************************************
  key press    =>        scanCode
  key release  =>   0xF0 scanCode
Example: 
 press   'a'   =>          0x1C
 release 'a'   =>   0xF0   0x1C
******************************************/

void kbd_handler()
{
  u8 scode, c;

  KBD *kp = &kbd;
  color = YELLOW;
  scode = *(kp->base + KDATA);  // get scan code from KDATA reg => clear IRQ

  //printf("\nscanCode = %x\n", scode);

  if (scode == 0xF0){       // key release 
     release = 1;           // set flag
     printf("RELEASE = 1\n");
     return;
  }

  if ( (scode == 0x12 || scode == 0x59) && (shifted == 1) ) {       // shift pressed
    shifted = 0;
    release = 0;
    printf("SHIFTED = 0\n");
    printf("RELEASE = 0\n");
    return;
  }

  if ( (scode == 0x14) && (control == 1) ) {       // Control pressed
    control = 0;
    release = 0;
    printf("SHIFTED = 0\n");
    printf("RELEASE = 0\n");
    return;
  }

  if (scode == 0x12 || scode == 0x59){
    shifted = 1;
    printf("SHIFTED = 1\n");
    return;
  }

  if (scode == 0x14 ){
    control = 1;
    printf("CONTROL = 1\n");
    return;
  }

  if (release && scode){    // next scan code following key release
     release = 0;           // clear flag 
     printf("RELEASE = 0\n");
     return;
  }

  if (control && (scode == 0x21)){ //Ctrl + C
    printf("Ctrl + C ::: ACK I KILLED MYSELF!\n");
  }

  if (control && (scode == 0x23)){ //Ctrl + D
    printf("Ctrl + D ::: KILL THAT FILE!\n");
    c = 0x04; //kill file input command
  }

  if (control && (scode == 0x1c)){ // EASTER EGG Ctrl + A
    for(i=0;i<1000;i++) {printf("I LOVE ANNA THE MOST! ! !\n");}
  }

  if (shifted && scode){
    c = utab[scode];
  }
  else{
    c = ltab[scode];
  }
/*
  if (!shifted){      
     c = ltab[scode];
  }
  else {              // ONLY IF YOU can catch LEFT or RIGHT shift key
     c = utab[scode];
  }
*/
  printf("c=%x %c\n", c, c);
  
  kp->buf[kp->head++] = c;
  kp->head %= 128;
  kp->data++; kp->room--;
}

int kgetc()
{
  char c;
  KBD *kp = &kbd;
  //printf("%d in kgetc\n", running->pid); 

  while(kp->data == 0);

  lock();
    c = kp->buf[kp->tail++];
    kp->tail %= 128;
    kp->data--; kp->room++;
  unlock();
  return c;
}

int kgets(char s[ ])
{
  char c;
  while( (c = kgetc()) != '\r'){
    if (c=='\b'){
      s--;
      continue;
    }
    *s++ = c;
  }
  *s = 0;
  return strlen(s);
}