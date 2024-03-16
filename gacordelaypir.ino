#include <DFPlayerMini_Fast.h>
#include <WiFi.h>
#include <BasicTimer.h>

const int buttonPin = 4;
const int relayPin = 21;
const int pirPin1 = 22;
const int pirPin2 = 23;

DFPlayerMini_Fast myMP3;
BasicTimer timer;

const char *ssid = "PROJECT-CURUG";
const char *password = "12345678";
IPAddress serverIP(192, 168, 4, 1);
const int serverPort = 80;

int Relay = 0;
int pirState = 0;

static bool cooldownActive = false;
const unsigned long cooldownTime = 600000;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);
  if (!myMP3.begin(Serial2)) {
    Serial.println(F("Gagal Komunikasi Serial2"));
    while (true);
  }
  Serial.println(F("Memulai DFPlayer."));
  myMP3.volume(30);

  pinMode(pirPin1, INPUT);
  pinMode(pirPin2, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

timer.begin();
}

void loop() {
  int pirValue1 = digitalRead(pirPin1);
  int pirValue2 = digitalRead(pirPin2);
  int buttonState = !digitalRead(buttonPin);

  Serial.print(pirValue1);
  Serial.print("|");
  Serial.println(pirValue2);

  if ((pirValue1 == HIGH || pirValue2 == HIGH) && !cooldownActive) {
    pirState = 1;
    Relay = 1;
    delay(100);
    myMP3.play(1);
    delay(2000);
    digitalWrite(relayPin, LOW);
    cooldownActive = true;
    timer.setTimeout(cooldownTime);
    Serial.println("Ada Gerakan, pirState = 1, Memutar Suara dan Menyalakan Relay");
  }

  if (pirState == 1 && timer.hasExpired()) {
    pirState = 0;
    digitalWrite(relayPin, HIGH);
    Serial.println("Cooldown selesai. pirState = 0");
    cooldownActive = false;
    timer.begin();
  }

  if (pirState && Relay == 1) {
    digitalWrite(relayPin, LOW);
    Serial.println("Relay tetap menyala");
  }
  
  if (pirState == 0){
    Relay = 0;
    digitalWrite(relayPin, HIGH);
    Serial.println("Tidak ada gerakan, Mematikan relay dan reset state");
  }

  if (buttonState == 1){
    sendHttpRequest("/meja1");
    Serial.println("Tombol Tekan! Mengirim tanda Meja 1.");
  }
  delay(1000);
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