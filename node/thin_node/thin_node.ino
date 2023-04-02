#include <WiFi.h>
#include <WiFiUdp.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Adafruit_NeoPixel.h>

#include "credentials.h"
// create your own credentials.h file with the following format:
//
// #define WIFI_SSID "Your SdSID Here"
// #define WIFI_PWD "Your WiFi Password Here"
 
const int ledPin = 13;
const int maxStripLen = 80;

// Node ID
const int nodeID = 1;

const unsigned int serverPort = 5431;
const unsigned int nodePort   = 5432;

const int udpBufferSize = 1024;

WiFiUDP udp;

IPAddress broadcastAddress(255,255,255,255);

Adafruit_NeoPixel strip(maxStripLen, ledPin, NEO_GRB + NEO_KHZ800);

// Function declarations
void connectToWiFi();

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("Setup started");

  // Initialize LED strip
  strip.begin();
  strip.fill((0,0,0));
  strip.show();
  Serial.println("LED strip initialized");

  // Connect to WiFi
  connectToWiFi();

  // Start the UDP server
  udp.begin(nodePort);
  Serial.print("UDP server started on port ");
  Serial.println(nodePort);
}

// Connect to WiFi
void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void sendRegister() {
  udp.beginPacket(broadcastAddress, serverPort);
  udp.print("MUT");
  byte buf[2];
  
  udp.write(nodeID >> 8);
  udp.write(nodeID & 0xFF);
  udp.write('R');
  udp.endPacket();
  Serial.print("Sent register packet for node");
  Serial.println(nodeID);
}

void pixelMsg(char *payload) {
  int length = payload[0] * 255 + payload[1];
  char *colors = payload + 2;

  for (int i = 0; i < length; i++) {
    strip.setPixelColor(i, int(colors[i*3]), int(colors[i*3+1]), int(colors[i*3+2]));
  }
  strip.show();
}

unsigned long lastRegisterTime = 0;
char packetBuffer[udpBufferSize]; //buffer to hold incoming packet
int lastSeqNum = 0;

void readMsg() {
  int len = udp.read(packetBuffer, udpBufferSize);

  if (strncmp("MUT", packetBuffer, 3) != 0) {
    Serial.println(packetBuffer);
    Serial.println("ERROR packet received with invalid signature");
    return;
  }
  int seqNum = packetBuffer[3] * 255 + packetBuffer[4];  

  if (seqNum < lastSeqNum) {
    Serial.println("WARNING: dropping out of sequence packet");

    // if the sequence number is way off, assume that the server
    // reset the sequence numbers rather than lost packets
    if (seqNum < lastSeqNum - 10) {
      lastSeqNum = seqNum;
    }
    return;
  }

  if (seqNum > lastSeqNum + 1) {
    Serial.println("WARNING: detected missed packet");
  }
  lastSeqNum = seqNum;

  char msgType = packetBuffer[5];
  char *payload = packetBuffer + 6;

  switch (msgType) {
    case 'P':
      pixelMsg(payload);
      break;
    default:
      Serial.print("WARNING: received unknown message type ");
      Serial.println(msgType);
  }

  //Serial.println("Received packet");
  
}
void loop() {
unsigned long currentTime = millis();

  if ((currentTime == 0) || (currentTime >= lastRegisterTime + 10000)) {
    sendRegister();
    lastRegisterTime = currentTime;
  }

  int packetSize = udp.parsePacket();

  if (packetSize) {
    readMsg();
  }


  delay(10);
}