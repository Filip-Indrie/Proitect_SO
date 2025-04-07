#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#define NAME_LENGTH 50
#define CLUE_LENGTH 100

typedef struct{
  int treasure_id,value;
  float latitude,longitude;
  char user_name[NAME_LENGTH],clue[CLUE_LENGTH];
}treasure_t;

size_t treasure_size(){
  /*
    return the size of treasure_t
    (didn't use sizeof(treasure_t) because that also counts the paddings)
  */
  treasure_t treasure;
  return sizeof(treasure.treasure_id)+sizeof(treasure.value)+sizeof(treasure.user_name)+sizeof(treasure.clue)+sizeof(treasure.latitude)+sizeof(treasure.longitude);
}

void print_usage_info(){
  printf("Usage: ./treasure_hunt <operation> <arg1> <arg2> ...\n");
  printf("Operations:\n");
  printf("\t--add <hunt_id> --> adds a new treasure to the specified hunt\n");
  printf("\t--list <hunt_id> --> lists all treasures in the specified hunt\n");
  printf("\t--view <hunt_id> <treasure_id> --> lists the details of the specified treasure in the specified hunt\n");
  printf("\t--remove_treasure <hunt_id> <treasure_id> --> removes the specified treasure from the specified hunt\n");
  printf("\t--remove_hunt <hunt_id> --> removes the specified hunt\n");
}

int read_treasure_from_stdin(treasure_t *treasure){
  /*
    Reads treasure input
    Return 1 if all inputs are valid and 0 otherwise
  */
  char buffer[20],*endptr=NULL;
  
  printf("Treasure ID: ");
  /*
    reads to a string
  */
  if(fgets(buffer,20,stdin)==NULL) return 0;
  /*
    cheks if the input is valid
    (we can convert the entire string into an int)
  */
  treasure->treasure_id=(int)strtol(buffer,&endptr,10);
  /*
    if the first invalid char is \n then
    the input is valid, otherwise it isn't
  */
  if(*endptr!='\n' || endptr==buffer) return 0;

  endptr=NULL;
  printf("Value: ");
  if(fgets(buffer,20,stdin)==NULL) return 0;
  treasure->value=(int)strtol(buffer,&endptr,10);
  if(*endptr!='\n' || endptr==buffer) return 0;
  
  endptr=NULL;
  printf("User Name: ");
  if(fgets(treasure->user_name,NAME_LENGTH,stdin)==NULL) return 0;
  treasure->user_name[strlen(treasure->user_name)-1]='\0';

  endptr=NULL;
  printf("Clue: ");
  if(fgets(treasure->clue,CLUE_LENGTH,stdin)==NULL) return 0;
  treasure->clue[strlen(treasure->clue)-1]='\0';

  endptr=NULL;
  printf("Latitude: ");
  if(fgets(buffer,20,stdin)==NULL) return 0;
  treasure->latitude=strtof(buffer,&endptr);
  if(*endptr!='\n' || endptr==buffer) return 0;

  endptr=NULL;
  printf("Longitude: ");
  if(fgets(buffer,20,stdin)==NULL) return 0;
  treasure->longitude=strtof(buffer,&endptr);
  if(*endptr!='\n' || endptr==buffer) return 0;
  
  return 1;
}

