#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

void clear_buffer(){
  /*
    clears buffer
  */
  char buffer[100];
  while(fgets(buffer,100,stdin)!=NULL){
    if(buffer[strlen(buffer)-1]=='\n') return;
  }
}

void run_command(int sig,siginfo_t *info,void *context){
  /*
    runs commands according to the signal info value
  */
  switch(info->si_value.sival_int){//checks data sent alongside signal
    
    case 2:{
      //list hunts
      int command_pid;
      if((command_pid=fork())<0){
	perror(NULL);
	exit(-1);
      }
      if(command_pid==0){
	if(execl("./treasure_hunt","treasure_hunt","--list_hunts",NULL)==-1){
	  perror(NULL);
	  exit(-1);
	}
      }
      int status;
      if(waitpid(command_pid,&status,0)==-1){
	perror(NULL);
	exit(-1);
      }
      if(WIFEXITED(status) && WEXITSTATUS(status)!=0) printf("Encountered an error while running this command (EXIT CODE: %d)\n",WEXITSTATUS(status));
      if(kill(getppid(),SIGUSR2)!=0){//sends signal to parent process that it completed its task
	perror(NULL);
	exit(-1);
      }
      break;
    }
      
    case 3:{
      //list treasures

      //reads hunt name length and hunt name from command file
      char hunt_name[100],hunt_name_len;
      if(read(0,&hunt_name_len,1)==-1){
	perror(NULL);
	exit(-1);
      }
      if(read(0,hunt_name,hunt_name_len)==-1){
	perror(NULL);
	exit(-1);
      }
      
      int command_pid;
      if((command_pid=fork())<0){
	perror(NULL);
	exit(-1);
      }
      if(command_pid==0){
	if(execl("./treasure_hunt","treasure_hunt","--list",hunt_name,NULL)==-1){
	  perror(NULL);
	  exit(-1);
	}
      }
      
      int status;
      if(waitpid(command_pid,&status,0)==-1){
	perror(NULL);
	exit(-1);
      }
      if(WIFEXITED(status) && WEXITSTATUS(status)!=0) printf("Encountered an error while running this command (EXIT CODE: %d)\n",WEXITSTATUS(status));
      if(kill(getppid(),SIGUSR2)!=0){
	perror(NULL);
	exit(-1);
      }
      break;
    }
      
    case 4:{
      //view treasure

      //reads use case
      char use_case;
      if(read(0,&use_case,1)==-1){
	perror(NULL);
	exit(-1);
      }
      
      char hunt_name[100],hunt_name_length;
      char input1[100],input1_length;
      char input2[100],input2_length;

      //reads hunt name length and hunt name from command file
      if(read(0,&hunt_name_length,1)==-1){
	perror(NULL);
	exit(-1);
      }
      if(read(0,hunt_name,hunt_name_length)==-1){
	perror(NULL);
	exit(-1);
      }

      //reads parameter 1 length and parameter 1 from command file
      if(use_case!=0){
	if(read(0,&input1_length,1)==-1){
	  perror(NULL);
	  exit(-1);
	}
	if(read(0,input1,input1_length)==-1){
	  perror(NULL);
	  exit(-1);
	}
      }

      //reads parameter 2 length and parameter 2 from command file if necessary
      if(use_case==2){
	if(read(0,&input2_length,1)==-1){
	  perror(NULL);
	  exit(-1);
	}
	if(read(0,input2,input2_length)==-1){
	  perror(NULL);
	  exit(-1);
	}
      }
      
      int command_pid;
      if((command_pid=fork())<0){
	perror(NULL);
	exit(-1);
      }
      if(command_pid==0){
	if(use_case==0){
	  if(execl("./treasure_hunt","treasure_hunt","--view",hunt_name,NULL)==-1){
	    perror(NULL);
	    exit(-1);
	  }
	}
	if(use_case==1){
	  if(execl("./treasure_hunt","treasure_hunt","--view",hunt_name,input1,NULL)==-1){
	    perror(NULL);
	    exit(-1);
	  }
	}
	if(use_case==2){
	  if(execl("./treasure_hunt","treasure_hunt","--view",hunt_name,input1,input2,NULL)==-1){
	    perror(NULL);
	    exit(-1);
	  }
	}
      }
      int status;
      if(waitpid(command_pid,&status,0)==-1){
	perror(NULL);
	exit(-1);
      }
      if(WIFEXITED(status) && WEXITSTATUS(status)!=0) printf("Encountered an error while running this command (EXIT CODE: %d)\n",WEXITSTATUS(status));
      if(kill(getppid(),SIGUSR2)!=0){
	perror(NULL);
	exit(-1);
      }
     break;
    }
  }
}

void terminate(int sig){
  /*
    sends a signal back to the parent so that it knows the child is terminating
  */
  if(kill(getppid(),SIGUSR2)!=0){
    perror(NULL);
    exit(-1);
  }
  sleep(5);
  exit(0);
}

void setup_signals(){
  /*
    sets up signal handlers
  */
  struct sigaction sa;
  if(sigemptyset(&sa.sa_mask)==-1){
    perror(NULL);
    exit(-1);
  }
  sa.sa_sigaction = run_command;
  sa.sa_flags = SA_RESTART|SA_SIGINFO;//restarts any interrupted system calls
  if (sigaction(SIGUSR1, &sa, NULL) == -1) {
    perror(NULL);
    exit(-1);
  }
  
  if(sigemptyset(&sa.sa_mask)==-1){
    perror(NULL);
    exit(-1);
  }
  sa.sa_handler = terminate;
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGUSR2, &sa, NULL) == -1) {
    perror(NULL);
    exit(-1);
  }
}
int main(){
  setup_signals();
  if(kill(getppid(),SIGUSR1)!=0){//sends signal to parent that it started successfully
    perror(NULL);
    exit(-1);
  }
  while(1);
  return 0;
}
