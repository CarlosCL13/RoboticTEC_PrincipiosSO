#include "arduino_lib.h"
#include <unistd.h>

int main() {
    sleep(1); // Espera 1 segundo antes de iniciar
    moveServo(1,90);
    sleep(1); // Espera 1 segundo para que el servo se mueva
    moveServo(1,45);
    sleep(1); // Espera 1 segundo para que el servo se mueva
    moveServo(1,180);
    sleep(1); // Espera 1 segundo para que el servo se mueva
    reset_position_servos();
    return 0;
}

// Compilación:
// gcc test_lib.c arduino_lib.a -o programa