void write_treasure_to_file(int treasure_file_desc,treasure_t treasure){
  /*
    writes data to file
  */
  if(write(treasure_file_desc,&(treasure.treasure_id),sizeof(treasure.treasure_id))==-1){
    perror(NULL);
    if(close(treasure_file_desc)!=0){
      perror(NULL);
      exit(141);
    }
    exit(170);
  }

  if(write(treasure_file_desc,&(treasure.value),sizeof(treasure.value))==-1){
    perror(NULL);
    if(close(treasure_file_desc)!=0){
      perror(NULL);
      exit(141);
    }
    exit(171);
  }

  int user_name_length=strlen(treasure.user_name);
  if(write(treasure_file_desc,treasure.user_name,user_name_length)==-1){
    perror(NULL);
    if(close(treasure_file_desc)!=0){
      perror(NULL);
      exit(141);
    }
    exit(173);
  }
  /*
    adds padding of 0 to make the user name size in file consistent
  */
  int zero=0;
  for(int i=0;i<NAME_LENGTH-user_name_length;++i){
    if(write(treasure_file_desc,&zero,1)==-1){
      perror(NULL);
      if(close(treasure_file_desc)!=0){
	perror(NULL);
	exit(141);
      }
      exit(174);
    }
  }

  int clue_length=strlen(treasure.clue);
  if(write(treasure_file_desc,treasure.clue,clue_length)==-1){
    perror(NULL);
    if(close(treasure_file_desc)!=0){
      perror(NULL);
      exit(141);
    }
    exit(176);
  }
  /*
    adds padding of 0 to make the clue size in file consistent
  */
  for(int i=0;i<CLUE_LENGTH-clue_length;++i){
    if(write(treasure_file_desc,&zero,1)==-1){
      perror(NULL);
      if(close(treasure_file_desc)!=0){
	perror(NULL);
	exit(141);
      }
      exit(177);
    }
  }

  if(write(treasure_file_desc,&(treasure.latitude),sizeof(treasure.latitude))==-1){
    perror(NULL);
    if(close(treasure_file_desc)!=0){
      perror(NULL);
      exit(141);
    }
    exit(178);
  }

  if(write(treasure_file_desc,&(treasure.longitude),sizeof(treasure.longitude))==-1){
    perror(NULL);
    if(close(treasure_file_desc)!=0){
      perror(NULL);
      exit(141);
    }
    exit(179);
  }
}

void log_operation(int log_file_desc,char *message){
  /*
    takes the time at which the operation is performed
  */
  time_t time_of_operation;
  if((time_of_operation=time(NULL))==-1){
    perror(NULL);
    exit(-1);
  }
  /*
    converts it to calendar time and write to log file
  */
  char *calendar_time=ctime(&time_of_operation);
  if(write(log_file_desc,calendar_time,strlen(calendar_time))==-1){
    perror(NULL);
    exit(-1);
  }
  /*
    writes message to log file
  */
  if(write(log_file_desc,message,strlen(message))==-1){
    perror(NULL);
    exit(-1);
  }
}

void log_add(int log_file_desc,treasure_t treasure){
  /*
    creates and appropriate message and calls log_operation
  */
  char message[40];
  sprintf(message,"Added treasure with ID: %d\n\n",treasure.treasure_id);
  log_operation(log_file_desc,message);
}

void log_list(int log_file_desc){
  /*
    creates and appropriate message and calls log_operation
  */
  log_operation(log_file_desc,"Listed hunt\n\n");
}

void log_view(int log_file_desc,treasure_t treasure){
  /*
    creates and appropriate message and calls log_operation
  */
  char message[40];
  sprintf(message,"Viewed treasure with ID: %d\n\n",treasure.treasure_id);
  log_operation(log_file_desc,message);
}

void log_remove_treasure(int log_file_desc,treasure_t treasure){
  /*
    creates and appropriate message and calls log_operation
  */
  char message[40];
  sprintf(message,"Removed treasure with ID: %d\n\n",treasure.treasure_id);
  log_operation(log_file_desc,message);
}

