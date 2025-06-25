#include <Servo.h>

Servo servo1;
Servo servo2;
#define PIN_LED 11   // Define el pin del LED como 11 

void setup() {
  Serial.begin(9600);
  servo1.attach(9);   // Conecta el primer servo al pin 9
  servo2.attach(10);  // Conecta el segundo servo al pin 10
  pinMode(PIN_LED, OUTPUT); // Configura el pin para encender el led
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n'); // Leer comando hasta salto de línea
    int sep = command.indexOf(':');

    if (sep > 0) {
      String name = command.substring(0, sep);
      int angle = command.substring(sep + 1).toInt();

      if (name == "servo1") {
        servo1.write(angle);
      } else if (name == "servo2") {
        servo2.write(angle);
      }
    } else if (command == "reset_servos"){
      servo1.write(0);
      servo2.write(0);

    } else if (command == "fin_word"){
      digitalWrite(PIN_LED, HIGH);

    } else if (command == "inicio_word"){
      digitalWrite(PIN_LED, LOW);
    }
  }
}
