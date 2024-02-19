#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

// Variables globales para contar las llamadas al sistema
int total_syscalls = 0;
int read_syscalls = 0;
int write_syscalls = 0;
int seek_syscalls = 0;
pid_t pid1, pid2;

// Manejador de señal para SIGINT (Ctrl + C)
void sigint_handler(int signum) {
    printf("\n - - Número total de llamadas al sistema realizadas por los procesos hijo: %d\n", total_syscalls);
    printf("Número de llamadas al sistema por tipo:\n");
    printf("  Read: %d\n", read_syscalls);
    printf("  Write: %d\n", write_syscalls);
    printf("  Seek: %d\n", seek_syscalls);
    kill(pid1, SIGTERM);
    kill(pid2, SIGTERM);
    exit(0);
}

int main() {
    // Asignar el manejador de señal para SIGINT
    signal(SIGINT, sigint_handler);

    // Crear el archivo de log
    FILE *logfile = fopen("syscalls.log", "w");
    if (logfile == NULL) {
        perror("Error al abrir el archivo de log");
        exit(1);
    }

    // Crear los procesos hijos
    //pid_t pid1, pid2;
    pid1 = fork();
    if (pid1 == 0) {
        // Código del primer hijo
        execl("./child", "child", NULL);
        perror("Error al ejecutar el proceso hijo 1");
        exit(1);
    } else if (pid1 < 0) {
        perror("Error al crear el proceso hijo 1");
        exit(1);
    }

    pid2 = fork();
    if (pid2 == 0) {
        // Código del segundo hijo
        execl("./child", "child", NULL);
        perror("Error al ejecutar el proceso hijo 2");
        exit(1);
    } else if (pid2 < 0) {
        perror("Error al crear el proceso hijo 2");
        exit(1);
    }

    // Ejecutar SystemTap dentro del proceso padre
    char command[200];  
   // sprintf(command, "sudo stap -e 'probe syscall.read { printf(\"Read: %d\\n\", pid()) }, syscall.write { printf(\"Write: %d\\n\", pid()) }' > syscalls.log", getpid(), getpid());
    //sprintf(command, "sudo stap -e 'probe syscall.read { printf(\"Read: %d\\n\", pid()) }' -e 'probe syscall.write { printf(\"Write: %d\\n\", pid()) }' > syscalls.log", getpid(), getpid());
    //sprintf(command, "sudo stap -e 'probe syscall.read { printf(\"Read: %d\\n\", pid()); } probe syscall.write { printf(\"Write: %d\\n\", pid()); }' > syscalls.log", getpid(), getpid());
    // ESTE SI JALA sprintf(command, "%s %d %d %s", "sudo stap trace.stp", pid1, pid2, "> syscalls.log");
    sprintf(command, "sudo stap trace.stp %d %d > syscalls.log", pid1, pid2);
    system(command);

    printf("\nsalimos del sistem \n");

    // // Esperar a que los hijos terminen y procesar su salida
    //  int status;
    // waitpid(pid1, &status, 0);
    // read_syscalls += WEXITSTATUS(status);
    //  total_syscalls += WEXITSTATUS(status);
    //  fprintf(logfile, "Proceso %d: %d llamadas al sistema\n", pid1, WEXITSTATUS(status));

    //  waitpid(pid2, &status, 0);
    //  write_syscalls += WEXITSTATUS(status);
    //  total_syscalls += WEXITSTATUS(status);
    //  fprintf(logfile, "Proceso %d: %d llamadas al sistema\n", pid2, WEXITSTATUS(status));

    //  // Cerrar el archivo de log
    //  fclose(logfile);


    // version 2 

    // Esperar a que los hijos terminen
    printf("vamos a esperar a que terminen los hijos");

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    // Analizar el archivo de log de SystemTap para contar las llamadas
    FILE *syscalls_log = fopen("syscalls.log", "r");
    if (syscalls_log == NULL) {
        perror("Error al abrir el archivo de log de llamadas al sistema");
        exit(1);
    }
    printf("vamos a analizar el reporte");

    // Rebobinar el archivo
    fseek(syscalls_log, 0, SEEK_SET);

    char line[100];
    while (fgets(line, sizeof(line), syscalls_log) != NULL) {
        if (strstr(line, "read")) {
            read_syscalls++;
            total_syscalls++;
        } else if (strstr(line, "write")) {
            write_syscalls++;
            total_syscalls++;
        } else if (strstr(line, "seek")) {
            seek_syscalls++;
            total_syscalls++;
        }
    }

    // Cerrar el archivo de log
    fclose(syscalls_log);

    // Imprimir los conteos
    printf("Reporte de Número total de llamadas al sistema realizadas por los procesos hijo: %d\n", total_syscalls);
    printf("Número de llamadas al sistema por tipo:\n");
    printf("  Read: %d\n", read_syscalls);
    printf("  Write: %d\n", write_syscalls);
    printf("  Seek: %d\n", seek_syscalls);

    // Cerrar el archivo de log
    fclose(logfile);

    return 0;
}