void add(char hunt_id[]){
  /*
    reads treasure data
  */
  treasure_t treasure;
  int valid;
  do{
    valid=read_treasure_from_stdin(&treasure);
    if(!valid) printf("INVALID INPUT\n");
  }while(valid==0);
  /*
    gets the absolute path of program direcotry
  */
  char path[PATH_MAX];
  if(getcwd(path,PATH_MAX)==NULL){
    perror(NULL);
    exit(11);
  }
  /*
    opens program directory
  */
  DIR *current_dir;
  if((current_dir=opendir(path))==NULL){
    perror(NULL);
    exit(12);
  }
  /*
    creates the path for the treasures file
  */
  strcat(path,"/");
  strcat(path,hunt_id);
  strcat(path,"/treasures.bin");
  /*
    parses through the directory
  */
  struct dirent *dir_entry;
  errno=0;
  while((dir_entry=readdir(current_dir))!=NULL){
    /*
      checks if the current file is a directory and
      if it has the same as hunt_id parameter
    */
    if(dir_entry->d_type==DT_DIR && strcmp(dir_entry->d_name,hunt_id)==0){
      /*
	opens file
      */
      int treasure_file_desc;
      if((treasure_file_desc=open(path,O_CREAT|O_WRONLY|O_APPEND,0644))==-1){
	perror(NULL);
	exit(131);
      }
      /*
	creates path for log file
      */
      strcpy(strrchr(path,'/'),"/logged_hunt.txt");
      /*
	opens log file
      */
      int log_file_desc;
      if((log_file_desc=open(path,O_CREAT|O_WRONLY|O_APPEND,0644))==-1){
	perror(NULL);
	exit(132);
      }
      /*
	writes data to file
      */
      write_treasure_to_file(treasure_file_desc,treasure);
      /*
	logs operation
      */
      log_add(log_file_desc,treasure);
      /*
	closes files
      */
      if(close(treasure_file_desc)!=0 || close(log_file_desc)!=0){
	perror(NULL);
	exit(141);
      }
      /*
	closes directory
      */
      if(closedir(current_dir)==-1){
	perror(NULL);
	exit(-1);
      }
      return;
    }
  }
  if(errno){
    perror(NULL);
    exit(15);
  }
  /*
    if theres no such directory, create one
  */
  if(mkdir(hunt_id,0755)!=0){
    perror(NULL);
    exit(16);
  }
  /*
    creates and opens file
  */
  int treasure_file_desc;
  if((treasure_file_desc=open(path,O_CREAT|O_WRONLY,0644))==-1){
    perror(NULL);
    exit(132);
  }
  /*
    creates path for log file
  */
  strcpy(strrchr(path,'/'),"/logged_hunt.txt");
  /*
    creates and opens log file
  */
  int log_file_desc;
  if((log_file_desc=open(path,O_CREAT|O_WRONLY,0644))==-1){
    perror(NULL);
    exit(132);
  }
  /*
    creates symbolic link to log file
  */
  char link_path[PATH_MAX]="logged_hunt-";
  strcat(link_path,hunt_id);
  if(symlink(path,link_path)==-1){
    printf("%s\n%s\n",path,link_path);
    perror(NULL);
    exit(-1);
  }
  /*
    wrote data to file
  */
  write_treasure_to_file(treasure_file_desc,treasure);
  /*
    logs operation
  */
  log_add(log_file_desc,treasure);
  /*
    closes files
  */
  if(close(treasure_file_desc)!=0 || close(log_file_desc)!=0){
    perror(NULL);
    exit(142);
  }
  /*
    closes directory
  */
  if(closedir(current_dir)==-1){
    perror(NULL);
    exit(-1);
  }
}

void print_treasure(treasure_t treasure){
  /*
    printing treasure data to standard output
  */
  printf("%d\n",treasure.treasure_id);
  printf("%d\n",treasure.value);
  printf("%s\n",treasure.user_name);
  printf("%s\n",treasure.clue);
  printf("%f\n",treasure.latitude);
  printf("%f\n\n",treasure.longitude);
}

int read_treasure_from_file(int treasure_file_desc,treasure_t *treasure){
  /*
    returns 1 if all data was read and
    0 if read data is incomplete or
    an error occurred
  */
  /*
    reading and checking if all bytes were read
  */
  int bytes_read;
  if((bytes_read=read(treasure_file_desc,&(treasure->treasure_id),sizeof(treasure->treasure_id)))==-1){
    perror(NULL);
    exit(240);
  }
  if(bytes_read!=sizeof(treasure->treasure_id)) return 0;
  
  if((bytes_read=read(treasure_file_desc,&(treasure->value),sizeof(treasure->value)))==-1){
    perror(NULL);
    exit(240);
  }
  if(bytes_read!=sizeof(treasure->value)) return 0;

  if((bytes_read=read(treasure_file_desc,treasure->user_name,NAME_LENGTH))==-1){
    perror(NULL);
    exit(240);
  }
  if(bytes_read!=NAME_LENGTH) return 0;
  
  if((bytes_read=read(treasure_file_desc,treasure->clue,CLUE_LENGTH))==-1){
    perror(NULL);
    exit(240);
  }
  if(bytes_read!=CLUE_LENGTH) return 0;
  
  if((bytes_read=read(treasure_file_desc,&(treasure->latitude),sizeof(treasure->latitude)))==-1){
    perror(NULL);
    exit(240);
  }
  if(bytes_read!=sizeof(treasure->latitude)) return 0;
  
  if((bytes_read=read(treasure_file_desc,&(treasure->longitude),sizeof(treasure->longitude)))==-1){
    perror(NULL);
    exit(240);
  }
  if(bytes_read!=sizeof(treasure->longitude)) return 0;
  
  return 1;
}

