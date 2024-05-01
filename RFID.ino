#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>

#define SS_PIN D2 //sda pin
#define RST_PIN D0 //pin verbonden met rst

MFRC522 rfid(SS_PIN, RST_PIN);

const char* host = "groep5-A.local"; //pi adres
const int port = 8080; //port van server

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  Serial.println(F("Starting..."));

  WiFi.begin("NSELab", "NSELabWiFi"); // probeer te verbinden met de wifi
  Serial.println("Connecting to NSE WiFi");
  while (WiFi.status() != WL_CONNECTED) // wacht totdat hij is verbonden
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Now scanning for RFID cards!");
}

void loop() {
  // reset loop wanneer er geen nieuwe kaart wordt gelezen.
  if (!rfid.PICC_IsNewCardPresent())
    return;


  if (!rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  Serial.println(F("A card has been detected."));
  Serial.println(F("The NUID tag is:"));
  Serial.print(F("In hex: "));
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();
  Serial.print(F("In dec: "));
  printDec(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  rfid.PICC_HaltA();

  rfid.PCD_StopCrypto1();

  sendHttpRequest();

  delay(500);
}

void sendHttpRequest() {
  WiFiClient client;

  if (!client.connect(host, port)) {
    Serial.println(F("Failed to connect to server"));
    return;
  }

  // maak http request met tag data
  String url = "/tag?data=";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      url += "0";
    }
    url += String(rfid.uid.uidByte[i], HEX);
  }

  // stuur http request
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(10);
  client.stop();

  Serial.println(F("HTTP request sent to server"));
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(' ');
    Serial.print(buffer[i], DEC);
  }
}