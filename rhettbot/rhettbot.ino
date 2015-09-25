// rhettbot.ino - sketch for the rhett bot, a fun robot built for a dog

#include <Servo.h>

#define LEFT_CENTER 1500
#define RIGHT_CENTER 1500
#define PIEZO_PIN 4
#define PIR_PIN 7

Servo servoLeft, servoRight;
bool running = false;

void setup() {
    // set up pins for input/output
    pinMode(10, INPUT);
    pinMode(3, INPUT);
    pinMode(9, OUTPUT);
    pinMode(2, OUTPUT);
    pinMode(7, INPUT);
    digitalWrite(PIR_PIN, LOW);

    // emit a sound to indicate start
    tone(PIEZO_PIN, 1000, 300);
    delay(300);
    tone(PIEZO_PIN, 2000, 300);
    delay(300);
    tone(PIEZO_PIN, 3000, 300);
    delay(300);

    // attach servos
    servoLeft.attach(13);
    servoRight.attach(12);
}

void loop() {
    int pir = digitalRead(PIR_PIN);
    if (pir == HIGH && !running) {
        servoLeft.attach(13);
        servoRight.attach(12);
        running = true;
    } else if (pir == LOW && running) {
        servoLeft.detach();
        servoRight.detach();
        running = false;
    }

    if (running) {
        // read the IR pins
        int irLeft = irDetect(9, 10, 38000);
        int irRight = irDetect(2, 3, 38000);

        if (irLeft == 0 && irRight == 0) {
            // both IRs are active, object directly in front
            backward(20);
        } else if (irLeft == 0) {
            // object on left, turn right
            right(20);
        } else if (irRight == 0) {
            // object on right, turn left
            left(20);
        } else {
            // no object in sight, move forward
            forward(20);
        }
    }
}

int irDetect(int irLedPin, int irReceiverPin, long frequency) {
    // emit tone and read IR pin
    tone(irLedPin, frequency, 8);
    delay(1);
    int ir = digitalRead(irReceiverPin);
    delay(1);
    return ir;
}

void forward(int time) {
    servoLeft.writeMicroseconds(LEFT_CENTER + 200);
    servoRight.writeMicroseconds(RIGHT_CENTER - 200);
    delay(time);
}

void backward(int time) {
    servoLeft.writeMicroseconds(LEFT_CENTER - 200);
    servoRight.writeMicroseconds(RIGHT_CENTER + 200);
    delay(time);
}

void left(int time) {
    servoLeft.writeMicroseconds(LEFT_CENTER - 200);
    servoRight.writeMicroseconds(RIGHT_CENTER - 200);
    delay(time);
}

void right(int time) {
    servoLeft.writeMicroseconds(LEFT_CENTER + 200);
    servoRight.writeMicroseconds(RIGHT_CENTER + 200);
    delay(time);
}
