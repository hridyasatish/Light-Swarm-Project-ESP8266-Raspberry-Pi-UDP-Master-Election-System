#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// Replace with your actual network credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Multicast configuration
IPAddress multicastAddress(239, 0, 0, 1);
unsigned int localUdpPort = 3000;
WiFiUDP udp;

// Packet constants
#define LIGHT_UPDATE_PACKET 0
#define RESET_SWARM_PACKET 1
const int PACKET_SIZE = 14;
byte packetBuffer[PACKET_SIZE];

// GPIO configuration
const int externalLedPin = D3;
const int onboardLedPin = LED_BUILTIN;
const int lightSensorPin = A0;

// Unique device ID (change for each ESP)
const uint8_t deviceID = 0x01;

// State variables
bool masterState = false;
int analogValue = 0;

// Swarm data structure
struct DeviceInfo {
  uint8_t deviceID;
  uint16_t reading;
  unsigned long lastHeard;
};

DeviceInfo devices[10];
int numDevices = 0;
unsigned long lastPacketTime = 0;
unsigned long lastBroadcastTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(externalLedPin, OUTPUT);
  pinMode(onboardLedPin, OUTPUT);
  digitalWrite(onboardLedPin, HIGH);

  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  udp.beginMulticast(WiFi.localIP(), multicastAddress, localUdpPort);
  devices[0] = {deviceID, 0, millis()};
  numDevices = 1;
  lastPacketTime = millis();
  lastBroadcastTime = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  analogValue = analogRead(lightSensorPin);
  int brightness = map(analogValue, 0, 1023, 0, 255);
  analogWrite(externalLedPin, masterState ? brightness : 0);
  digitalWrite(onboardLedPin, masterState ? LOW : HIGH);

  devices[0].reading = analogValue;
  devices[0].lastHeard = currentMillis;

  if (udp.parsePacket()) {
    udp.read(packetBuffer, PACKET_SIZE);
    if (packetBuffer[0] == 0xF0 && packetBuffer[13] == 0x0F) {
      if (packetBuffer[1] == LIGHT_UPDATE_PACKET) updateDevices(packetBuffer, currentMillis);
      else if (packetBuffer[1] == RESET_SWARM_PACKET) resetSwarm();
    }
  }

  if (currentMillis - lastBroadcastTime >= 500) {
    broadcastReadings();
    lastBroadcastTime = currentMillis;
  }

  determineMaster();
}

void updateDevices(byte* buffer, unsigned long currentMillis) {
  uint8_t senderID = buffer[2];
  uint16_t senderReading = (buffer[5] << 8) | buffer[6];

  for (int i = 0; i < numDevices; i++) {
    if (devices[i].deviceID == senderID) {
      devices[i].reading = senderReading;
      devices[i].lastHeard = currentMillis;
      return;
    }
  }

  if (numDevices < 10) {
    devices[numDevices++] = {senderID, senderReading, currentMillis};
  }
}

void broadcastReadings() {
  memset(packetBuffer, 0, PACKET_SIZE);
  packetBuffer[0] = 0xF0;
  packetBuffer[1] = LIGHT_UPDATE_PACKET;
  packetBuffer[2] = deviceID;
  packetBuffer[3] = masterState ? 1 : 0;
  packetBuffer[5] = (analogValue >> 8) & 0xFF;
  packetBuffer[6] = analogValue & 0xFF;
  packetBuffer[13] = 0x0F;

  udp.beginPacketMulticast(multicastAddress, localUdpPort, WiFi.localIP());
  udp.write(packetBuffer, PACKET_SIZE);
  udp.endPacket();
}

void determineMaster() {
  uint16_t maxReading = analogValue;
  uint8_t masterID = deviceID;
  unsigned long now = millis();

  for (int i = 0; i < numDevices; i++) {
    if (now - devices[i].lastHeard > 5000) {
      for (int j = i; j < numDevices - 1; j++) devices[j] = devices[j + 1];
      numDevices--; i--;
    } else if (devices[i].reading > maxReading) {
      maxReading = devices[i].reading;
      masterID = devices[i].deviceID;
    }
  }

  masterState = (masterID == deviceID);
}

void resetSwarm() {
  masterState = false;
  digitalWrite(onboardLedPin, HIGH);
  delay(100);
  ESP.restart();
}
