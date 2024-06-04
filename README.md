# How did I solve the problem

To address the issue of cocurrency, I decided to use fork and pipes to handle computing the inforation of CPU, USER, and Memory through the child process, copying the information through it's own respective pipeline, terminating it when its over, and copying the information from the pipe in the parent node.

In addition, for signally, I decided to use the single library to handle signals, especiall when it comes to Ctrl-C and Ctrl-Z, where I would set up handle signal function at the very start, such that each process will always contain the process handling.

To go into detail, this is how I handle the corrency: First, before everytime I need the information of a specific subject (for example, memory), i would fork at that moment. Then having an if statement that choose the id to be 0(go to child process), use my function to get info, send the info through pipe, exit the child. While in the parent, it will wait for the child to finish copying the info into pipe, and the printing out the info on it's end. Overall, the parent always produced a child for each function, exit the child, recieve info from child, and print out the info.


# An overview of the functions:


typedef struct linkList


   this is a struct that creates a link list the stores a string for printing




typedef struct infoList


   this is a struct that stores the head of CPU info link list and Memory info link list




struct commandInfo


   this is a struct that stores all the userInputs on the command line.




Node *insertNodeBottom(Node *head, Node *newNode)


   This function inserts a node to the end of the linked list




void deleteNodeList(Node *head)


   This function free all the nodes in the linked list




void printPlaceholder()


   This function sets up a temporary place holder line for the display




void printBasicInfo(int samples, int tdelay)


   This function takes input of the samples and tdelay number, displaying them with printf()
   It also displays the current Memory use with library sys/resource.h




char *printMemoryInfo(char *str)


   This function takes in a str thats malloced
   it measures the physical Memory and Virtual memory with the sys/sysinfo.h library
   then storing it into the str, returning it afterward




char *printCurrentUserSessions(char *str)

   This function takes in a str thats malloced
   This functions get all the sessions/users currently in the System using the utmp.h library
   store it into str	and return the info through str



void printCoreNum()


   This functions print out total numbers of cores the machine has, using the sysconf function from unistd.h




float printCPUInfo(Node *head, Node *new_node, struct commandInfo *inputInfo)


   This function takes input of the CPU info head node, and all the user command inputs info from the file /proc/states
   it gets out the current CPU percentage, by measuring the CPU usage at start, and CPU usage again after the set delay
   then it return the cpu percentage as a float


This function uses the sagaction sa struct, which handles signals such as Ctrl-C or Ctrl-Z

void setup_signal_handling() 
	
	This function uses the sagaction sa struct, which handles signals such as Ctrl-C or Ctrl-Z


void sigint_handler(int sig) 

	This function compute the signal for Crtl-c, and if y is pressed, end with exit, if n pressed, continue.


void getGraphicInfo(struct commandInfo *inputInfo, InfoList *infoList, float loadavg)


	This function takes input of the Graphic info head node, and all the user command inputs info
	it prints out the current CPU percentage with a graphical display


void printSysInfo()


   This function prints out the current system information with the sysinfo.h, utsname.h library.
   it prints out System name, Machine name, Version, Release date, Architecture, and System uptime




void displayInfo(struct commandInfo *inputInfo)


   This function takes input of the list of commands from inputInfo,
   and displaying the information about the System based on the command line inputs


# How to run(use) the program


step 1: give your terminal enough space
step 2: in linux, use the following command:
gcc -o ./mySystemStats ./mySystemStats


step 3: in linux, use the following command with any input line:
./mySystemStas [--system] [--user] [--sequential] [--graphics] [--samples=N] [--tdelay=T] [samples] [tdelay]


The program should accept several command line arguments:


--system
       to indicate that only the system usage should be generated




--user


       to indicate that only the users usage should be generated


--graphics
	
	to indicate that graphics is used for cpu



--sequential


       to indicate that the information will be output sequentially without needing to "refresh" the screen (useful if you would like to redirect the output into a file)




--samples=N


       if used the value N will indicate how many times the statistics are going to be collected and results will be average and reported based on the N number of repetitions. If not value is indicated the default value will be 10.




--tdelay=T


       to indicate how frequently to sample in seconds. If not value is indicated the default value will be 1 sec


The last two arguments can also be considered as positional arguments if not flag is indicated in the corresponding order: samples tdelay. for example:


./mySystemStats 8 3


	where 8 will be the new samples number, tdelay will be the new tdelay number


note: the positional arguments must be right next to each other to work, and the first positional arg will always be sample number






postional argument

        only one positional arguments is need, where the system will only take in the FIRST position argument, rest will be ignored.
        if the position argument is given, the tables will only printf out info that contain the PID equal to the positional argument.
        if no valid PID is given, the table will display nothing.
        If argument not provided, the table will display all PID.






