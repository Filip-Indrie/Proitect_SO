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
      char hunt_name[100];
      printf("Hunt name: ");
      if(fgets(hunt_name,100,stdin)==NULL){
	printf("Error while reading input\n");
	exit(-1);
      }
      printf("\n");
      if(hunt_name[strlen(hunt_name)-1]!='\n') clear_buffer();
      hunt_name[strlen(hunt_name)-1]='\0';
      
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
      char hunt_name[100];
      printf("Hunt name: ");
      if(fgets(hunt_name,100,stdin)==NULL){
	printf("Error while reading input\n");
	exit(-1);
      }
      if(hunt_name[strlen(hunt_name)-1]!='\n') clear_buffer();
      hunt_name[strlen(hunt_name)-1]='\0';

      int got_treasure_id=0,got_user_name=0;
      
      char treasure_id[20];
      printf("Treasure ID: ");
      if(fgets(treasure_id,20,stdin)==NULL){
	printf("Error while reading input\n");
	exit(-1);
      }
      if(treasure_id[strlen(treasure_id)-1]!='\n') clear_buffer();
      treasure_id[strlen(treasure_id)-1]='\0';
      got_treasure_id=(strlen(treasure_id)!=0);
      
      char user_name[100];
      printf("User name: ");
      if(fgets(user_name,100,stdin)==NULL){
	printf("Error while reading input\n");
	exit(-1);
      }
      if(user_name[strlen(user_name)-1]!='\n') clear_buffer();
      user_name[strlen(user_name)-1]='\0';
      got_user_name=(strlen(user_name)!=0);
      printf("\n");
      
      int command_pid;
      if((command_pid=fork())<0){
	perror(NULL);
	exit(-1);
      }
      if(command_pid==0){
	if(got_treasure_id && got_user_name){
	  if(execl("./treasure_hunt","treasure_hunt","--view",hunt_name,treasure_id,user_name,NULL)==-1){
	    perror(NULL);
	    exit(-1);
	  }
	}
	if(got_treasure_id){
	  if(execl("./treasure_hunt","treasure_hunt","--view",hunt_name,treasure_id,NULL)==-1){
	    perror(NULL);
	    exit(-1);
	  }
	}
	if(got_user_name){
	  if(execl("./treasure_hunt","treasure_hunt","--view",hunt_name,user_name,NULL)==-1){
	    perror(NULL);
	    exit(-1);
	  }
	}
	printf("Invalid input\n");
	exit(-1);
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
