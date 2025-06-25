#ifndef ARDUINO_LIB_H
#define ARDUINO_LIB_H

int moveServo(short servo_num, short angulo);
int reset_position_servos(void);
int on_led(short señal);

#endif