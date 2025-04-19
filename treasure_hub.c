#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

int monitor_status=0;//1 if running and 0 if not running
int accepting_commands=1;//1 if accepting and 0 if rejecting

void print_commands(){
  /*
    prints command list
  */
  printf("Commands:\n");
  printf("\t1 - start monitor\n");
  printf("\t2 - list hunts\n");
  printf("\t3 - list treasures\n");
  printf("\t4 - view treasure\n");
  printf("\t5 - stop monitor\n");
  printf("\t6 - exit\n");
}

void monitor_stopped(int sig){
  /*
    handles SIGCHLD signal
  */
  monitor_status=0;
  int ret_val,status;
  while ((ret_val=waitpid(-1, &status, WNOHANG))>0){//acknowledging terminated children
    if(WIFEXITED(status)) printf("Monitor stopped with exit status %d\n",WEXITSTATUS(status));
  }
  if(ret_val==-1 && errno!=ECHILD){
    perror(NULL);
    exit(-1);
  }
  accepting_commands=1;
}

void monitor_started(int sig){
  /*
    handles SIGUSR1 signal (child sends SIGUSR1 when it started successfully)
  */
  monitor_status=1;
  printf("Monitor started\n");
}

void do_nothing(){
  //empty on purpose
}

void setup_signals(){
  /*
    sets up signal handlers
  */
  struct sigaction sa;
  //sets the set of blocked signals to 0 (no signals will be blocked)
  if(sigemptyset(&sa.sa_mask)==-1){
    perror(NULL);
    exit(-1);
  }
  sa.sa_handler = monitor_started;
  sa.sa_flags = SA_RESTART;//restarts any interrupted system calls
  if(sigaction(SIGUSR1, &sa, NULL) == -1){//child will send SIGUSR1 when it started succesfully
    perror(NULL);
    exit(-1);
  }

  if(sigemptyset(&sa.sa_mask)==-1){
    perror(NULL);
    exit(-1);
  }
  sa.sa_handler = do_nothing;
  sa.sa_flags = SA_RESTART;
  if(sigaction(SIGUSR2, &sa, NULL) == -1){//child will send SIGUSR2 when it completed/acknowledged its task
    perror(NULL);
    exit(-1);
  }
  
  if(sigemptyset(&sa.sa_mask)==-1){
    perror(NULL);
    exit(-1);
  }
  sa.sa_handler = monitor_stopped;
  sa.sa_flags = SA_RESTART;
  if(sigaction(SIGCHLD, &sa, NULL) == -1){
    perror(NULL);
    exit(-1);
  }
}

void clear_buffer(){
  /*
    clears buffer
  */
  char buffer[100];
  while(fgets(buffer,100,stdin)!=NULL){
    if(buffer[strlen(buffer)-1]=='\n') return;
  }
}

void start_monitor(int *monitor_pid){
  /*
    starts a new process for the treasure_monitor
  */
  if(((*monitor_pid)=fork())<0){
    perror(NULL);
    exit(-1);
  }
  if(*monitor_pid==0){
    //child process
    if(execl("./monitor","monitor",NULL)==-1){
      perror(NULL);
      exit(-1);
    }
  }
}

int main(){
  int monitor_pid;
  setup_signals();
  while(1){
    print_commands();
    char buffer[3],*endptr=NULL;
    if(fgets(buffer,3,stdin)==NULL){
      printf("Error while reading option\n");
      exit(-1);
    }
    if(!accepting_commands){
      printf("Hub is currently not accepting commands\n");
      if(buffer[strlen(buffer)-1]!='\n') clear_buffer();
      continue;
    }
    int command=(int)strtol(buffer,&endptr,10);
    if(*endptr!='\n' || endptr==buffer){
      printf("Invalid input\n");
      if(buffer[strlen(buffer)-1]!='\n') clear_buffer();
      continue;
    }
    switch(command){
      case 1:{
        if(!monitor_status){
	  start_monitor(&monitor_pid);
	  pause();
	}
	else printf("Monitor alreeady running (PID: %d)\n",monitor_pid);
        break;
      }
      case 2:{
	if(monitor_status){
	  sigqueue(monitor_pid,SIGUSR1,(union sigval)2);
	  pause();
	}
	else printf("Monitor not running\n");
        break;
      }
      case 3:{
        if(monitor_status){
	  sigqueue(monitor_pid,SIGUSR1,(union sigval)3);
	  pause();
	}
	else printf("Monitor not running\n");
        break;
      }
      case 4:{
        if(monitor_status){
	  sigqueue(monitor_pid,SIGUSR1,(union sigval)4);
	  pause();
	}
	else printf("Monitor not running\n");
        break;
      }
      case 5:{
        if(monitor_status){
	  if(kill(monitor_pid,SIGUSR2)!=0){
	    perror(NULL);
	    exit(-1);
	  }
	  pause();
	  printf("Stopping monitor\n");
	  accepting_commands=0;
	}
	else{
	  printf("Monitor not running\n");
	}
        break;
      }
      case 6:{
	if(monitor_status) printf("Monitor still running (PID: %d)\n",monitor_pid);
	else return 0;
	break;
      }
      default:{
        printf("Invalid command\n");
        break;
      }
    }
  }
  return 0;
}
