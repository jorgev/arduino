// build status

#include <SPI.h>
#include <Ethernet.h>

#define LRQ			2
#define STROBE		9

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x8E, 0xC1 };
byte ip[] = { 172, 22, 4, 120 };
byte gateway[] = { 172, 22, 0, 1 };
byte subnet[] = { 255, 255, 0, 0 };
EthernetServer server(80);

const int BUFSIZE = 1024;

void send_response(EthernetClient& client, int status_code, char* body);

void setup() {
	Serial.begin(9600);

	// set up ethernet
	Ethernet.begin(mac, ip, gateway, subnet);
	server.begin();
	Serial.print("server is at ");
	Serial.println(Ethernet.localIP());
}

void loop() {
	EthernetClient client = server.available();
	if (client) {
		char* buf = (char*) malloc(BUFSIZE);
		if (buf == NULL)
			Serial.println("*** FAILED TO ALLOCATE BUFFER FOR READ ***");
		char* p = buf;
		bool in_headers = true;
		bool request_line = true;
		int content_length = 0;
		int status_code = 200;
		while (client.connected()) {
			if (client.available()) {
				char ch = client.read();
				Serial.write(ch);
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
							}
							
							// if we got a blank line, we're out of the headers
							if (strlen(buf) == 0) {
								in_headers = false;
								
								// if this is a request without a body, respond immediately and close
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
					if ((int) (p - ch) > BUFSIZE) {
						// out of buffer space, process what we have and reset pointer
						p = buf;
					}
					if (--content_length == 0) {
						// send the response and break out of the receive loop
						send_response(client, status_code, NULL);
						break;
					}
				}
			}
		}

		// add a carriage return to separate the requests
		Serial.println();

		// we're done receiving the data, terminate the buffer, reset our pointer
		*p = 0;
		p = buf;

		// loop over the command
		while (*p) {
			// have to wait for the LRQ pin to go high
			while (analogRead(LRQ) == 0) {
			}
			
			// set the analog bits to the right values

			// toggle the strobe pin
			digitalWrite(STROBE, HIGH);
			digitalWrite(STROBE, LOW);

			// move to the next character
			p++;
		}

		// done with the buffer
		free(buf);
	}
}

void send_response(EthernetClient& client, int status_code, char* body)
{
	// send the appropriate response
	if (status_code == 200) {
		client.println("HTTP/1.1 200 OK");
	}
	else if (status_code == 405) {
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

