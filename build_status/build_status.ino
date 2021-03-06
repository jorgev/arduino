// build status

#include <SPI.h>
#include <Ethernet.h>

#define LRQ         9
#define STROBE      8
#define PING        7
#define NEAR        6
#define FAR         5
#define DATA        3
#define STAGE       4
#define IR_RECV     2
#define NEAR_DISTANCE_COUNT 10
#define FAR_DISTANCE_COUNT 100

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x8E, 0xC1 };
IPAddress ip(172, 22, 4, 120);
IPAddress devdns(172, 22, 1, 1);
IPAddress gateway(172, 22, 0, 1);
IPAddress subnet(255, 255, 0, 0);
EthernetServer server(80);
int near_count = NEAR_DISTANCE_COUNT;
int far_count = FAR_DISTANCE_COUNT;
bool is_near = false;
int last_inches = -1;

const int BUFSIZE = 256;
char buf[BUFSIZE];
char last_message[BUFSIZE] = "Ed:M\"N3\")M'4\"4\"D40"; // shall we play a game?
char* hammer = "/d\"\"\"Ue\"\"\"/d\"\"\"Ue\"\"\"Y\\\"(//0K\"2//\"Y:0S";
char* djeat = "*3-";
char* squeat = "WJ63-";
char* shesaid = "2:-W\"N/1\"E3\"WW''5";
char* yousaid = "Q?\"WW''5\",\"-\"0::+";
char* jesus = "X/U<77A3\"H//IWW\"N,=\"2//\"*3K/W";
char* notapussy = "9''WW\"\"\"8&\":0\"77MWWU\"X88-\"4\")//WW4";
char* chill = "R,Med";
char* mofo = "0//2:S\"H//IS";
char* mumbler = "0//0MS";
char* huge = "Q?dddddd*";

void send_response(EthernetClient& client, int status_code);
void say_message(const char* pb);
void set_bits(byte data);

void setup() {
    Serial.begin(9600);

    // set up ethernet
    Ethernet.begin(mac, ip, devdns, gateway, subnet);
    server.begin();
    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());

    // designate the input/output pins
    pinMode(STROBE, OUTPUT);
    pinMode(LRQ, INPUT);
    pinMode(DATA, OUTPUT);
    pinMode(STAGE, OUTPUT);
    pinMode(FAR, OUTPUT);
    pinMode(NEAR, OUTPUT);
    pinMode(PING, OUTPUT);
    pinMode(IR_RECV, INPUT);

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
        int status_code = 204;
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
                            if (strncasecmp(buf, "POST", 4) != 0) {
                                status_code = 405;
                            }
                            request_line = false;
                        } else {
                            // the content length is probably the most important header
                            if (strncasecmp(buf, "Content-Length: ", 16) == 0) {
                                content_length = atoi(buf + 16);
                            } else if (strncasecmp(buf, "Transfer-Encoding: ", 19) == 0) {
                                // assume chunked, not sure if there is another valid value
                                chunked = true;
                            }
                            
                            // if we got a blank line, we're out of the headers
                            if (strlen(buf) == 0) {
                                in_headers = false;
                                
                                // if content length is 0, nothing left to do
                                if (content_length == 0) {
                                    send_response(client, status_code);
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
                        send_response(client, status_code);
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
    if (digitalRead(IR_RECV) == LOW) {
        // wait for a start bit
        unsigned long ret = pulseIn(IR_RECV, LOW, 100000);
        while (ret < 2200) {
            if (ret == 0)
                break;
            ret = pulseIn(IR_RECV, LOW, 100000);
        }

        // loop and set bits as needed
        int i, result = 0;
        for (i = 0; i < 12; i++) {
            if (pulseIn(IR_RECV, LOW) > 1000) {
                result |= 1 << i;
            }
        }

        // do something based on the code received
        Serial.println(result, HEX);
        switch (result) {
            case 0x80: // #1
                say_message(djeat);
                break;

            case 0x81: // #2
                say_message(squeat);
                break;

            case 0x82: // #3
                say_message(yousaid);
                break;

            case 0x83: // #4
                say_message(jesus);
                break;

            case 0x85: // #6
                say_message(hammer);
                break;

            case 0x86: // #7
                say_message(chill);
                break;

            case 0x88: // #9
                say_message(shesaid);
                break;

            case 0x89: // #0
                say_message(notapussy);
                break;

            case 0x8B: // Enter
                break;

            case 0xBB: // prev ch
                say_message(last_message);
                break;

            default:
                Serial.println("No handler for code");
                break;
        }
    }

    // check for data on the serial port
    if (Serial.available()) {
        char ch = Serial.read();
        if (ch == '\r' || ch == '\n') {
            Serial.println();
        } else {
            Serial.print(ch); // echo to sender
        }
    }
}

void send_response(EthernetClient& client, int status_code)
{
    // send the appropriate response
    if (status_code == 204) {
        client.println("HTTP/1.1 204 No Content");
    } else if (status_code == 405) {
        client.println("HTTP/1.1 405 Method Not Allowed");
    }
    client.println("Server: buildbot/1.0");
    client.println("Content-Length: 0");
    client.println();

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

