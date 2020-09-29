#include <WiFiManager.h>
#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const char* AWS_endpoint = "***** ";                            //Replace with your AWS IoT Endpoint 

WiFiUDP ntpUDP;
WiFiClientSecure espClient;

NTPClient timeClient(ntpUDP, "pool.ntp.org"); 

long lastMsg = 0;
char msg[50];
int value = 0;
int LED = 13;

String relay;
String strTopic;
String strPayload;

void callback(char* topic, byte* payload, unsigned int length) {

  payload[length] = '\0';
  strTopic = String((char*)topic);

    if(strTopic == "ESP8266") {              // Set to desired topic

        relay = String((char*)payload);

        if(relay == "relay_5_off") { 
        digitalWrite(12, LOW);
        digitalWrite(LED, HIGH);
        }

        if(relay == "relay_5_on") { 
        digitalWrite(12, HIGH); 
        digitalWrite(LED, LOW);
        }

        if(relay == "relay_all_off") { 
        digitalWrite(12, LOW); 
        digitalWrite(LED, HIGH);
        }

        if(relay == "relay_all_on") { 
        digitalWrite(12, HIGH); 
        digitalWrite(LED, LOW);
        }
    }
}

PubSubClient client(AWS_endpoint, 8883, callback, espClient);

void setup_wifi() {
  
    delay(100);

    espClient.setBufferSizes(512, 512);

    WiFiManager wifiManager;                 // Because of WifiManeger Library, you don't need to enter Wi-Fi credentials. If device cannot connect to any Wi-Fi network, It's automaticliy switches to AP mode.
    wifiManager.autoConnect("SonOff_Basic"); // Connect to AP named "SonOff_Basic" and enter your Wi-Fi credentials to connect SonOff device to Wi-Fi

    digitalWrite(LED, LOW);
    delay(200);
    digitalWrite(LED, HIGH);

    timeClient.begin();

    while(!timeClient.update()) {

        timeClient.forceUpdate();
    }

    espClient.setX509Time(timeClient.getEpochTime());
}


void reconnect() {

    digitalWrite(LED, LOW);
    delay(200);
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
    delay(200);
    digitalWrite(LED, HIGH);

    while (!client.connected()) {

        Serial.print("Attempting MQTT connection...");
        
        if (client.connect("ESP8266")) {                          

            Serial.println("connected");                             
        
            client.publish("ESP8266", "SonOff Switch Connected!");    // Set to desired topic
            client.subscribe("ESP8266");                              // Set to desired topic
 
            digitalWrite(LED, LOW);
            delay(200);
            digitalWrite(LED, HIGH);
            delay(200);
            digitalWrite(LED, LOW);
            delay(200);
            digitalWrite(LED, HIGH);
            delay(200);
            digitalWrite(LED, LOW);
            delay(200);
            digitalWrite(LED, HIGH);
        }

        else {

            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");

            char buf[256];
            espClient.getLastSSLError(buf,256);

            Serial.print("WiFiClientSecure SSL error: ");
            Serial.println(buf);

            digitalWrite(LED, LOW);
            delay(400);
            digitalWrite(LED, HIGH);
            delay(4600);
        }
    }
}

void setup() {

    Serial.begin(115200);
    Serial.setDebugOutput(true);
    
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
    pinMode(12, OUTPUT);
    setup_wifi();
    delay(1000);

    if (!SPIFFS.begin()) {
        
        Serial.println("Failed to mount file system");
        return;
    }

    Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());

    File cert = SPIFFS.open("/cert.der", "r"); 
    
    if (!cert) {
        Serial.println("Failed to open cert file");
    }

    else {
        Serial.println("Success to open cert file");
    }

    delay(1000);

    if (espClient.loadCertificate(cert)) {
        Serial.println("cert loaded");
    }

    else{
        Serial.println("cert not loaded");
    }

    File private_key = SPIFFS.open("/private.der", "r"); 

    if (!private_key) {
        Serial.println("Failed to open private cert file");
    }

    else {
        Serial.println("Success to open private cert file");
    }

    delay(1000);

    if (espClient.loadPrivateKey(private_key)) {
        Serial.println("private key loaded");
    }

    else{
        Serial.println("private key not loaded");
    }

    File ca = SPIFFS.open("/ca.der", "r"); 

    if (!ca) {
        Serial.println("Failed to open ca ");
    }

    else {
        Serial.println("Success to open ca");
    }

    delay(1000);

    if(espClient.loadCACert(ca)){
        Serial.println("ca loaded");
    }

    else {
        Serial.println("ca failed");
    }

    Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());
}

void loop() {

    if (!client.connected()) {
        reconnect();
    }
    
    client.loop();
}
