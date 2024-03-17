#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define MAX_USERS 400
#define MAX_NAME_LENGTH 50
#define MAX_OPERATIONS 2000
#define CSV_ROW_LENGTH 1000
#define NUM_THREADS 3
#define NUM_OPERATION_THREADS 4

// Estructura para almacenar los datos de un usuario
struct user {
    int account_number;
    char first_name[MAX_NAME_LENGTH];
    double balance;
};

struct user users[MAX_USERS]; // Arreglo para almacenar los usuarios
int total_users = 0; // Contador de usuarios cargados

// Estructura para almacenar los datos de una operación
struct operation {
    int operation_type;
    int account1;
    int account2;
    double amount;
};

struct operation operations[MAX_OPERATIONS]; // Arreglo para almacenar las operaciones
int total_operations = 0; // Contador de operaciones cargadas

pthread_mutex_t mutex_users = PTHREAD_MUTEX_INITIALIZER; // Mutex para controlar el acceso al arreglo de usuarios
pthread_mutex_t mutex_operations = PTHREAD_MUTEX_INITIALIZER; // Mutex para controlar el acceso al arreglo de operaciones

// Estructura para almacenar el resumen de carga de usuarios por hilo
struct thread_summary {
    int thread_id;
    int count;
};

struct thread_summary thread_summaries[NUM_THREADS];

struct thread_operation_summary {
    int thread_id;
    int count;
};


struct thread_operation_summary operation_thread_summaries[NUM_OPERATION_THREADS];


// Estructura para almacenar registros de errores durante la carga
struct error_record {
    int line_number;
    char row[CSV_ROW_LENGTH];
    char error_message[100];
};

struct error_record error_records[MAX_USERS]; // Arreglo para almacenar registros de errores
int total_errors = 0; // Contador de registros de errores


// Estructura para almacenar el resumen de operaciones
struct operation_summary {
    int withdrawals;
    int deposits;
    int transfers;
};

// Función para cargar operaciones masivas desde un archivo CSV en un hilo
void *load_operations(void *filename) {
    char *csv_file = (char *)filename;
    FILE *fp = fopen(csv_file, "r");
    if (fp == NULL) {
        perror("Error al abrir el archivo");
        pthread_exit(NULL);
    }

    char row[CSV_ROW_LENGTH];
    char *token;
    int line_number = 2; // Para mantener el número de línea

    // Se omite la primera fila (encabezados)
    fgets(row, CSV_ROW_LENGTH, fp);

    while (fgets(row, CSV_ROW_LENGTH, fp) != NULL) {
        token = strtok(row, ",");
        int operation_type = atoi(token);

        token = strtok(NULL, ",");
        int account1 = atoi(token);

        token = strtok(NULL, ",");
        int account2 = atoi(token);

        token = strtok(NULL, ",");
        double amount = atof(token);

        // Validar monto negativo
        if (amount < 0) {
            sprintf(error_records[total_errors].error_message, "Monto negativo");
            error_records[total_errors].line_number = line_number;
            strncpy(error_records[total_errors].row, row, CSV_ROW_LENGTH);
            total_errors++;
        } else {
            // Bloquear el acceso al arreglo de operaciones antes de agregar una nueva
            pthread_mutex_lock(&mutex_operations);

            if (total_operations < MAX_OPERATIONS) {
                operations[total_operations].operation_type = operation_type;
                operations[total_operations].account1 = account1;
                operations[total_operations].account2 = account2;
                operations[total_operations].amount = amount;
                total_operations++;
            } else {
                printf("Error: No se pudo cargar la operación %d, límite de operaciones alcanzado.\n", operation_type);
            }

            // Desbloquear el acceso al arreglo de operaciones
            pthread_mutex_unlock(&mutex_operations);
        }

        line_number++;
    }

    fclose(fp);
    pthread_exit(NULL);
}