void list(char hunt_id[]){
  /*
    getting the absolute path of program direcotry
  */
  char path[PATH_MAX];
  if(getcwd(path,PATH_MAX)==NULL){
    perror(NULL);
    exit(21);
  }
  /*
    opening program directory
  */
  DIR *current_dir;
  if((current_dir=opendir(path))==NULL){
    perror(NULL);
    exit(22);
  }
  /*
    parsing through the directory
  */
  struct dirent *dir_entry;
  errno=0;
  while((dir_entry=readdir(current_dir))!=NULL){
    if(dir_entry->d_type==DT_DIR && strcmp(dir_entry->d_name,hunt_id)==0){
      /*
	creates the path for the hunt directory
      */
      strcat(path,"/");
      strcat(path,hunt_id);
      /*
	gets status info for hunt directory
      */
      struct stat hunt_stat;
      if(stat(path,&hunt_stat)==-1){
	perror(NULL);
	exit(20);
      }
      /*
	creates the path fot the treasures file
      */
      strcat(path,"/treasures.bin");
      /*
	gets status info for treasures file
      */
      struct stat treasures_stat;
      if(stat(path,&treasures_stat)==-1){
	perror(NULL);
	exit(20);
      }
      /*
	prints time of last modification of the treasures file
      */
      printf("Name: %s\n",hunt_id);
      printf("Total size: %ld\n",hunt_stat.st_size);
      printf("Treasures file last modification time: %s===============================\n",ctime(&(treasures_stat.st_mtim.tv_sec)));
      /*
	opens file
      */
      int treasure_file_desc;
      if((treasure_file_desc=open(path,O_RDONLY))==-1){
	perror(NULL);
	exit(131);
      }
      /*
	parses through the treasures file
	and prints to stdout its content
      */
      treasure_t treasure;
      while(read_treasure_from_file(treasure_file_desc,&treasure)){
	print_treasure(treasure);
      }
      /*
	creates path for log file
      */
      strcpy(strrchr(path,'/'),"/logged_hunt.txt");
      /*
	opens log file
      */
      int log_file_desc;
      if((log_file_desc=open(path,O_WRONLY|O_APPEND,0644))==-1){
	perror(NULL);
	exit(132);
      }
      /*
	logs operation
      */
      log_list(log_file_desc);
      /*
	closes files
      */
      if(close(treasure_file_desc)!=0 || close(log_file_desc)!=0){
	perror(NULL);
	exit(142);
      }
      /*
	closes directory
      */
      if(closedir(current_dir)==-1){
	perror(NULL);
	exit(-1);
      }
      return;
    }
  }
  if(errno){
    perror(NULL);
    exit(25);
  }
  printf("Hunt with id %s does not exist\n",hunt_id);
  /*
    closes directory
  */
  if(closedir(current_dir)==-1){
    perror(NULL);
    exit(-1);
  }
}

void view(char hunt_id[],int treasure_id){
  /*
    getting the absolute path of program direcotry
  */
  char path[PATH_MAX];
  if(getcwd(path,PATH_MAX)==NULL){
    perror(NULL);
    exit(21);
  }
  /*
    opening program directory
  */
  DIR *current_dir;
  if((current_dir=opendir(path))==NULL){
    perror(NULL);
    exit(22);
  }
  /*
    parsing through the directory
  */
  struct dirent *dir_entry;
  errno=0;
  while((dir_entry=readdir(current_dir))!=NULL){
    if(dir_entry->d_type==DT_DIR && strcmp(dir_entry->d_name,hunt_id)==0){
      /*
	creates the path for the treasures file
      */
      strcat(path,"/");
      strcat(path,hunt_id);
      strcat(path,"/treasures.bin");
      /*
	opens file
      */
      int treasure_file_desc;
      if((treasure_file_desc=open(path,O_RDONLY))==-1){
	perror(NULL);
	exit(131);
      }
      /*
	parses through the treasures and
	prints the treasure with treasure_id
	equal to the one received through
	treasure_id parameter
      */
      treasure_t treasure;
      while(read_treasure_from_file(treasure_file_desc,&treasure)){
	if(treasure.treasure_id==treasure_id){
	  print_treasure(treasure);
	  /*
	    creates path for log file
	  */
	  strcpy(strrchr(path,'/'),"/logged_hunt.txt");
	  /*
	    opens log file
	  */
	  int log_file_desc;
	  if((log_file_desc=open(path,O_WRONLY|O_APPEND,0644))==-1){
	    perror(NULL);
	    exit(132);
	  }
	  /*
	    logs operation
	  */
	  log_view(log_file_desc,treasure);
	  /*
	    closes files
	  */
	  if(close(log_file_desc)!=0 || close(treasure_file_desc)!=0){
	    perror(NULL);
	    exit(142);
	  }
	  /*
	    closes directory
	  */
	  if(closedir(current_dir)==-1){
	    perror(NULL);
	    exit(-1);
	  }
	  return;
	}
      }
      printf("Treasure with id %d does not exist\n",treasure_id);
      /*
	closes file
      */
      if(close(treasure_file_desc)!=0){
	perror(NULL);
	exit(142);
      }
      /*
	closes directory
      */
      if(closedir(current_dir)==-1){
	perror(NULL);
	exit(-1);
      }
      return;
    }
  }
  if(errno){
    perror(NULL);
    exit(25);
  }
  printf("Hunt with id %s does not exist\n",hunt_id);
  /*
    closes directory
  */
  if(closedir(current_dir)==-1){
    perror(NULL);
    exit(-1);
  }
}

