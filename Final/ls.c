
#include "ucode.c"

#define S_ISDIR(m) ((m >> 9) == 040) //bit shift
#define S_ISREG(m) ((m >> 12) == 010)
#define S_ISLNK(m) ((m >> 12) == 012)

char buffer[1024];
char cwd[256];
char temp[256];

void ls_file(char * filename)
{
	const char * t1 = "xwrxwrxwr-------";
	const char * t2 = "----------------";
	STAT s;
	stat(filename, &s);
	if(S_ISREG(s.st_mode))
		mputc('-');
	else if(S_ISDIR(s.st_mode))
		mputc('d');
	else
		mputc('l');
	for(int i = 8; i >= 0; i--)
		mputc((s.st_mode & (1 << i)) ? t1[i] : t2[i]);
	printf(" %d %d %d %d ", s.st_nlink, s.st_gid, s.st_uid, s.st_size);
	prints(filename);
	if(S_ISLNK(s.st_mode))
	{
		prints(" -> ");
		char link[256];
		readlink(filename, link);
		prints(link);
	}
	prints("\n");
}

void ls_dir(char * filename)
{
	int dir = open(filename, O_RDONLY);
	read(dir, buffer, 1024);
	char * cp = buffer;
	DIR * dp = (DIR*) buffer;
	while(cp < 1024 + buffer)
	{
		mystrncpy(temp, dp->name, dp->name_len);
		temp[dp->name_len] = '\0';
		ls_file(temp);
		cp += dp->rec_len;
		dp = (DIR*)cp;	
	}
}

int main(int argc, char * argv[])
{
	getcwd(cwd);
	if(argc < 2)
	{   

		ls_dir(cwd);
	}

	else
	{
		STAT s;
		stat(argv[1], &s);
		if(S_ISDIR(s.st_mode))
			ls_dir(argv[1]);
		else
			ls_file(argv[1]);
	}

	return 0;
}