void generate_user_load_report() {
    // Obtener la fecha y hora actual
    time_t current_time = time(NULL);
    struct tm *local_time = localtime(&current_time);
    char datetime[20];
    strftime(datetime, 20, "%Y_%m_%d-%H_%M_%S", local_time);

    // Crear el nombre del archivo con el formato especificado
    char filename[50];
    sprintf(filename, "carga_%s.log", datetime);

    // Abrir el archivo en modo escritura
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Error al abrir el archivo de reporte");
        return;
    }

    // Escribir el encabezado del reporte
    fprintf(fp, "-----Carga De Usuarios:----\n");
    fprintf(fp, "Fecha: %s\n\n", datetime);

    // Escribir la cantidad de usuarios cargados por cada hilo
    fprintf(fp, "Usuarios Cargados por hilo:\n");
    int total_loaded_users = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        fprintf(fp, "Hilo %d: %d\n", i, thread_summaries[i].count);
        total_loaded_users += thread_summaries[i].count;
    }
    fprintf(fp, "Total: %d\n\n", total_loaded_users);

    // Escribir los registros con errores
    fprintf(fp, "Errores:\n");
    for (int i = 0; i < total_errors; i++) {
        fprintf(fp, "Línea %d: %s - %s\n", error_records[i].line_number, error_records[i].row, error_records[i].error_message);
    }

    // Cerrar el archivo
    fclose(fp);

    printf("Reporte de carga de usuarios generado: %s\n", filename);
}



// Función para generar el reporte de operaciones
void generate_operation_report(struct operation_summary summary) {
    // Obtener la fecha y hora actual
    time_t current_time = time(NULL);
    struct tm *local_time = localtime(&current_time);
    char datetime[20];
    strftime(datetime, 20, "%Y_%m_%d-%H_%M_%S", local_time);

    // Crear el nombre del archivo con el formato especificado
    char filename[50];
    sprintf(filename, "operaciones_%s.log", datetime);

    // Abrir el archivo en modo escritura
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Error al abrir el archivo de reporte de operaciones");
        return;
    }

    // Escribir el encabezado del reporte
    fprintf(fp, "------ Resumen de Operaciones------\n");
    fprintf(fp, "Fecha: %s\n\n", datetime);

    // Escribir el resumen de operaciones
    fprintf(fp, "Operaciones realizadas:\n");
    fprintf(fp, "Retiros: %d\n", summary.withdrawals);
    fprintf(fp, "Depositos: %d\n", summary.deposits);
    fprintf(fp, "Transferencias: %d\n", summary.transfers);
    fprintf(fp, "Total: %d\n\n", summary.withdrawals + summary.deposits + summary.transfers);

    // Escribir los registros con errores
    fprintf(fp, "Errores:\n");
    for (int i = 0; i < total_errors; i++) {
        fprintf(fp, "Linea %d: %s - %s\n", error_records[i].line_number, error_records[i].row, error_records[i].error_message);
    }

    // Cerrar el archivo
    fclose(fp);

    printf("Reporte de operaciones generado: %s\n", filename);
}

// Función para mostrar el menú y obtener la opción del usuario
int show_menu() {
    int option;
    printf("\nMenu:\n");
    printf("1. Operaciones Individuales.\n");
    printf("2. Carga Masiva de Operaciones.\n");
    printf("3. Estado De Cuentas.\n");
    printf("4. Reporte de Carga de Usuarios.\n");
    printf("5. Reporte de Errores.\n");
    printf("6. Salir.\n");
    printf("Ingrese el número de opción deseada: ");
    scanf("%d", &option);
    return option;
}

