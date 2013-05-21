// build status

#include <SPI.h>
#include <Ethernet.h>
#include "IRremote.h"

#define LRQ         9
#define STROBE      8
#define PING        7
#define NEAR        6
#define FAR         5
#define DATA        3
#define STAGE       4
#define IR_RECV     11
#define NEAR_DISTANCE_COUNT 10
#define FAR_DISTANCE_COUNT 100

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x8E, 0xC1 };
byte ip[] = { 172, 22, 4, 120 };
byte gateway[] = { 172, 22, 0, 1 };
byte subnet[] = { 255, 255, 0, 0 };
EthernetServer server(80);
int near_count = NEAR_DISTANCE_COUNT;
int far_count = FAR_DISTANCE_COUNT;
bool is_near = false;
int last_inches = -1;

IRrecv irrecv(IR_RECV);
decode_results results;

const int BUFSIZE = 256;
char buf[BUFSIZE];
char last_message[BUFSIZE] = "Ed:M\"N3\")M'4\"4\"D40"; // shall we play a game?
char* hammer = "/d\"\"\"Ue\"\"\"/d\"\"\"Ue\"\"\"Y\\\"(//0K\"2//\"Y:0S";
char* djeat = "*3-";
char* squeat = "WJ63-";
char* shesaid = "2:-W\"N/1\"E3\"WW''5";

void send_response(EthernetClient& client, int status_code, char* body);
void say_message(const char* pb);
void set_bits(byte data);

void setup() {
    Serial.begin(9600);

    // set up the IR receiver
    irrecv.enableIRIn();

    // set up ethernet
    Ethernet.begin(mac, ip, gateway, subnet);
    server.begin();
    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());

    // designate the input/output pins
    pinMode(STROBE, OUTPUT);
    pinMode(LRQ, INPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);

    // set strobe high, we will monitor on it for sending data
    digitalWrite(STROBE, HIGH);
    digitalWrite(NEAR, HIGH);
    digitalWrite(FAR, LOW);
}

