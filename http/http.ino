// http client

#include <SPI.h>
#include <Ethernet.h>
#include "http_client.h"

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x8E, 0xC1 };
byte ip[] = { 192, 168, 1, 120 };
byte gateway[] = { 192, 168, 1, 254 };
byte subnet[] = { 255, 255, 255, 0 };
byte server[] = { 74, 125, 224, 211 };

http_client client;

void process_command(const char* cmd);

void setup()
{
	// set up ethernet
	Ethernet.begin(mac, ip, gateway, subnet);
	Serial.begin(9600);
}

void loop()
{
	static char cmd[80];
	static char* p = cmd;

	if (Serial.available())
	{
		char ch = Serial.read();
		if (ch == '\r')
		{
			*p = 0;
			Serial.println();
			if (p > cmd)
			{
				process_command(cmd);
				p = cmd;
			}
		}
		else
		{
			*p++ = tolower(ch);
			Serial.print(ch);
		}
	}
}

void process_command(const char* cmd)
{
	if (strstr(cmd, "request") != NULL)
	{
		// make a request
		int ret = client.request(server, 80, "http://www.google.com/");
		if (ret != 0)
			Serial.println(ret);
	}
	else
	{
		char buf[80];
		sprintf(buf, "Unknown command: %s", cmd);
		Serial.println(buf);
	}
}