// Función para imprimir los estados de cuenta en consola
void print_account_states() {
    printf("\nEstados de Cuenta:\n");
    printf("No. Cuenta\tNombre\t\tSaldo\n");
    for (int i = 0; i < total_users; i++) {
        printf("%d\t\t%s\t\t%.2f\n", users[i].account_number, users[i].first_name, users[i].balance);
    }
}
// Función para cargar usuarios desde un archivo CSV en un hilo
void *load_users(void *arg) {
    int thread_id = *((int *)arg); // Identificador del hilo
    char filename_users[] = "usuarios.csv"; // Nombre del archivo CSV con los usuarios

    FILE *fp = fopen(filename_users, "r");
    if (fp == NULL) {
        perror("Error al abrir el archivo");
        pthread_exit(NULL);
    }

    char row[CSV_ROW_LENGTH];
    char *token;

    // Se omite la primera fila (encabezados)
    fgets(row, CSV_ROW_LENGTH, fp);

    int count = 0; // Contador para rastrear el número de usuarios cargados por este hilo

    while (fgets(row, CSV_ROW_LENGTH, fp) != NULL) {
        // Verificar si este hilo debe procesar esta línea
        if (count % NUM_THREADS != thread_id) {
            count++;
            continue;
        }

        token = strtok(row, ",");
        int account_number = atoi(token);

        token = strtok(NULL, ",");
        char first_name[MAX_NAME_LENGTH];
        strncpy(first_name, token, MAX_NAME_LENGTH);

        token = strtok(NULL, ",");
        double balance;
        if (sscanf(token, "%lf", &balance) != 1 || balance < 0) {
            // Agregar el registro con error al reporte de errores
            pthread_mutex_lock(&mutex_users);
            error_records[total_errors].line_number = total_users + 1; // +1 para incluir la fila de encabezados
            strcpy(error_records[total_errors].row, row);
            strcpy(error_records[total_errors].error_message, "Saldo no válido o negativo");
            total_errors++;
            pthread_mutex_unlock(&mutex_users);
            continue; // Omitir el registro con error
        }

        // Bloquear el acceso al arreglo de usuarios antes de agregar uno nuevo
        pthread_mutex_lock(&mutex_users);
        if (total_users < MAX_USERS) {
            users[total_users].account_number = account_number;
            strncpy(users[total_users].first_name, first_name, MAX_NAME_LENGTH);
            users[total_users].balance = balance;
            total_users++;

            // Actualizar el resumen de carga por hilo
            thread_summaries[thread_id].thread_id = thread_id;
            thread_summaries[thread_id].count++;
        } else {
            printf("Error: No se pudo cargar el usuario %d, límite de usuarios alcanzado.\n", account_number);
        }
        // Desbloquear el acceso al arreglo de usuarios
        pthread_mutex_unlock(&mutex_users);

        count++;
    }

    fclose(fp);
    pthread_exit(NULL);
}




int main() {
    // Crear los identificadores de hilo para cargar usuarios
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
    }

    // Cargar usuarios en hilos
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, load_users, (void *)&thread_ids[i]) != 0) {
            perror("Error al crear el hilo de carga de usuarios");
            exit(EXIT_FAILURE);
        }
    }

    // Esperar a que los hilos de carga de usuarios terminen
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Error al unir el hilo de carga de usuarios");
            exit(EXIT_FAILURE);
        }
    }

    // Generar el reporte de carga de usuarios
    generate_user_load_report();

    // Mostrar el menú y obtener la opción del usuario
    int option;
    do {
        option = show_menu();
        // Aquí puedes agregar la lógica para manejar las diferentes opciones del menú
        switch (option) {
            // Casos para las diferentes opciones del menú
            case 1:
                // Operaciones Individuales
                // Aquí puedes agregar la lógica para las operaciones individuales
                break;
            case 2:
                // Carga Masiva de Operaciones
                // Cargar operaciones masivas desde el archivo transacciones.csv
                pthread_t threads_operations;
                if (pthread_create(&threads_operations, NULL, load_operations, (void *)"transacciones.csv") != 0) {
                    perror("Error al crear el hilo de carga de operaciones");
                    exit(EXIT_FAILURE);
                }

                // Esperar a que el hilo de carga de operaciones termine
                if (pthread_join(threads_operations, NULL) != 0) {
                    perror("Error al unir el hilo de carga de operaciones");
                    exit(EXIT_FAILURE);
                }

                printf("Carga de operaciones completada.\n");

                // Generar el reporte de operaciones
                struct operation_summary summary;
                summary.withdrawals = 0;
                summary.deposits = 0;
                summary.transfers = 0;
                // Contar el número de cada tipo de operación
                for (int i = 0; i < total_operations; i++) {
                    switch (operations[i].operation_type) {
                        case 1:
                            summary.withdrawals++;
                            break;
                        case 2:
                            summary.deposits++;
                            break;
                        case 3:
                            summary.transfers++;
                            break;
                    }
                }
                generate_operation_report(summary);

                break;
            case 3:
                // Estado De Cuentas
                // Mostrar estados de cuenta en consola
                print_account_states();
                break;
            case 4:
                // Reporte de Carga de Usuarios
                // Aquí puedes agregar la lógica para generar el reporte de carga de usuarios
                break;
            case 5:
                // Reporte de Errores
                // Aquí puedes agregar la lógica para mostrar el reporte de errores
                break;
            case 6:
                // Salir del programa
                printf("Saliendo del programa...\n");
                break;
            default:
                printf("Opción inválida. Por favor, seleccione una opción válida.\n");
        }
    } while (option != 6);

    return 0;
}
