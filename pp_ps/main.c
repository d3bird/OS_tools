#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

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

    // get the misc data that is needed
    long phys_pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    long phys_mem_size = phys_pages * page_size;
   // printf("page sizesize: %ld \n", page_size);
   // printf("phys_mem_size: %ld \n", phys_mem_size);

    struct winsize wsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize);
        //the formatting  the terminal 
        //set amount of space for the PID and state
        int statl = 10;
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
            //printf("the uptime is %f\n", uptime);
        }
    }
    fclose(fp1);

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
                //printf("reading stats from %s \n", newpath);

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
                        int flag =0;
                        for (int i = 0; i < size + 1; i++)
                        {
                            //check to makesure that it is not inside the () when determining name
                            if(data[i] == '(' && num_spaces ==1){
                                flag =1;
                            } else if(data[i] == ')' && num_spaces ==1){
                                flag =0;
                            }
                            
                            if (data[i] == ' ' && flag ==0)
                            {
                                //printf("%s\n",input);

                                index = 0;
                                //parse out the data based off the spaces
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

                                        //calculate the cpu
                                        float process_time =   ( list[cursor].utime / sysconf(_SC_CLK_TCK) ) + ( list[cursor].stime/ sysconf(_SC_CLK_TCK) );
                                        float real_time = uptime -( list[cursor].starttime / sysconf(_SC_CLK_TCK) );
                                       
                                        list[cursor].cpu= process_time * 100 / real_time;
                                    }
                                    break;
                                }
                                num_spaces++;
                                memset(input, 0,  100);
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

        //print the data

        printf("%-*s", pidl, "PID ");
        printf("%-*s", coml, " command ");
        printf("%-*s", statl, "state");
        printf("%-*s", cpul, "CPU ");
        printf("%-*s", meml, "mem ");
        printf("%-*s", vszl, "VSZ ");
        printf("%-*s", rssl, "RSS ");
        printf("%-*s", cpuel, "CPU_excuted");
        printf("\n");
        for (int i = 0; i < cursor; i++)
        {
            printf("%-*ld", pidl, list[i].pid);
            printf("%-*s", coml, list[i].command);
            printf("%-*c", statl, list[i].state);
            printf("%-*f", cpul, list[i].cpu);
            printf("%-*f", meml, list[i].mem);
            printf("%-*ld", vszl, list[i].VSZ);
            printf("%-*ld", rssl, list[i].RSS);
            printf("%-*ld", cpuel, list[i].CPU_exc);
            // printf("%s", list[i].command);
            printf("\n");
        }
    }
    else
    {
        perror("Couldn't open the directory");
        exit(-1);
    }
    return 0;
}
