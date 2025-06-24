
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define ARDUINO_DEVICE "/dev/arduino_driver"

int moveServo(short servo_num, short angulo) {

    int archivo_dispositivo = open(ARDUINO_DEVICE, O_WRONLY);
    if (archivo_dispositivo < 0) {
        perror("Error abriendo el dispositivo Arduino");
        return -1;
    }
    
    char comando[64]; // buffer para el comando (tamaño maximo para enviar al Arduino)
    
    snprintf(comando, sizeof(comando), "servo%hi:%hi\n", servo_num, angulo); // construir el comando para enviar al Arduino (para mover el servo 1)

    ssize_t bytes_escritos = write(archivo_dispositivo, comando, strlen(comando)); // se escribe el comando al archivo del dispositivo

    if (bytes_escritos < 0) {
        perror("Error escribiendo al archivo de dispositivo Arduino");
        close(archivo_dispositivo);
        return -1;
    }

    close(archivo_dispositivo);
    return 0;
}

// Esta función envía un comando al Arduino para resetear la posición de los servos
int reset_position_servos() {
    
    int archivo_dispositivo = open(ARDUINO_DEVICE, O_WRONLY);

    if (archivo_dispositivo < 0) {
        perror("Error abriendo el dispositivo Arduino");
        return -1;
    }

    char *comando = "reset_servos\n"; // comando para colocar en cero los ángulos de los servos
    ssize_t bytes_escritos = write(archivo_dispositivo, comando, strlen(comando));

    if (bytes_escritos < 0) {
        perror("Error escribiendo al archivo de dispositivo Arduino");
        close(archivo_dispositivo);
        return -1;
    }

    close(archivo_dispositivo);
    return 0;
}

// compilar: gcc -c arduino_lib.c -o arduino_lib.o
// crear la librería estática: ar rcs arduino_lib.a arduino_lib.o