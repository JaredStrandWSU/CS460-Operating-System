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

#define bcopy  kbcopy
#define memcpy kmemcpy
#define strcpy kstrcpy
#define strlen kstrlen

int kbcopy(const void *src, void *dest, unsigned int n)
{
   const char *s = (const char *)src;
   char *d = (char *)dest;
   if (s <= d){
      for (; n>0; --n)
         d[n-1] = s[n-1];
   } 
   else{
     for (; n>0; --n)
	*d++ = *s++;
   }
}

char *kmemcpy(void *dest, const void *src, unsigned int n)
{
      kbcopy(src, dest, n);
      return dest;
}

int kstrlen(char *s)
{
  int i = 0;
  while(*s){
    i++; s++;
  }
  return i;
}

int kstrcmp(char *s1, char *s2)
{
  while((*s1 && *s2) && (*s1==*s2)){
      s1++; s2++;
  }
  if ((*s1==0) && (*s2==0))
    return 0;
  return(*s1 - *s2);

}

int kstrcpy(char *dest, char *src)
{
  while(*src){
    *dest++ = *src++;
  }
  *dest = 0;
}
