#include <signal.h>

#include "stats_functions.h"


/*
This function compute the signal for Crtl-c, and if y is pressed, end with exit, if n pressed, continue.
*/
void sigint_handler(int sig) {
    char c;

    signal(sig, SIG_IGN);
    printf("\nDo you really want to quit? [y/n] ");
    c = getchar();
    if (c == 'y' || c == 'Y')
        exit(0);
    else
        signal(SIGINT, sigint_handler); //Continue signal 
}


/*
This function uses the sagaction sa struct, which handles signals such as Ctrl-C or Ctrl-Z
*/
void setup_signal_handling() {
    struct sigaction sa;

    //set up SIGINT (Ctrl-C)
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error: cannot handle SIGINT");
    }

    //ignore SIGTSTP (Ctrl-Z)
    sa.sa_handler = SIG_IGN;
   	
   	//does nothing
    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        perror("Error: cannot ignore SIGTSTP"); 
    }
}


/*
This function takes input of the list of commands from inputInfo,
and displaying the information about the System based on the command line inputs
*/
void displayInfo(struct commandInfo *inputInfo)
{
setup_signal_handling();
    if (inputInfo == NULL) //No inputInfo is given
    {
        return;
    }

    InfoList *infoStorage = malloc(sizeof(InfoList));
    

    Node *Memory_head = NULL;
    char buffer[BUFFER_SIZE] ="";
    infoStorage->CPU_head = -1;
    infoStorage->Graphic_head = NULL;
    infoStorage->Memory_head = NULL;

    int mem_pipe[2],user_pipe[2],cpu_pipe[2];

    printf("\033[2J"); // clear screen
    printf("\033[0;0H"); // move Cursor to 0 0;

    for (int i = 0; i < inputInfo->samples; i++) //loop based on the number of samples
    {
        if(pipe(mem_pipe)==-1 || pipe(user_pipe)==-1 || pipe(cpu_pipe)==-1){ //create/call pipe
            fprintf(stderr,"pipe failed");
        }
        pid_t memPID,userPID,cpuPID;
        int empty_space_index = inputInfo->samples;
        if (inputInfo->sequential_flag == 0) //if --sequential is not a input, don't refresne screen
        {
             // clear screen
            printf("\033[0;0H"); // move Cursor to 0 0;

        }
        printBasicInfo(inputInfo->samples, inputInfo->tdelay); 
        if (inputInfo->user_flag == 0) //if --user is not an input
        {
            printf("---------------------------------------\n");
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
            Node *Memory_node = infoStorage->Memory_head;

            //prints out all the previous memory line.
            while (Memory_node != NULL) 
            {
                empty_space_index--;
                if (inputInfo->sequential_flag == 0)
                {
                    printf("%s", Memory_node->str);
                }
                else
                {
                    printf("\n");
                }
                Memory_node = Memory_node->next;
            }
 
         
          




           Node *new_mem_node = malloc(sizeof(Node));
           if (new_mem_node == NULL)
           {
               fprintf(stderr, "Memory not allocated");
               exit(EXIT_FAILURE);
           }
        memPID=fork();
        if(memPID== -1){// create child process to write the stat by using fcn from stat_function.c
           fprintf(stderr, "fork not initiated");
          
          exit(EXIT_FAILURE); 
       }
        
           
        if(memPID==0){// create child process to write the stat by using fcn from stat_function.c
           close(mem_pipe[0]); //generate other close fds.
           char *mem_str = malloc(1024*sizeof(char));
          mem_str = printMemoryInfo(mem_str);
           write(mem_pipe[1],mem_str, strlen(mem_str));
           free(mem_str);
          
          exit(0);
       }

     
      close(mem_pipe[1]);
       memset(buffer, 0, BUFFER_SIZE);
       read(mem_pipe[0], buffer, BUFFER_SIZE);
       close(mem_pipe[0]);

           
           strcpy(new_mem_node->str,buffer);
          // printMemoryInfo(new_mem_node); //prints out the current memory line
           Memory_head = insertNodeBottom(Memory_head, new_mem_node);  //store the Memory print line into the linked list
           infoStorage->Memory_head = Memory_head;
           empty_space_index--;


            for (int j = empty_space_index; j > 0; j--) //reserve empty space for future memory line prints
            {
                printf("\n"); // print Empty line
            }
        }

        //prints when --system is not an input command
        if(inputInfo->system_flag == 0){
    printf("---------------------------------------\n");
    printf("### Sessions/users ###\n");
             userPID=fork();
             if(userPID== -1){// create child process to write the stat by using fcn from stat_function.c
           fprintf(stderr, "fork not initiated");
          
          exit(EXIT_FAILURE); 
       }
      
        
           
        if(userPID==0){// create child process to write the stat by using fcn from stat_function.c
           close(user_pipe[0]); //generate other close fds.
           char *user_str;
        user_str = printCurrentUserSessions();
           write(user_pipe[1],user_str, strlen(user_str));
           free(user_str);
          
          exit(0);
       }

     
      close(user_pipe[1]);
       memset(buffer, 0, BUFFER_SIZE);
       read(user_pipe[0], buffer, BUFFER_SIZE);
       close(user_pipe[0]);
       printf("%s",buffer);
        printf("---------------------------------------\n");
        }

        //prints when --user is not an input command
         if (inputInfo->user_flag == 0){
        printCoreNum();
         }


//below is for cpu fork
        cpuPID=fork();
        if(cpuPID== -1){// create child process to write the stat by using fcn from stat_function.c
           fprintf(stderr, "fork not initiated");
          
          exit(EXIT_FAILURE); 
       }

        
        if(cpuPID==0){// create child process to write the stat by using fcn from stat_function.c
           close(cpu_pipe[0]); //generate other close fds.
           float cpu_float = printCPUInfo(inputInfo, infoStorage);
           write(cpu_pipe[1],&cpu_float, sizeof(float));
          
          exit(0); 
       }

     float recieve_cpu_float;
      close(cpu_pipe[1]);
       read(cpu_pipe[0], &recieve_cpu_float, sizeof(float));
       close(cpu_pipe[0]);

       
       
       if(inputInfo->user_flag == 0){ //checks for --user command line input
   printf(" total cpu use = %0.2f\n", recieve_cpu_float);
   if(inputInfo->graphics_flag == 1){
   getGraphicInfo(inputInfo, infoStorage,recieve_cpu_float);

       }}

       
    //sleep(1);
//end of cpu fork       
    }
    printSysInfo();

    deleteNodeList(infoStorage->Graphic_head);
    deleteNodeList(infoStorage->Memory_head);
    free(infoStorage);
}