void remove_treasure(char hunt_id[],int treasure_id){
  /*
    getting the absolute path of program direcotry
  */
  char path[PATH_MAX];
  if(getcwd(path,PATH_MAX)==NULL){
    perror(NULL);
    exit(21);
  }
  /*
    opening program directory
  */
  DIR *current_dir;
  if((current_dir=opendir(path))==NULL){
    perror(NULL);
    exit(22);
  }
  /*
    parsing through the directory
  */
  struct dirent *dir_entry;
  errno=0;
  while((dir_entry=readdir(current_dir))!=NULL){
    if(dir_entry->d_type==DT_DIR && strcmp(dir_entry->d_name,hunt_id)==0){
      /*
	creates the path for the treasures file
      */
      strcat(path,"/");
      strcat(path,hunt_id);
      strcat(path,"/treasures.bin");
      /*
	opens file
      */
      int treasure_file_desc;
      if((treasure_file_desc=open(path,O_RDWR))==-1){
	perror(NULL);
	exit(131);
      }
      /*
	parses through the treasures and
	prints the treasure with treasure_id
	equal to the one received through
	treasure_id parameter
      */
      int bytes_read=0,found=0;
      treasure_t treasure;
      while(read_treasure_from_file(treasure_file_desc,&treasure)){
	bytes_read+=treasure_size();
	/*
	  checks if it found the treasure
	*/
	if(treasure_id==treasure.treasure_id){
	  found=1;
	}
	if(found){
	  /*
	    overwrites the current treasure with the next one
	    (in the long run it eliminates the found treasure)
	  */
	  treasure_t temp;
	  if(read_treasure_from_file(treasure_file_desc,&temp)){
	    lseek(treasure_file_desc,-2*treasure_size(),SEEK_CUR);
	    write_treasure_to_file(treasure_file_desc,temp);
	  }
	}
      }
      if(found){
	bytes_read-=treasure_size();
	ftruncate(treasure_file_desc,bytes_read);
	/*
	  closes file
	*/
	if(close(treasure_file_desc)!=0){
	  perror(NULL);
	  exit(142);
	}
	/*
	  creates path for log file
	*/
	strcpy(strrchr(path,'/'),"/logged_hunt.txt");
	/*
	  opens log file
	*/
	int log_file_desc;
	if((log_file_desc=open(path,O_WRONLY|O_APPEND,0644))==-1){
	  perror(NULL);
	  exit(132);
	}
	/*
	  logs operation
	*/
	log_remove_treasure(log_file_desc,treasure);
	/*
	  closes log file
	*/
	if(close(log_file_desc)!=0){
	  printf("aici1\n");
	  perror(NULL);
	  exit(142);
	}
	/*
	  closes directory
	*/
	if(closedir(current_dir)==-1){
	  perror(NULL);
	  exit(-1);
	}
	return;
      }
      printf("Treasure with id %d does not exist\n",treasure_id);
      /*
	closes file
      */
      if(close(treasure_file_desc)!=0){
	perror(NULL);
	exit(142);
      }
      /*
	closes directory
      */
      if(closedir(current_dir)==-1){
	perror(NULL);
	exit(-1);
      }
      return;
    }
  }
  if(errno){
    perror(NULL);
    exit(25);
  }
  printf("Hunt with id %s does not exist\n",hunt_id);
  /*
    closes directory
  */
  if(closedir(current_dir)==-1){
    perror(NULL);
    exit(-1);
  }
}

