// rhettbot.ino - sketch for the rhett bot, a fun robot built for a dog

#include <Servo.h>

Servo servoLeft, servoRight;

void setup() {
    servoLeft.attach(13);
    servoRight.attach(12);

    // initialize to still
    stop();

    // emit a sound to indicate start
    tone(4, 3000, 1000);
    delay(1000);
}

void loop() {
}

void goForward() {
    servoLeft.writeMicroseconds(1700);
    servoRight.writeMicroseconds(1300);
}

void goBackward() {
    servoLeft.writeMicroseconds(1300);
    servoRight.writeMicroseconds(1700);
}

void stop() {
    servoLeft.writeMicroseconds(1500);
    servoRight.writeMicroseconds(1500);
}

void turnLeft() {
    servoLeft.writeMicroseconds(1300);
    servoRight.writeMicroseconds(1300);
}

void turnRight() {
    servoLeft.writeMicroseconds(1700);
    servoRight.writeMicroseconds(1700);
}
