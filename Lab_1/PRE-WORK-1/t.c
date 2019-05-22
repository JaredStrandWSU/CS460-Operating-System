/*********  t.c file *********************/

int prints(char *s)
{
	
	// write YOUR code
	while(1)
	{
		//if( *s == '\r')
		//{ break; }
		if(*s == '\0')
		{
			break;
		}
		else{
			putc(*s);
			s = s + 1;
		}
	}
	return 0;
	
/*
		while(*s)
	{
		putc(*s++);
	}
	*/
}

int gets(char *s)
{
	
	// write YOUR code
	short i = 0;
	char input = 0;

	while(1)
	{
		input = getc();
		if(input == '\r')
		{
			break;
		}
		else
		{
			//s[i] = input;
			//putc(s[i]);
			//i = i + 1;
			*s = input;
			putc(*s);
			s = s + 1;
			i = i + 1;
		}
	}
	s[i] = '\0';

	//Add null character at end of string
	return 0;
	
	/*
  // write YOUR code
	while ((*s=getc()) != 'r')
		putc(*s++);
	*s = 0;
	*/
}

char ans[64];

main()
{
  while(1){
    prints("Jared's : What's your name? ");
    gets(ans);  prints("\n");

    if (ans[0]==0){
      prints("return to assembly and hang\n\r");
			getc();
			return;
    }
    prints("Welcome "); prints(ans); prints("\n\r");
  }
}
  
