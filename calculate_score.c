#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "treasure_struct.h"

#define CHUNK 10

typedef struct{
  char user_name[NAME_LENGTH];
  int score;
}score_t;

int read_treasure_from_file(treasure_t *treasure){
  /*
    returns 1 if all data was read and
    0 if read data is incomplete or
    an error occurred
  */
  int bytes_read;
  if((bytes_read=read(0,treasure,sizeof(treasure_t)))==-1){
    perror(NULL);
    exit(-1);
  }
  if(bytes_read!=sizeof(treasure_t)) return 0;
  return 1;
}

int found_in_list(score_t *scores,char *user_name,int true_size){
  /*
    returns the position at which the user name is located and -1 if user name is not found
  */
  for(int i=0;i<true_size;++i){
    if(strcmp(user_name,scores[i].user_name)==0) return i;
  }
  return -1;
}

int main(){
  int size=0,true_size=0;
  score_t *scores;
  treasure_t treasure;
  while(read_treasure_from_file(&treasure)){
    if(size==true_size){
      size+=CHUNK;
      if((scores=realloc(scores,size*sizeof(treasure_t)))==NULL){
	perror(NULL);
	exit(-1);
      }
    }
    int location=found_in_list(scores,treasure.user_name,true_size);
      if(location!=-1){
	scores[location].score+=treasure.value;
      }
      else{
	strcpy(scores[true_size].user_name,treasure.user_name);
	scores[true_size].score=treasure.value;
	++true_size;
      }
  }
  for(int i=0;i<true_size;++i){
    printf("\tUser Name: %s\n\tScore: %d\n\n",scores[i].user_name,scores[i].score);
  }
  free(scores);
  return 0;
}
