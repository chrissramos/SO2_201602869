#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
void main(){

    pid_t pid = fork(); //Se realiza el fork
    
    if(pid == -1){
        perror("fork");
        exit(1);
    } else if(pid == 0) {
        printf("Soy el proceso hijo\n");
    }else {
        printf("Soy el proceso padre\n");
    }
}