void remove_hunt(char hunt_id[]){
  /*
    getting the absolute path of program direcotry
  */
  char path[PATH_MAX];
  if(getcwd(path,PATH_MAX)==NULL){
    perror(NULL);
    exit(21);
  }
  /*
    opening program directory
  */
  DIR *current_dir;
  if((current_dir=opendir(path))==NULL){
    perror(NULL);
    exit(22);
  }
  /*
    parsing through program directory
  */
  struct dirent *current_dir_entry;
  errno=0;
  while((current_dir_entry=readdir(current_dir))!=NULL){
    /*
      checks if the current file is a directory and
      if it has the same as hunt_id parameter
    */
    if(current_dir_entry->d_type==DT_DIR && strcmp(current_dir_entry->d_name,hunt_id)==0){
      /*
	getting symlinks path
      */
      strcat(path,"/logged_hunt-");
      strcat(path,hunt_id);
      /*
	removing symlink
      */
      if(remove(path)==-1){
	perror(NULL);
	exit(-1);
      }
      /*
	getting hunt directory path
      */
      strcpy(strrchr(path,'/')+1,hunt_id);
      /*
	opening hunt directory
      */
      DIR *hunt_dir;
      if((hunt_dir=opendir(path))==NULL){
	perror(NULL);
	exit(22);
      }
      /*
	parsing through hunt directory
      */
      struct dirent *hunt_dir_entry;
      errno=0;
      while((hunt_dir_entry=readdir(hunt_dir))!=NULL){
	/*
	  getting the path for each file in directory and removing it
	*/
	if(strcmp(hunt_dir_entry->d_name,".")!=0 && strcmp(hunt_dir_entry->d_name,"..")!=0){
	  char file_path[PATH_MAX];
	  strcpy(file_path,path);
	  strcat(file_path,"/");
	  strcat(file_path,hunt_dir_entry->d_name);
	  if(remove(file_path)==-1){
	    perror(NULL);
	    exit(-1);
	  }
	}
      }
      if(errno){
	perror(NULL);
	exit(25);
      }
      /*
	closes hunt directory
      */
      if(closedir(hunt_dir)==-1){
	perror(NULL);
	exit(-1);
      }
      /*
	removing direcotry
      */
      if(remove(path)==-1){
	perror(NULL);
	exit(-1);
      }
      return;
    }
  }
  if(errno){
    perror(NULL);
    exit(25);
  }
  printf("Hunt with id %s does not exist\n",hunt_id);
  /*
    closes program directory
  */
  if(closedir(current_dir)==-1){
    perror(NULL);
    exit(-1);
  }
}

int main(int argc,char **argv){
  if(argc==2 && strcmp(argv[1],"--help")==0){
    print_usage_info();
  }
  else if(argc==3 && strcmp(argv[1],"--add")==0){
    add(argv[2]);
  }
  else if(argc==3 && strcmp(argv[1],"--list")==0){
    list(argv[2]);
  }
  else if(argc==4 && strcmp(argv[1],"--view")==0){
    int treasure_id;
    char *endptr=NULL;
    /*
      checks if the received treasure id is valid
    */
    treasure_id=(int)strtol(argv[3],&endptr,10);
    if(*endptr!='\0'){
      printf("Invalid treasue ID\n");
      return 0;
    }
    view(argv[2],treasure_id);
  }
  else if(argc==4 && strcmp(argv[1],"--remove_treasure")==0){
    int treasure_id;
    char *endptr=NULL;
    /*
      checks if the received treasure id is valid
    */
    treasure_id=(int)strtol(argv[3],&endptr,10);
    if(*endptr!='\0'){
      printf("Invalid treasue ID\n");
      return 0;
    }
    remove_treasure(argv[2],treasure_id);
  }
  else if(argc==3 && strcmp(argv[1],"--remove_hunt")==0){
    remove_hunt(argv[2]);
  }
  else{
    printf("Incorrect format\n");
    exit(-1);
  }
  return 0;
}
