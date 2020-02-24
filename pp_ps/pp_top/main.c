#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define clear() printf("\033[H\033[J")
#define gotoxy(x, y) printf("\033[%d;%dH", (x), (y))

struct process
{
    long int pid;
    char command[100];
    char state;
    float cpu;     //% used
    float mem;     //% used
    u_long VSZ;    //(virtual memory size
    u_long RSS;    //resident set size  in bytes
    u_int CPU_exc; //last executed

    long utime;
    long stime;
    long starttime;
};

int CPUcomparator(const void *p, const void *q)
{
    float l = ((struct process *)p)->cpu;
    float r = ((struct process *)q)->cpu;
    if (l < r)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int MEMcomparator(const void *p, const void *q)
{
    float l = ((struct process *)p)->mem;
    float r = ((struct process *)q)->mem;
    if (l < r)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int PIDcomparator(const void *p, const void *q)
{
    long int l = ((struct process *)p)->pid;
    long int r = ((struct process *)q)->pid;
    if (l > r)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int COMcomparator(const void *p, const void *q)
{
    char *l = ((struct process *)p)->command;
    char *r = ((struct process *)q)->command;
    int out = strcmp(l, r);
    //printf("%s comp %s is %d\n",l,r,out);
    return out;
}

int sizeFile(char *newpath)
{
    int count = 0;
    FILE *fp = fopen(newpath, "r");
    char ch;
    while ((ch = fgetc(fp)) != EOF)
    {
        count++;
        //printf("%c", ch);
    }

    fclose(fp);
    return count;
}

float getUptime()
{

    float uptime;
    int tsize = sizeFile("/proc/uptime");
    char tdata[tsize];
    FILE *fp1 = fopen("/proc/uptime", "r");
    if (fp1 == NULL)
    {
        printf("Error opening uptime file");
    }
    else
    {
        if (fgets(tdata, tsize, fp1) != NULL)
        {
            char input[tsize];
            for (int i = 0; i < tsize; i++)
            {
                if (tdata[i] == ' ')
                {
                    break;
                }
                else
                {
                    input[i] = tdata[i];
                }
            }
            uptime = strtod(input, NULL);
            printf("the uptime is %f\n", uptime);
        }
    }
    fclose(fp1);
    return uptime;
}



int main(int argc, char *argv[])
{

    //deal with args
    if (argc < 2)
    {
        printf("please input one with following with the program: -cpu | -mem | -pid | -com \n");
        return -1;
    }
    int sorttype = -1;
    if (strcmp(argv[1], "-cpu") == 0)
    {
        sorttype = 0;
    }
    else if (strcmp(argv[1], "-mem") == 0)
    {
        sorttype = 1;
    }
    else if (strcmp(argv[1], "-pid") == 0)
    {
        sorttype = 2;
    }
    else if (strcmp(argv[1], "-com") == 0)
    {
        sorttype = 3;
    }
    else
    {
        printf("please input one with following with the program: -cpu | -mem | -pid | -com \n");
        printf("not %s\n", argv[1]);
        return -1;
    }

    //get the static infomation
    struct winsize wsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize);
    printf("The terminal has %d rows and %d columns\n", wsize.ws_row, wsize.ws_col);

    long phys_pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    long phys_mem_size = phys_pages * page_size;
    long total_mem =0;
    printf("page sizesize: %ld \n", page_size);
    printf("phys_mem_size: %ld \n", phys_mem_size);

    //the formatting  the terminal 
        //set amount of space for the PID and state
        int statl = 6;
        int pidl = 10;

        int space = wsize.ws_col;
        space -=(statl + pidl+1);
        //calc the rest based on the space advalible 
        int cpuel = space/6;
        int coml = space/6;
        int cpul = space/6;
        int meml = space/6;
        int vszl = space/6;
        int rssl = space/6;

    //get the information that changes

    while(1){
    float uptime = getUptime();

    //header info
    long int total_process =0;
    long int running_processs =0;
    float total_cpu =0;
    //phys_mem_size

    DIR *dp;
    struct dirent *ep;
    long int pid;

    char basepath[] = "/proc";
    dp = opendir(basepath);

    int num_proc = 999;
    struct process list[num_proc];
    int cursor = 0;
    if (dp != NULL)
    {
        while (ep = readdir(dp))
        {
            pid = strtol(ep->d_name, NULL, 10);
            if ((ep->d_type == DT_DIR) && (pid > 0))
            {
                //printf("directory name: %s \n", ep->d_name);
                // constructs the new path
                char newpath[1000];
                strcpy(newpath, basepath);
                strcat(newpath, "/");
                strcat(newpath, ep->d_name);
                strcat(newpath, "/stat");
                total_process++;
                //printf("reading stats from %s \n", newpath);
                //printf("\n");

                //printf("the file is %d chars\n", sizeFile(newpath));

                int size = sizeFile(newpath);
                char data[size];

                FILE *fp = fopen(newpath, "r");
                if (fp == NULL)
                {
                    printf("Error opening file");
                }
                else
                {
                    //read in the data in the file
                    if (fgets(data, size, fp) != NULL)
                    {
                        //printf("inported: %s \n", data);

                        //parsing the data
                        char input[100] = "";
                        int num_spaces = 0;
                        int index = 0;
                        int done = 0;
                        for (int i = 0; i < size + 1; i++)
                        {
                            if (data[i] == ' ')
                            {
                                //printf("%s\n",input);

                                index = 0;

                                switch (num_spaces)
                                {
                                case 0:
                                    //long int pid;
                                    //printf("pid %s\n", input);
                                    list[cursor].pid = strtol(input, NULL, 10);
                                    //printf("pid %ld\n", list[cursor].pid);
                                    break;
                                case 1:
                                    // char command[30];
                                    // printf("command = %s\n", input);
                                    //list[cursor].command = input;
                                    input[0] = ' ';
                                    input[strlen(input) - 1] = '\0';
                                    strcpy(list[cursor].command, input);
                                    //printf("command = %s\n", list[cursor].command);
                                    break;
                                case 2:
                                    //char state;
                                    // printf("state = %s\n", input);
                                    list[cursor].state = input[0];
                                    if(list[cursor].state == 'R'){
                                        running_processs++;
                                    }
                                    //printf("state = %c\n", list[cursor].state);
                                    break;
                                case 13:
                                    //utime
                                    //  printf("utime = %s\n", input);
                                    list[cursor].utime = strtol(input, NULL, 10);
                                    //printf("utime = %ld\n", list[cursor].utime);
                                    break;
                                case 14:
                                    //stime
                                    // printf("stime = %s\n", input);
                                    list[cursor].utime = strtol(input, NULL, 10);
                                    //  printf("stime = %ld\n", list[cursor].utime);
                                    break;
                                case 21:
                                    //  printf("start time = %s\n", input);
                                    list[cursor].starttime = strtol(input, NULL, 10);
                                    // printf("start time = %ld\n", list[cursor].starttime);
                                    break;
                                case 22:
                                    //u_long VSZ;    //(virtual memory size
                                    //  printf("vsz = %s\n", input);
                                    list[cursor].VSZ = strtol(input, NULL, 10);
                                    //  printf("vsz = %ld\n", list[cursor].VSZ);
                                    break;
                                case 23:
                                    //u_long RSS;    //resident set size  in bytes
                                    //  printf("rss = %s\n", input);
                                    list[cursor].RSS = strtol(input, NULL, 10);
                                    // printf("rss = %ld\n", list[cursor].RSS);
                                    break;
                                case 38:
                                    //u_int CPU_exc; //last executed
                                    //  printf("cpu was last called %s\n", input);
                                    list[cursor].CPU_exc = strtol(input, NULL, 10);
                                    //  printf("cpu was last called %ld\n", list[cursor].CPU_exc);
                                    break;
                                default:
                                    if (num_spaces > 38)
                                    {
                                        done = 1;
                                        //calculate the memory
                                        list[cursor].mem = (float)(list[cursor].RSS * page_size * 100) / phys_mem_size;
                                        total_mem +=list[cursor].mem; 
                                        // list[cursor].mem =0;
                                        //calculate the cpu
                                        long process_time = (list[cursor].utime / sysconf(_SC_CLK_TCK)) + (list[cursor].stime / sysconf(_SC_CLK_TCK));
                                        long real_time = uptime - (list[cursor].starttime / sysconf(_SC_CLK_TCK));
                                        if (real_time == 0)
                                        {
                                            list[cursor].cpu = 0;
                                        }
                                        else
                                        {
                                            list[cursor].cpu = (float)(process_time * 100) / real_time;
                                        }
                                        total_cpu += list[cursor].cpu;
                                    }
                                    break;
                                }
                                num_spaces++;
                                memset(input, 0, sizeof 100);
                                if (done == 1)
                                {
                                    break;
                                }
                            }
                            else
                            {
                                input[index] = data[i];
                                index++;
                            }
                        }

                        cursor++;

                        //printf("\n");
                    }
                }
                fclose(fp);
            }
        }
        closedir(dp);

        //sort all of the data
        if (sorttype == 2)
        {
            qsort(list, cursor, sizeof(struct process), PIDcomparator);
        }
        else if (sorttype == 0)
        {
            qsort(list, cursor, sizeof(struct process), CPUcomparator);
        }
        else if (sorttype == 1)
        {
            qsort(list, cursor, sizeof(struct process), MEMcomparator);
        }
        else if (sorttype == 3)
        {
            qsort(list, cursor, sizeof(struct process), COMcomparator);
        }

        clear();

        printf("%ld total no. of process | %ld running process | %ld physical memory | %ld mem used | %f cpu used",total_process, running_processs, phys_mem_size,total_mem, total_cpu);

        printf("\n");
        printf("%-*s", pidl, " PID ");
        printf("%-*s", coml, " command");
        printf("%-*s", statl," state");
        printf("%-*s", cpul, " CPU ");
        printf("%-*s", meml, " mem ");
        printf("%-*s", vszl, " VSZ ");
        printf("%-*s", rssl, " RSS ");
        printf("%-*s", cpuel, " CPU_excuted");
        
        //   printf("The terminal has %d rows and %d columns\n", wsize.ws_row, wsize.ws_col);
        //gotoxy(x, y)
        int loop = total_process;
        if (loop>wsize.ws_row-2){
            loop = wsize.ws_row-2;
        }
        for(int y =1; y< loop; y++){
            gotoxy(y+2,0);
            printf("%-*ld", pidl, list[y].pid);
            gotoxy(y+2,pidl+1);
            printf(" ");
            printf("%-*s", coml, list[y].command);
            gotoxy(y+2,pidl+coml+1);
            printf(" ");
            printf("%-*c", statl, list[y].state);
            gotoxy(y+2,pidl+coml+statl+1);
            printf(" ");
            printf("%-*f", cpul, list[y].cpu);
            gotoxy(y+2,pidl+coml+statl+cpul+1);
            printf(" ");
            printf("%-*f", meml, list[y].mem);
            gotoxy(y+2,pidl+coml+statl+cpul+meml+1);
            printf(" ");
            printf("%-*ld", vszl, list[y].VSZ);
            gotoxy(y+2,pidl+coml+statl+cpul+meml+vszl+1);
            printf(" ");
            printf("%-*ld", rssl, list[y].RSS);
            gotoxy(y+2,pidl+coml+statl+cpul+meml+vszl+rssl+1);
            printf(" ");
            printf("%-*ld", cpuel, list[y].CPU_exc);

        }
            printf("\n");
    

    }
    else
    {
        perror("Couldn't open the directory");
        exit(-1);
    }
    sleep(1);
    }
    return 0;
}
