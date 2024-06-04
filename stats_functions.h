#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <getopt.h>
#include <utmp.h>
#include <sys/resource.h>
#include <string.h>
#define BUFFER_SIZE 2048
/*
the is a struct that creates a link list the stores a string for printing
*/
typedef struct linkList
{
    char str[1024];
    struct linkList *next;
} Node;

/*
the is a struct that stores the head of CPU info link list and Memory info link list
*/
typedef struct infoList
{

    float CPU_head; 
     Node *Graphic_head;

    Node *Memory_head; 


} InfoList;

/*
the is a struct that stores all the userInputs on the command line.
*/
struct commandInfo 
{
    int system_flag;
    int user_flag;
    int graphics_flag;
    int sequential_flag;
    int samples;
    int tdelay;
};

/*
This function inserts a node to the end of the linked list
*/
Node *insertNodeBottom(Node *head, Node *newNode)
{

    if (newNode == NULL) //if no newNode created
    {
        return NULL;
    }
    if (head == NULL) //if there's no head node
    {
        return newNode; //set headnode as new node
    }

    // loop through the list and find the last node
    Node *current = head;
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = newNode; //insert the new node into the last node.
    return head;
}


/*
This function free all the nodes in the linked list
*/
void deleteNodeList(Node *head)
{

    while (head != NULL) //looping through each node in the link list and freeing them
    {
        Node *x = head;
        head = head->next;
        free(x);
    }
}

/*
This function sets up a temporary place holder line for the display
*/
void printPlaceholder()
{
    printf("\n");
}

/*
This function takes input of the samples and tdelay number, displaying them with printf()
It also displays the current Memory use with library <sys/resource.h>
*/
void printBasicInfo(int samples, int tdelay)
{
    struct rusage *r_usage = malloc(sizeof(struct rusage));
    if (r_usage == NULL)
    {
        fprintf(stderr, "Memory not allocated");
        exit(EXIT_FAILURE);
    }

    int success = getrusage(RUSAGE_SELF, r_usage);
    if (success == 0)
    {
        printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay);
        // Print the maximum resident set size used (in kilobytes).
        printf("Memory usage: %ld kilobytes\n", r_usage->ru_maxrss);
    }else{
        fprintf(stderr, "struct not completed");
        exit(EXIT_FAILURE);
    }
    free(r_usage);
}

/*
This function takes in a new node for the link list
it measures the physical Memory and Virtual memory with the sysinfo library
then storing it into the node's str info
*/
char *printMemoryInfo(char *mem_str)
{
    struct sysinfo *info = malloc(sizeof(struct sysinfo));
    if (info == NULL)
    {
        fprintf(stderr, "Memory not allocated");
        exit(EXIT_FAILURE);
    }

    int success_sysinfo = sysinfo(info); //fill in all the fields within the struct, return 0 if successful
    if (success_sysinfo == 0)
    {
        double physical_use = (double)(info->totalram - info->freeram) * info->mem_unit / (1024 * 1024 * 1024); 
        double physical_total = (double)(info->totalram) * info->mem_unit / (1024 * 1024 * 1024);
        double swap_use = (double)(info->totalswap - info->freeswap) * info->mem_unit / (1024 * 1024 * 1024);
        double swap_total = (double)(info->totalswap) * info->mem_unit / (1024 * 1024 * 1024);
        double virtual_use = physical_use + swap_use;
        double virtual_total = physical_total + swap_total;
        //the above info returns physical memory in GB, and virtual memory in GB

        printf("%0.2f GB / %0.2f GB  -- %0.2f GB / %0.2f GB\n", physical_use, physical_total, virtual_use, virtual_total);
        sprintf(mem_str, "%0.2f GB / %0.2f GB  -- %0.2f GB / %0.2f GB\n", physical_use, physical_total, virtual_use, virtual_total);
    }
    return mem_str;
}

/*
This functions print out all the sessions/users currently in the System
*/
char *printCurrentUserSessions()
{
    struct utmp *currentUserSession = malloc(sizeof(struct utmp));
    char *stringcpy = malloc(1024 * sizeof(char));
    *stringcpy = '\0';
    char buffer[BUFFER_SIZE] = "";
    //opens the utmp file
    setutent();

    while ((currentUserSession = getutent()) != NULL)//reads each line in utent file
    {
        
        if (currentUserSession->ut_type == USER_PROCESS) //checks if the user is logged in
        {
            sprintf(buffer," %s\t%s\t(%s)\n",
                   currentUserSession->ut_user,  //user's username
                   currentUserSession->ut_line,  //user's terminal name
                   currentUserSession->ut_host); //user's hostname/IP address
        }
        strcat(stringcpy, buffer);
    }
 

    // close the utmp file
    endutent();
    free(currentUserSession);
    return stringcpy;
}

/*
This functions print out total numbers of cores the machine has
*/
void printCoreNum()
{
    printf("Number of cores: %ld\n", sysconf(_SC_NPROCESSORS_ONLN)); //print out the Number of processors online(which reflects number of cores)
    
}

void printGraphicsInfo(Node *graphic_node, float cur_cpu_info){
    int multi = (int)(cur_cpu_info / 0.5);
    int needed_size = 10 + multi + 1;  
    char *baseStr = malloc(needed_size);
    
    if (baseStr == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }

    strcpy(baseStr, "      |");
    for (int i = 0; i < multi; i++) {
        strcat(baseStr, "|");
    }
    
    printf("%s %0.2f\n", baseStr, cur_cpu_info);
    
    
    if (strlen(baseStr) + 10 < 1024) {  
        sprintf(graphic_node->str, "%s %0.2f\n", baseStr, cur_cpu_info);
    } else {
        fprintf(stderr, "Buffer overflow prevented\n");
    }
    
    free(baseStr);
}

