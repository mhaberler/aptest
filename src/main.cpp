
/*
  WiFiAccessPoint.ino creates a WiFi access point and provides a web server on it.

  Steps:
  1. Connect to the access point "yourAp"
  2. Point your web browser to http://192.168.4.1/H to turn the LED on or http://192.168.4.1/L to turn it off
     OR
     Run raw TCP "GET /H" and "GET /L" on PuTTY terminal with 192.168.4.1 as IP address and 80 as port

  Created for arduino-esp32 on 04 July, 2018
  by Elochukwu Ifediora (fedy0)
*/

#include <WiFi.h>
#include <NetworkClient.h>
#include <WiFiAP.h>
#include <ESPmDNS.h>

#include <PicoMQTT.h>

#if __has_include("config.h")
    #include "config.h"
#endif
#define CHANNEL 1
#define MAX_CLIENTS 4
#ifndef LED_BUILTIN
    #define LED_BUILTIN 2  // Set the GPIO pin where you connected your test LED or comment this line out if your dev board has a built-in LED
#endif

// Set these to your desired credentials.
const char *ssid = "yourAP";
const char *password = NULL; // no password 

NetworkServer server(80);
PicoMQTT::Server mqtt;

void setup() {
    delay(3000);
    //   pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    log_i("Configuring access point...");

    IPAddress localIP =  IPAddress("192.168.5.1");
    IPAddress gatewayIP =  IPAddress("0.0.0.0");
    IPAddress subnetMask =  IPAddress("255.255.255.0");
    WiFi.softAPConfig( localIP,  gatewayIP,  subnetMask); // , dhcp_lease_start, dns);

    if (!WiFi.softAP(ssid, password, CHANNEL, MAX_CLIENTS)) { // no password
        log_e("Soft AP creation failed.");
        while (1);
    }
    IPAddress myIP = WiFi.softAPIP();
    log_i("AP IP address: %s", myIP.toString().c_str());

    server.begin();

    if (MDNS.begin(ssid)) {
        log_i("MDNS responder started");
    }
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("mqtt", "tcp", 1883);
    log_i("weberver started");
    mqtt.begin();
    log_i("mqtt erver started");

}

void loop() {
    mqtt.loop();

    NetworkClient client = server.accept();  // listen for incoming clients

    if (client) {                     // if you get a client,
        log_i("New Client.");  // print a message out the serial port
        String currentLine = "";        // make a String to hold incoming data from the client
        while (client.connected()) {    // loop while the client's connected
            if (client.available()) {     // if there's bytes to read from the client,
                char c = client.read();     // read a byte, then
                // log_i("%c", c);            // print it out the serial monitor
                if (c == '\n') {            // if the byte is a newline character

                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0) {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();

                        // the content of the HTTP response follows the header:
                        client.print("Click <a href=\"/H\">here</a> to turn ON the LED.<br>");
                        client.print("Click <a href=\"/L\">here</a> to turn OFF the LED.<br>");

                        // The HTTP response ends with another blank line:
                        client.println();
                        // break out of the while loop:
                        break;
                    } else {  // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                } else if (c != '\r') {  // if you got anything else but a carriage return character,
                    currentLine += c;      // add it to the end of the currentLine
                }

                // Check to see if the client request was "GET /H" or "GET /L":
                if (currentLine.endsWith("GET /H")) {
                    //   digitalWrite(LED_BUILTIN, HIGH);  // GET /H turns the LED on
                }
                if (currentLine.endsWith("GET /L")) {
                    //   digitalWrite(LED_BUILTIN, LOW);  // GET /L turns the LED off
                }
            }
        }
        // close the connection:
        client.stop();
        log_i("Client Disconnected.");
    }
}