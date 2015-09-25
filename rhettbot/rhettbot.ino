// rhettbot.ino - sketch for the rhett bot, a fun robot built for a dog

#include <Servo.h>

Servo servoLeft, servoRight;
unsigned long nextChange;
bool running = true;
int LEFT_STOP = 1500;
int RIGHT_STOP = 1500;

void setup() {
    // set up pins for input/output
    pinMode(10, INPUT);
    pinMode(3, INPUT);
    pinMode(9, OUTPUT);
    pinMode(2, OUTPUT);

    // emit a sound to indicate start
    tone(4, 1000, 300);
    delay(300);
    tone(4, 2000, 300);
    delay(300);
    tone(4, 3000, 300);
    delay(300);

    // attach servos
    servoLeft.attach(13);
    servoRight.attach(12);

    // set next change time
    nextChange = millis() + random(10, 20) * 1000;
}

void loop() {
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

    // check if we've reached our time interval
    if (nextChange <= millis()) {
        if (running) {
            // if bot is moving, stop it
            servoLeft.detach();
            servoRight.detach();
            running = false;
        } else {
            // if bot is stopped, start it
            servoLeft.attach(13);
            servoRight.attach(12);
            running = true;
        }

        // set up next time interval for start/stop
        nextChange = millis() + random(10, 20) * 1000;
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
    servoLeft.writeMicroseconds(LEFT_STOP + 200);
    servoRight.writeMicroseconds(RIGHT_STOP - 200);
    delay(time);
}

void backward(int time) {
    servoLeft.writeMicroseconds(LEFT_STOP - 200);
    servoRight.writeMicroseconds(RIGHT_STOP + 200);
    delay(time);
}

void left(int time) {
    servoLeft.writeMicroseconds(LEFT_STOP - 200);
    servoRight.writeMicroseconds(RIGHT_STOP - 200);
    delay(time);
}

void right(int time) {
    servoLeft.writeMicroseconds(LEFT_STOP + 200);
    servoRight.writeMicroseconds(RIGHT_STOP + 200);
    delay(time);
}