void loop() {
    EthernetClient client = server.available();
    if (client) {
        char* p = buf;
        bool in_headers = true;
        bool request_line = true;
        bool chunked = false;
        int chunked_size = -1;
        int chunked_count = 0;
        int content_length = -1;
        int status_code = 200;
        while (client.connected()) {
            if (client.available()) {
                char ch = client.read();
                //Serial.write(ch);
                if (in_headers) {
                    if (ch == '\n') {
                        // end of line, process it
                        *p = 0;
                        
                        if (request_line) {
                            // this is the very first line
                            if (strncmp(buf, "POST", 4) != 0) {
                                status_code = 405;
                            }
                            request_line = false;
                        } else {
                            // the content length is probably the most important header
                            if (strncmp(buf, "Content-Length: ", 16) == 0) {
                                content_length = atoi(buf + 16);
                            } else if (strncmp(buf, "Transfer-Encoding: ", 19) == 0) {
                                // assume chunked, not sure if there is another valid value
                                chunked = true;
                            }
                            
                            // if we got a blank line, we're out of the headers
                            if (strlen(buf) == 0) {
                                in_headers = false;
                                
                                // if content length is 0, nothing left to do
                                if (content_length == 0) {
                                    send_response(client, status_code, NULL);
                                    break;
                                }
                            }
                        }

                        // reset our pointer
                        p = buf;
                    } else if (ch != '\r') {
                        *p++ = ch;
                    }
                } else {
                    *p++ = ch;
                    if ((int) (p - buf) > BUFSIZE) {
                        // TODO: out of buffer space, process what we have and reset pointer
                        p = buf;
                    }

                    if (chunked) {
                        if (ch == '\n') {
                        }
                    } else if (--content_length == 0) {
                        // send the response and break out of the receive loop
                        send_response(client, status_code, NULL);
                        break;
                    }
                }
            }
        }

        // add a carriage return to separate the requests
        //Serial.println();

        // we're done receiving the data, terminate the buffer, reset our pointer
        *p = 0;
        p = buf;
        if (strlen(p) > 0) {
            say_message(p);
            strcpy(last_message, p);
        }
    }

    // check proximity
    pinMode(PING, OUTPUT);
    digitalWrite(PING, LOW);
    delayMicroseconds(2);
    digitalWrite(PING, HIGH);
    delayMicroseconds(5);
    digitalWrite(PING, LOW);
    pinMode(PING, INPUT);
    long duration = pulseIn(PING, HIGH);
    long inches = (duration / 74);
    if (last_inches != -1) {
        int delta = inches - last_inches;
        inches = last_inches + max(min(delta / 8, 2), -2);
    }
    last_inches = inches;
    //Serial.println(inches);
    //pinMode(PING, OUTPUT);
    if (inches < 24) {
        if (near_count > 0) {
            near_count--;
            far_count = FAR_DISTANCE_COUNT;
        }
    } else if (inches >= 36) {
        if (far_count > 0) {
            far_count--;
            near_count = NEAR_DISTANCE_COUNT;
        }
    }
    int val = map(inches, 1, 120, 0, 255);
    val = constrain(val, 0, 255);
    analogWrite(FAR, val);
    delay(10);
    
    if (near_count == 0 && !is_near) {
        is_near = true;
        Serial.println("Object is near");
        digitalWrite(NEAR, LOW);
        say_message(last_message);
    }
    if (far_count == 0 && is_near) {
        is_near = false;
        Serial.println("Object is far");
        digitalWrite(NEAR, HIGH);
    }

    // check for IR signal
    if (irrecv.decode(&results)) {
        irrecv.resume();
        switch (results.value) {
            case -1:
                // bad signal, ignoring
                break;

            case 0x10: // #1
                say_message(djeat);
                break;

            case 0x810: // #2
                say_message(squeat);
                break;

            case 0xa10: // #6
                say_message(hammer);
                break;

            case 0x110: // #9
                say_message(shesaid);
                break;

            case 0xd10: // Enter
                break;

            case 0xdd0: // prev ch
                say_message(last_message);
                break;

            default:
                Serial.print("Unhandled code: ");
                Serial.println(results.value, HEX);
                break;
        }

        // dump out any remaining signals
        while (irrecv.decode(&results)) {
            irrecv.resume();
        }
    }

    // check for data on the serial port
    if (Serial.available()) {
        int ch = Serial.read();
        //Serial.write(ch);
        if (ch == 'r')
            say_message(last_message);
    }
}

void send_response(EthernetClient& client, int status_code, char* body)
{
    // send the appropriate response
    if (status_code == 200) {
        client.println("HTTP/1.1 200 OK");
    } else if (status_code == 405) {
        client.println("HTTP/1.1 405 Method Not Allowed");
    }
    client.println("Server: arduino/1.0");
    int content_length = 0;
    if (body != NULL)
        content_length = strlen(body);
    client.print("Content-Length: ");
    client.println(content_length);
    client.println();

    // send the body, if we have one
    if (body != NULL) {
    }

    // sample code indicates we need this
    delay(1);
    client.stop();
}

void say_message(const char* p)
{
    // loop over the command
    while (*p) {
        if (*p >= 'a') {
            if (*p == 'a')
                delay(10);
            else if (*p == 'b')
                delay(20);
            else if (*p == 'c')
                delay(50);
            else if (*p == 'd')
                delay(100);
            else if (*p == 'e')
                delay(200);
        } else {
            set_bits(*p - 0x20);
        }
        p++;
    }

    // turn off
    set_bits(0);
}

void set_bits(byte data)
{
    int i;

    int mask = 0x20;
    for (i = 0; i < 6; i++) {
        digitalWrite(DATA, (data & mask) ? HIGH : LOW);
        //Serial.print(data & mask);
        mask >>= 1;
        digitalWrite(STAGE, HIGH);
        digitalWrite(STAGE, LOW);
    }
    //Serial.println();

    while (digitalRead(LRQ) == HIGH) {
        // waiting for LRQ
    }

    digitalWrite(STROBE, LOW);
    digitalWrite(STROBE, HIGH);
}

