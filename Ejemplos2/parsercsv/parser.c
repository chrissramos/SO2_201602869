#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define MAX_USERS 400
#define MAX_NAME_LENGTH 50
#define CSV_ROW_LENGTH 1000

// Estructura para almacenar los datos de un usuario
struct user {
    int account_number;
    char first_name[MAX_NAME_LENGTH];
    double balance;
};

struct user users[MAX_USERS]; // Arreglo para almacenar los usuarios
int total_users = 0; // Contador de usuarios cargados
pthread_mutex_t mutex_users = PTHREAD_MUTEX_INITIALIZER; // Mutex para controlar el acceso al arreglo de usuarios

// Estructura para almacenar los registros con errores
struct error_record {
    int line_number;
    char row[CSV_ROW_LENGTH];
    char error_message[100];
};

struct error_record error_records[MAX_USERS]; // Arreglo para almacenar los registros con errores
int total_errors = 0; // Contador de registros con errores

// Función para imprimir el registro cargado y el número de hilo asociado
void print_loaded_record(int thread_number, int account_number, char *first_name, double balance) {
    printf("Hilo %d: Cargando registro - Cuenta: %d, Nombre: %s, Saldo: %.2lf\n", thread_number, account_number, first_name, balance);
}
//Funcion para Estados de cuenta:
void show_account_status() {
    printf("\nEstado de Cuentas:\n");
    printf("No. Cuenta, Nombre, Saldo\n");
    for (int i = 0; i < total_users; i++) {
        printf("%d, %s, %.2lf\n", users[i].account_number, users[i].first_name, users[i].balance);
    }
}


// Función para cargar usuarios desde un archivo CSV en un hilo
void *load_users(void *filename) {
    char *csv_file = (char *)filename;
    FILE *fp = fopen(csv_file, "r");
    if (fp == NULL) {
        perror("Error al abrir el archivo");
        pthread_exit(NULL);
    }

    char row[CSV_ROW_LENGTH];
    char *token;

    // Se omite la primera fila (encabezados)
    fgets(row, CSV_ROW_LENGTH, fp);

    while (fgets(row, CSV_ROW_LENGTH, fp) != NULL) {
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
        } else {
            printf("Error: No se pudo cargar el usuario %d, límite de usuarios alcanzado.\n", account_number);
        }
        // Desbloquear el acceso al arreglo de usuarios
        pthread_mutex_unlock(&mutex_users);
    }

    fclose(fp);
    pthread_exit(NULL);
}
// Función para generar el reporte de carga de usuarios
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
    fprintf(fp, "Usuarios Cargados:\n");
    for (int i = 0; i < 3; i++) {
        fprintf(fp, "Hilo %d: %d\n", i + 1, total_users / 3);
    }
    fprintf(fp, "Total: %d\n\n", total_users);

    // Escribir los registros con errores
    fprintf(fp, "Errores:\n");
    for (int i = 0; i < total_errors; i++) {
        fprintf(fp, "Línea %d: %s - %s\n", error_records[i].line_number, error_records[i].row, error_records[i].error_message);
    }

    // Cerrar el archivo
    fclose(fp);

    printf("Reporte de carga de usuarios generado: %s\n", filename);
}

// Función para mostrar el menú y obtener la opción del usuario
int show_menu() {
    int option;
    printf("\nMenu:\n");
    printf("1. Operaciones Individuales.\n");
    printf("2. Carga Masiva.\n");
    printf("3. Estado De Cuentas.\n");
    printf("4. Reporte de Carga de usuarios.\n");
    printf("5. Reporte de carga de operaciones masivas.\n");
    printf("Ingrese el número de opción deseada: ");
    scanf("%d", &option);
    return option;
}

int main() {
    // Nombre del archivo CSV con los usuarios
    char filename[] = "usuarios.csv";

    // Hilos para la carga masiva de usuarios
    pthread_t threads[3];

    // Iniciar la carga masiva de usuarios en hilos separados
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&threads[i], NULL, load_users, (void *)filename) != 0) {
            perror("Error al crear el hilo");
            exit(EXIT_FAILURE);
        }
    }

    // Esperar a que todos los hilos de carga masiva de usuarios terminen
    for (int i = 0; i < 3; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Error al unir el hilo");
            exit(EXIT_FAILURE);
        }
    }

    printf("Carga masiva de usuarios completada.\n");

    // Generar el reporte de carga de usuarios al seleccionar la opción 4 del menú
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
                // Carga Masiva
                // Aquí puedes agregar la lógica para la carga masiva de operaciones
                break;
            case 3:
                // Estado De Cuentas
                show_account_status();
                // Aquí puedes agregar la lógica para mostrar el estado de las cuentas
                break;
            case 4:
                // Reporte de Carga de usuarios
                // Aquí puedes agregar la lógica para generar el reporte de carga de usuarios
                generate_user_load_report();
                break;
            case 5:
                // Reporte de carga de operaciones masivas
                // Aquí puedes agregar la lógica para generar el reporte de carga de operaciones masivas
                break;
            default:
                printf("Opción inválida. Por favor, seleccione una opción válida.\n");
        }
    } while (option != 0);

    return 0;
}
