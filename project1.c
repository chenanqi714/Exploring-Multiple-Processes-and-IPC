#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <time.h>

#define N 2000
#define length 100

typedef int bool;
#define true 1
#define false 0

/*
The fork() function is used to create a new process from an existing process. The new process is called the child process, and the existing process is called the parent. You can tell which is which by checking the return value from fork(). The parent gets the child's pid returned to him, but the child gets 0 returned to him. 
*/

struct information {
	int action;      //action = 0 read; action = 1 write; action = -1 exit
	int index;
};

int random_int(int min, int max)
{
	return min + rand() % (max + 1 - min);
}

bool check_address(int address, int mode) {
	if (address < 0 || address >= 2000) {
		printf("Fail. Trying to access invalid memory address.\n");
		return false;
	}
	if (mode == 0 && address >= 1000) {
		printf("User has no access to protected memory\n");
		return false;
	}
	else
		return true;
}

int main(int argc, char *argv[])
{
   pid_t pid;
   int pd1[2], pd2[2];
   pipe(pd1);
   pipe(pd2);
   int timer;

   if (argc != 3) {
	   printf("usage: %s filename X\n", argv[0]);
	   return 1;
   }
   
   if (atoi(argv[2]) <= 0) {
	   printf("Invalid timer value\n");
	   return 1;
   }

   switch (pid = fork())
   {
       case -1:
	   {
			  /* Here pid is -1, the fork failed */
			  /* Some possible reasons are that you're */
			  /* out of process slots or virtual memory */
			  printf("The fork failed!");
			  exit(-1);
	    }
	   
      case 0: //child process: memory
	    {
			int memory[N];
			int i = 0;
			int num;
			int index;
			char *line;
			float fl;
			size_t len = 0;
			struct information info;

			FILE *f = fopen(argv[1], "r");
			if (!f) {
				printf("Error loading file.\n");
				_exit(1);
			}

			line = (char *)malloc(length * sizeof(char));
			while (getline(&line, &len, f) != -1) {             // read from file line by line
				if (line[0] == '.' && line[1] >= '0' && line[1] <= '9') {
					sscanf(line, "%f", &fl);
					i = (int)(10000 * fl);
					continue;
				}

				else if (line[0] >= '0' && line[0] <= '9') {
					sscanf(line, "%d", &num);
					memory[i] = num;
					++i;
				}
			}
			free(line);
			fclose(f);

			  while (1) {
				  read(pd1[0], &info, sizeof(info));              // read action and address from pd1
				  switch (info.action) {
				  case 0:                                         // action = 0 read from memory
					  index = info.index;                         //fetch the address from pd1 and save to index
					  write(pd2[1], &memory[index], sizeof(int)); //send the content at address index to pd2
					  break;
				  case 1:                                         // action = 1 write to memory
					  index = info.index;                         // fetch the address from pd1 and save to index
					  read(pd1[0], &num, sizeof(int));            // fetch the value from pd1 and save to num
					  memory[index] = num;                        // write to memory
					  break;
				  case -1:                                        // action = -1 end process
					  _exit(0);
				  default:
					  _exit(1);
				  }
				  
			  }
		      _exit(0);
	      }

      
        default: //parent process: CPU
           {
			   int PC = 0;
			   int IR = 0;
			   int SP = 1000;
			   int AC = 0; 
			   int X = 0;
			   int Y = 0;
			   int rig1 = 0;
			   int rig2 = 0;
			   int flag = 0;  // exit if flag = 1;
			   int port = 0;
			   int count = 0; // timer
			   int mode = 0;  // user mode = 0; kernel mode = 1;
			   struct information info;

			   srand(time(0));
			   while (flag == 0) {
				   info.action = 0;                            // set initial value of action to 0 (read from memory)
				   info.index = PC;                            // set initial value of index to PC (= 0)
				   write(pd1[1], &info, sizeof(info));         // send info (including action and PC) to pd1  
				   read(pd2[0], &IR, sizeof(int));             // fetch instruction from pd2
				   switch (IR)                                 // decode instruction
				   {
				   case 1:                                     // load value into AC
					   info.action = 0;                        // action is read from memory             
					   info.index = PC + 1;                      
					   write(pd1[1], &info, sizeof(info));
					   read(pd2[0], &AC, sizeof(int));         // load the value at PC+1 into AC
					   PC = PC + 2;                            // set PC to next instruction
					   break;
				   case 2:                                     // load the value at address into AC
					   info.action = 0;                        // action is read from memory
					   info.index = PC + 1;					   
					   write(pd1[1], &info, sizeof(info));
					   read(pd2[0], &rig1, sizeof(int));       // load the address at PC+1 into rig1
					   info.index = rig1;                      

					   if (check_address(info.index, mode) == false)
						   return 1;

					   write(pd1[1], &info, sizeof(info));     
					   read(pd2[0], &AC, sizeof(int));         // fetch the value at that address and save to AC
					   PC = PC + 2;
					   break;
				   case 3:                                     // Load the value from the address found in the given address into the AC
					   info.action = 0;                        // action is read from memory
					   info.index = PC + 1;
					   write(pd1[1], &info, sizeof(info));
					   read(pd2[0], &rig1, sizeof(int));       
					   info.index = rig1;                      // load the address at PC+1 into rig1

					   if (check_address(info.index, mode) == false)
						   return 1;

					   write(pd1[1], &info, sizeof(info));     
					   read(pd2[0], &rig1, sizeof(int));       // fetch the value (which is an address also) at that address and save to rig1
					   info.index = rig1;                      

					   if (check_address(info.index, mode) == false)
						   return 1;

					   write(pd1[1], &info, sizeof(info));     
					   read(pd2[0], &AC, sizeof(int));         // fetch the value at that address and save to AC
					   PC = PC + 2;
					   break;
				   case 4:                                     // Load the value at (address+X) into the AC
					   info.action = 0;
					   info.index = PC + 1;
					   write(pd1[1], &info, sizeof(info));
					   read(pd2[0], &rig1, sizeof(int));       // load the address at PC into rig1
					   rig1 = rig1 + X;                        // set rig1 to rig1 + X
					   info.index = rig1;                      

					   if (check_address(info.index, mode) == false)
						   return 1;

					   write(pd1[1], &info, sizeof(info));    
					   read(pd2[0], &AC, sizeof(int));         // fetch the value at that address and save to AC
					   PC = PC + 2;
					   break;
				   case 5:
					   info.action = 0;
					   info.index = PC + 1;
					   write(pd1[1], &info, sizeof(info));
					   read(pd2[0], &rig1, sizeof(int));       // load the address at PC into rig1
					   rig1 = rig1 + Y;                        // set arig1 to rig1 + Y
					   info.index = rig1;                      

					   if (check_address(info.index, mode) == false)
						   return 1;

					   write(pd1[1], &info, sizeof(info));     
					   read(pd2[0], &AC, sizeof(int));         // fetch the value at that address and save to AC
					   PC = PC + 2;
					   break;
				   case 6:                                     // read from address SP + X and save to AC
					   info.action = 0;                        // action is read from memory
					   info.index = SP + X;                    // set address to SP + X
					   if (check_address(info.index, mode) == false)
						   return 1;
					   write(pd1[1], &info, sizeof(info));     
					   read(pd2[0], &AC, sizeof(int));         // fetch the value from address and save to AC
					   PC = PC + 1;
					   break;
				   case 7:                                     // store AC to address
					   info.action = 0;                        // action is write to memory
					   info.index = PC + 1;
					   write(pd1[1], &info, sizeof(info));
					   read(pd2[0], &rig1, sizeof(int));       // fetch address
					   info.action = 1;
					   info.index = rig1;                     
					   if (check_address(info.index, mode) == false) // check if valid address
						   return 1;
					   write(pd1[1], &info, sizeof(info));     
					   write(pd1[1], &AC, sizeof(int));        // store AC
					   PC = PC + 2;
					   break;
				   case 8:
					   AC = random_int(1, 100);                // generate a random num from 1 to 100 and store in AC
					   PC = PC + 1;
					   break;
				   case 9:                                     // outout AC to screen
					   info.action = 0;
					   info.index = PC + 1;
					   write(pd1[1], &info, sizeof(info));
					   read(pd2[0], &port, sizeof(int));       // load the value at PC into port
					   if (port == 1)
						   printf("%d", AC);
					   else if (port == 2)
						   printf("%c", AC);
					   else
						   printf("Wrong port number.\n");
					   PC = PC + 2;
					   break;
				   case 10:
					   AC = AC + X;
					   PC = PC + 1;
					   break;
				   case 11:
					   AC = AC + Y;
					   PC = PC + 1;
					   break;
				   case 12:
					   AC = AC - X;
					   PC = PC + 1;
					   break;
				   case 13:
					   AC = AC - Y;
					   PC = PC + 1;
					   break;
				   case 14:
					   X = AC;
					   PC = PC + 1;
					   break;
				   case 15:
					   AC = X;
					   PC = PC + 1;
					   break;
				   case 16:
					   Y = AC;
					   PC = PC + 1;
					   break;
				   case 17:
					   AC = Y;
					   PC = PC + 1;
					   break;
				   case 18:
					   SP = AC;
					   PC = PC + 1;
					   break;
				   case 19:
					   AC = SP;
					   PC = PC + 1;
					   break;
				   case 20:                                      // jump to address
					   info.action = 0;
					   info.index = PC + 1;
					   write(pd1[1], &info, sizeof(info));
					   read(pd2[0], &PC, sizeof(int));           // load the address at PC into PC
					   if (check_address(PC, mode) == false)
						   return 1;
					   break;
				   case 21:                                      // jump to address if AC == 0
					   info.action = 0;
					   info.index = PC + 1;
					   write(pd1[1], &info, sizeof(info));
					   read(pd2[0], &rig1, sizeof(int));         // load the address at PC into rig1
					   
					   if (AC == 0) {                            //check if AC is 0
						   PC = rig1;                            // if true, set PC to that address
						   if (check_address(PC, mode) == false)
							   return 1;
					   }
					   else {
						  PC = PC + 2;                           // else, set PC to the address of next instruction
					   }
					   break;
				   case 22:                                      // jump to address if AC != 0
					   info.action = 0;
					   info.index = PC + 1;
					   write(pd1[1], &info, sizeof(info));
					   read(pd2[0], &rig1, sizeof(int));         // load the address at PC into rig1
					   if (AC != 0) {                            //check if AC is 0
						   PC = rig1;                            // if alse, set PC to that address
						   if (check_address(PC, mode) == false) 
							   return 1;
					   }
					   else {                                    // else, set PC to the address of next instruction
						   PC = PC + 2;
					   }
					   break;
				   case 23:                                      // call address
					   info.action = 1;                          // action is write to memory
					   info.index = SP - 1;                                                  
					   write(pd1[1], &info, sizeof(info));       // send action and address to memory
					   rig1 = PC + 2;
					   write(pd1[1], &rig1, sizeof(int));        // store PC
					   SP--;
					   info.action = 0;
					   info.index = PC + 1;
					   write(pd1[1], &info, sizeof(info));
					   read(pd2[0], &PC, sizeof(int));           // load the address at PC into PC
					   if (check_address(PC, mode) == false)
						   return 1;
					   break;
				   case 24:                                      // return from call. pop PC from stack
					   info.action = 0;
					   info.index = SP;
					   write(pd1[1], &info, sizeof(info));
					   read(pd2[0], &PC, sizeof(int));           // load the value at SP into PC
					   SP++;
					   break;
				   case 25:
					   X = X + 1;
					   PC = PC + 1;
					   break;
				   case 26:
					   X = X - 1;
					   PC = PC + 1;
					   break;
				   case 27:                                       // push AC to stack
					   info.action = 1;                           // action is write to memory
					   info.index = SP - 1;                    
					   write(pd1[1], &info, sizeof(info));        // send action and index to memory
					   write(pd1[1], &AC, sizeof(int));           // store AC
					   SP--;
					   PC = PC + 1;
					   break;
				   case 28:                                       // pop AC from stack
					   info.action = 0;
					   info.index = SP;
					   write(pd1[1], &info, sizeof(info));
					   read(pd2[0], &AC, sizeof(int));            // load the value at SP into AC
					   SP++;
					   PC = PC + 1;
					   break;
				   case 29:                                       // system call
					   if (mode == 0) {                           // checi if in user mode
						   info.action = 1;
						   info.index = N - 1;                    
						   write(pd1[1], &info, sizeof(info));    
						   write(pd1[1], &SP, sizeof(int));       // store SP at system stack (memory[n - 1])
						   info.index = N - 2;
						   write(pd1[1], &info, sizeof(info));    
						   PC = PC + 1;
						   write(pd1[1], &PC, sizeof(int));       // store PC at system stack (memory[n - 2])
						   SP = N - 2;
						   PC = 1500;
						   mode = 1;                              // switch to kernel mode
					   }
					   break;
				   case 30:                                       // return from system call
					   info.action = 0;
					   info.index = N - 1;
					   write(pd1[1], &info, sizeof(info));
					   read(pd2[0], &SP, sizeof(int));            // restore SP
					   info.action = 0;
					   info.index = N - 2;
					   write(pd1[1], &info, sizeof(info));
					   read(pd2[0], &PC, sizeof(int));            // restore PC
					   mode = 0;
					   break;
				   case 50:                                       // exit parent process
					   info.action = -1;
					   write(pd1[1], &info, sizeof(info));
					   waitpid(-1, NULL, 0);
					   flag = 1;
					   break;
				   default:                                       // invalid instrucion not 1 - 30 or 50
					   printf("Invalid instruction: IR = %d\n", IR);
					   return 1;
				   }
				   if(mode == 0)                                  //if in user mode count + 1
				        count++;
;				   if (count == atoi(argv[2]) && mode == 0 && IR != 30) { // system call caused by timer. If return from system call, don't do system call again
					   info.action = 1;
					   info.index = N - 1;                    
					   write(pd1[1], &info, sizeof(info));    
					   write(pd1[1], &SP, sizeof(int));       // store SP at system stack (memory[n - 1])
					   info.index = N - 2;
					   write(pd1[1], &info, sizeof(info));    
					   write(pd1[1], &PC, sizeof(int));       // store PC at system stack (memory[n - 2])
					   SP = N - 2;
					   PC = 1000;
					   mode = 2;
					   count = 0;                             // reset timer
				   }
                   if (count == atoi(argv[2]))                // reset timer
                       count = 0;
			   }
           }
     }
}
 

