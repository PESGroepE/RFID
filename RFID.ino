#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>

#define SS_PIN D2 // SDA pin
#define RST_PIN D0 // Pin verbonden met RST

MFRC522 rfid(SS_PIN, RST_PIN);

const char* ssid = "pigroep5"; // SSID van het WiFi-netwerk
const char* pass = "pigroep5"; // Wachtwoord van het WiFi-netwerk
const char* host = "10.0.10.1"; // IP-adres van de Pi
const int port = 8080; // Poort van de server

/**
 * @brief Setup.
 * 
 * Initialisatie van seriële communicatie, SPI en de MFRC522 RFID-module.
 * Verbinding maken met WiFi-netwerk.
 */
void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  Serial.println(F("Starting..."));

  connectWiFi();
  
  Serial.println("Now scanning for RFID cards!");
}

/**
 * @brief Loop.
 * 
 * Als er een nieuwe RFID-kaart wordt gedetecteerd, wordt de ID uitgelezen en doorgestuurd naar de server die op de Pi draait.
 */
void loop() {
  // Reset loop als er geen nieuwe kaart wordt gedetecteerd
  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (!rfid.PICC_ReadCardSerial())
    return;

  Serial.println(F("A card has been detected."));
  Serial.println(F("The NUID tag in hex is:"));
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  sendHttpRequest();
}

/**
 * @brief Functie om een HTTP-verzoek naar de server te sturen met de RFID-gegevens.
 */
void sendHttpRequest() {
  WiFiClient client;

  if (!client.connect(host, port)) {
    Serial.println(F("Failed to connect to server"));
    return;
  }

  // Maak HTTP-verzoek met tagdata
  String url = "/rfid/tag?data=";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      url += "0";
    }
    url += String(rfid.uid.uidByte[i], HEX);
  }

  // Stuur HTTP-verzoek
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(10);
  client.stop();

  Serial.println(F("HTTP request sent to server"));
}

/**
 * @brief Functie om verbinding te maken met het WiFi-netwerk.
 * 
 * Deze functie probeert verbinding te maken met het WiFi-netwerk en wacht tot de verbinding tot stand is gebracht.
 */
void connectWiFi() {
  WiFi.begin(ssid, pass); // Probeer verbinding te maken met het WiFi-netwerk.
  Serial.println("Connecting to Pi WiFi");
  while (WiFi.status() != WL_CONNECTED) // Wacht tot verbonden.
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

/**
 * @brief Functie om gegevens in hexadecimale vorm af te drukken.
 * 
 * @param buffer Een array met bytes die afgedrukt moeten worden.
 * @param bufferSize De grootte van de buffer.
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}