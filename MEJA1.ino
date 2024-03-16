#include <WiFi.h>
#include <DFRobotDFPlayerMini.h>

const char* ssid = "PROJECT-CURUG";
const char* password = "12345678";
const char* serverIP = "192.168.4.1";
const int serverPort = 80;

const int pirPin = 2;
const int LED_PIN = 4;

DFRobotDFPlayerMini myMP3;

bool isPlaying = false; 
int pirState = LOW;  // Initialize to LOW
int lastPirState = LOW;  // Initialize to LOW
unsigned long lastMotionTime = 0; 
const unsigned long motionDelay = 10000;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);  // Add missing semicolon
  pinMode(pirPin, INPUT);
  pinMode(LED_PIN, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  myMP3.begin(Serial2);  // Initialize DFPlayerMini with the correct Serial
  myMP3.volume(30);
  Serial.println(F("DFPlayer Mini initialized."));
}

void loop() {
  int pirValue = digitalRead(pirPin);  // Rename pirState to pirValue
  
  if (pirValue != lastPirState) {
    lastPirState = pirValue;

    if (pirValue == HIGH && !isPlaying && (millis() - lastMotionTime >= motionDelay)) {
      digitalWrite(LED_PIN, HIGH);
      sendHttpRequest("/meja2?meja=on");
      Serial.println("Motion detected! Turning on Meja 1.");
      myMP3.play(1);
      isPlaying = true;
      lastMotionTime = millis();
    }
  }

  if (isPlaying && myMP3.available()) {
    if (myMP3.readType() == DFPlayerPlayFinished) {
      Serial.println(F("Audio playback finished."));
      isPlaying = false;
    }
  }

  if ((pirValue == LOW || (millis() - lastMotionTime >= motionDelay)) && !isPlaying) {
    digitalWrite(LED_PIN, LOW);
    sendHttpRequest("/meja2?meja=off");
    Serial.println("Motion stopped. Turning off Meja 1.");
  }

  delay(100);
}

void sendHttpRequest(String endpoint) {
  WiFiClient client;
  if (client.connect(serverIP, serverPort)) {
    Serial.println("Connected to server");
    client.print(String("GET ") + endpoint + " HTTP/1.1\r\n" +
                 "Host: " + serverIP + "\r\n" +
                 "Connection: close\r\n\r\n");
    delay(10);
    client.stop();
    Serial.println("Request sent");
  } else {
    Serial.println("Unable to connect to server");
  }
}
