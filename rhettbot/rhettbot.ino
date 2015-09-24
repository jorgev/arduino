// rhettbot.ino - sketch for the rhett bot, a fun robot built for a dog

#include <Servo.h>

Servo servoLeft, servoRight;

void setup() {
    servoLeft.attach(13);
    servoRight.attach(12);

    // initialize to still
    stop();

    // set up pins for input/output
    pinMode(10, INPUT);
    pinMode(3, INPUT);
    pinMode(8, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(2, OUTPUT);

    // emit a sound to indicate start
    tone(4, 1000, 300);
    delay(300);
    tone(4, 2000, 300);
    delay(300);
    tone(4, 3000, 300);
    delay(300);
}

void loop() {
    int irLeft = irDetect(9, 10, 38000);
    int irRight = irDetect(2, 3, 38000);

    digitalWrite(8, !irLeft);
    digitalWrite(7, !irRight);
}

int irDetect(int irLedPin, int irReceiverPin, long frequency) {
    tone(irLedPin, frequency, 8);
    delay(1);
    int ir = digitalRead(irReceiverPin);
    delay(1);
    return ir;
}

void forward(int time) {
    servoLeft.writeMicroseconds(1700);
    servoRight.writeMicroseconds(1300);
    delay(time);
}

void backward(int time) {
    servoLeft.writeMicroseconds(1300);
    servoRight.writeMicroseconds(1700);
    delay(time);
}

void turnLeft(int time) {
    servoLeft.writeMicroseconds(1300);
    servoRight.writeMicroseconds(1300);
    delay(time);
}

void turnRight(int time) {
    servoLeft.writeMicroseconds(1700);
    servoRight.writeMicroseconds(1700);
    delay(time);
}

void stop() {
    servoLeft.writeMicroseconds(1500);
    servoRight.writeMicroseconds(1500);
}
