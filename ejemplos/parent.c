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
    }
    
     else if(pid == 0){

        /*Se escriben los argumentos para el proceso hijo*/
        char *arg_Ptr[4];
        arg_Ptr[0] = " child.bin";
        arg_Ptr[1] =  NULL;
        arg_Ptr[2] =  NULL;
        arg_Ptr[3] = NULL; // El ultimo indice de argv siempre debe de ser NULL

        /*Se ejecuta el ejecutable del proceso hijo*/
        execv("/home/chriss/Documentos/Sopes2Lab/SO2_201602869/ejemplos/child.bin", arg_Ptr);
    
    }else{
        printf("Soy el proceso padre\n");

       
    }
}