#include "ucode.c"

//cat function

int main(int argc, char *argv[])
{
    int fd, n, i;
    char buf[1024];
    char newline = '\r'; //used to replace \newlines in files

    prints("\n### Jared's cat is fat, doesn't do much... ###\n\n");
    for (i=0; i < argc; i++)
    {
        printf("argv[%d]: %s\n", i, argv[i]);
    }

    if (argc < 2) //cat wihtout a filename
    {
        prints("Doing CAT from STDIN!\n");
        while (1)
        {
            prints("input: ");
            gets(buf);
            prints("output: ");
            write(1, buf, strlen((const char *)buf)); //write to stdout
            prints("\n");
            for(i=0; i < 1024; i ++)
                buf[i] = 0;
        }
    }
    else if (argc >= 2) //if passing a filename
    {
        prints("Doing CAT from FILE!\n");
        fd = open(argv[1], O_RDONLY); //open filename for read mode

        if (fd < 0) //if not exists error
            exit(0);
        while ((n = read(fd, buf, 1024)) > 0) //read from file until no left
        {
            for (i = 0; i < n; i++)
            {   //write the contents to stdout
                write(1, &(buf[i]), 1); //write 1 byte at a time
                if (buf[i] == '\n')
                    write(2, &newline, 1); //write return char as stdin
            }

            //CLEANSE THE BUFFER
            for (i = 0; i < 1024; i++)
                buf[i] = 0;
        }
    }
    //done catting
    close(fd);
    exit(0);
}