int main(int argc, char **argv)
{

    // Define input options for the command line
    static struct option long_options[] = {
        {"system", no_argument, 0, 's'},
        {"user", no_argument, 0, 'u'},
        {"graphics", no_argument, 0, 'g'},
        {"sequential", no_argument, 0, 'q'},
        {"samples", optional_argument, 0, 'n'},
        {"tdelay", optional_argument, 0, 't'},
        {0, 0, 0, 0} 
    };

    struct commandInfo *inputInfo = malloc(sizeof(struct commandInfo));
    if (inputInfo == NULL)
    {
        fprintf(stderr, "Memory not allocated");
        exit(EXIT_FAILURE);
    }

    //sets up default command line inputs
    int opt;
    inputInfo->system_flag = 0;
    inputInfo->user_flag = 0;
    inputInfo->sequential_flag = 0;
    inputInfo->graphics_flag = 0;
    inputInfo->samples = 10;
    inputInfo->tdelay = 1;
    int option_index = 0;

    //reads each argv command and set up the commandInfo;
    while ((opt = getopt_long(argc, argv, "suqg:n::t:", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 's': // --system
            inputInfo->system_flag = 1;
            break;
        case 'u': // --user
            inputInfo->user_flag = 1;
            break;
        case 'q': // --sequential
            inputInfo->sequential_flag = 1;
            break;
        case 'g': // --sequential
            inputInfo->graphics_flag = 1;
            break;
        case 'n': // --samples
            inputInfo->samples = atoi(optarg);
            break;
        case 't': // --tdelay
            inputInfo->tdelay = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s [--system] [--user] [--sequential] [--graphics] [--samples=N] [--tdelay=T] [samples] [tdelay]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // process positional arguments 
    if (optind < argc)
    {
        inputInfo->samples = atoi(argv[optind++]);
        if (optind < argc)
        {
            inputInfo->tdelay = atoi(argv[optind++]);
        }
    }

    displayInfo(inputInfo);


    free(inputInfo);

    return 0;
}