/*
This function takes input of the CPU info head node, and all the user command inputs info
it prints out the current CPU percentage, by measuring the CPU usage at start, and CPU usage again after the set delay
*/
float printCPUInfo(struct commandInfo *inputInfo, InfoList *infoList)
{
    long double a[7], b[7];
    double loadavg;
    double last_idle;
    double last_total;
    double current_total;
    double current_idle;
    FILE *fp;

    // float tmp_float = infoList->CPU_head;
    // if(tmp_float != -1){ //checks if this is the first time printing the CPU percentage
                              
    // if(inputInfo->sequential_flag == 0 && inputInfo->user_flag == 0){ //checks the user commands does not have --sequential or --user
    //    printf(" total cpu use = %0.2f\n", tmp_float);
    //     printf("\033[1A");
    // }
    // }

    // first read of the file 
        fp = fopen("/proc/stat", "r");
        if (fp == NULL)
        {
            fprintf(stderr, "Failed to open /proc/stat");
            exit(EXIT_FAILURE);
        }
        fscanf(fp, "%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6]);
        last_idle = (double)(a[3]); // idle
        last_total = (double)(a[0] + a[1] + a[2] + a[3] + a[4] + a[5] + a[6]); //total CPU memory usage
        fclose(fp);

   
    sleep(inputInfo->tdelay); // Wait in delay time before the second meassure

    // Second read
    fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Failed to open /proc/stat");
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3], &b[4], &b[5], &b[6]);
    current_idle = (double)(b[3]); // idle
    current_total = (double)(b[0] + b[1] + b[2] + b[3] + b[4] + b[5] + b[6]); //total CPU memory usage
    fclose(fp);


    double diff_total = current_total - last_total;
    double diff_idle = current_idle - last_idle;

    loadavg = (diff_total - diff_idle) / diff_total;
    loadavg *= 100;
    return loadavg;
    
      


}


/*
This function takes input of the Graphic info head node, and all the user command inputs info
it prints out the current CPU percentage with a graphical display
*/
void getGraphicInfo(struct commandInfo *inputInfo, InfoList *infoList, float loadavg){
    int empty_space_index = inputInfo->samples;
        if (inputInfo->graphics_flag == 1) //if --graphics is an input
        {
            
            Node *tmp_graphic_head = infoList->Graphic_head;

            //prints out all the previous memory line.
            while (tmp_graphic_head!= NULL) 
            {
                empty_space_index--;
                if (inputInfo->sequential_flag == 0)
                {
                    printf("%s", tmp_graphic_head->str);
                }
                else
                {
                    printf("\n");
                }
                tmp_graphic_head = tmp_graphic_head->next;
            }
            Node *new_graphic_node = malloc(sizeof(Node));
            strcpy(new_graphic_node->str,"");
            if (new_graphic_node == NULL)
            {
                fprintf(stderr, "Memory not allocated");
                exit(EXIT_FAILURE);
            }
            printGraphicsInfo(new_graphic_node, loadavg); //prints out the current graphic line
            infoList->Graphic_head = insertNodeBottom(infoList->Graphic_head, new_graphic_node);  //store the graphic print line into the linked list
            empty_space_index--;
            infoList->CPU_head = loadavg;
            for (int j = empty_space_index; j > 0; j--) //reserve empty space for future graphic line prints
            {
                
                printf("\n"); // print Empty line
            }
        }
}


/*
This function prints out the current system information
such as System name, Machine name, Version, Release date, Architecture, and System uptime
*/
void printSysInfo()
{

    struct utsname *buf = malloc(sizeof(struct utsname));
    if (buf == NULL)
    {
        fprintf(stderr, "Memory not allocated");
        exit(EXIT_FAILURE);
    }

    struct sysinfo *info = malloc(sizeof(struct sysinfo));
    if (info == NULL)
    {
        fprintf(stderr, "Memory not allocated");
        exit(EXIT_FAILURE);
    }

    int success_sysinfo = sysinfo(info); //fill in all the fields within the struct, return 0 if successful
    int success_utsname = uname(buf); //fill in all the fields within the struct, return 0 if successful

    if (success_utsname == 0 && success_sysinfo == 0)
    {
        //converting the total uptime of the System from seconds to other units
        int uptime_seconds = info->uptime;
        int days = uptime_seconds / (24 * 3600);
        int hours = (uptime_seconds - (days * 24 * 3600)) / 3600;
        int minutes = (uptime_seconds - (days * 24 * 3600) - (hours * 3600)) / 60;
        int seconds = uptime_seconds - (days * 24 * 3600) - (hours * 3600) - (minutes * 60);
        int total_hours = uptime_seconds / 3600;
        int total_minutes = (uptime_seconds - (total_hours * 3600)) / 60;

        printf("---------------------------------------\n");
        printf("### System Information ###\n");
        printf(" System Name = %s\n", buf->sysname);
        printf(" Machine Name = %s\n", buf->nodename);
        printf(" Version =  %s\n", buf->version);
        printf(" Release = %s\n", buf->release);
        printf(" Architecture = %s\n", buf->machine);
        printf(" System running since last reboot: %d days %02d:%02d:%02d (%d:%02d)\n", days, hours, minutes, seconds, total_hours, total_minutes);
        printf("---------------------------------------\n");
    }
    else
    {
        fprintf(stderr, "struct not completed");
        exit(EXIT_FAILURE);
    }

    free(buf);
    free(info);
}
