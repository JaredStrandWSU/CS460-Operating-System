#include "ucode.c"
int main(int argc, char *argv[ ])
{
    //do grep
    prints("Doing GREP\n");

    char buffer[1024];
    int fd = 0, n = 0, i = 0;

	if(argc < 2)
	{
		prints("not enough parameters\n");
		return 0;
	}
	
	if(argc > 2)
	{
        //we have at least 3 things to look fo
        //open the file we want to search
		fd = open(argv[2], O_RDONLY); 
        if (fd < 0)
        {
            //bad filename
            prints("bad filename or argument\n");
            return(0);
        }
        //find the pattern in the file, getline,
        while(1)
        {
            n = read(fd, buffer, 1024);
            //tokenize file by '\n'

            //search all lines with strcmp and print if match
            for (i=0; i < n; i++)
            {
                //finish
                prints("grep tokenize and print under construction.\n");
                break;
            }
        }
        
        close(fd);
	}
return 0;
}