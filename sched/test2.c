#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
 pid_t pid;
 /*fork another porcess*/
 pid = fork();
 if(pid<0) /*error occurred*/
 {
  fprintf(stderr, "Fork Failed");
  exit(-1);
 }
 else if(pid==0) /*child process*/
 {
  sleep(2);
  execlp("/bin/ls","ls",NULL);
  return 0 ;
 }
 else /*parent process*/
 {
  //wait(NULL);
  //wait();
  printf("Child Complete\n");
  exit(0);
 }
}
