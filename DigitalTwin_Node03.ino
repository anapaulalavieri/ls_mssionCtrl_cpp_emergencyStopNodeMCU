// *************************************************************************
// *               |               ACCENTURE                |              *
// *               |           Liquid Studios BR            |              *
// *************************************************************************
// * Program      : Digital Twin IoT - Connection between prototype and SCP*
// *                account and Digital Twin                               *
// * Creation date: 12-Jun-2023                                            *
// * Responsible  : Accenture Liquid Studio SP                             *
// * Description  : RFID tag reading and pressing a button on the Digital  *
// *                Twin circuit and sending an action to SAP BTP          *
// *************************************************************************
// * Vr. |Request    | Responsible        | Description                    *
// *-----------------------------------------------------------------------*
// * 001 |SX4KXXXXXX | Complete name      | Initial version                *
// *************************************************************************

// Libraries
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>

// Define pins
#define buttonPin 0  //D3
#define pinSS 2      //D4
#define pinReset 15  //D8
#define ledRed 4     //D2
#define ledGreen 5   //D1

// Constants - WiFi configuration
const char* cId = "Nome_da_Rede";
const char* cPassword = "Senha_do_WIFI";

// Constants - SAP Cloud configuration
const char* cHost = "brazilsaopauloaccenturetechnologysap.apimanagement.hana.ondemand.com";
const int iHttpsPort = 443;

int iReturnRFID = 0;
int iReturnButton = 0;

// Create MFRC522 instance
MFRC522 mfrc522(pinSS, pinReset);

// Setup code here (run once)
void setup() {

  // Initialize pins, serial and log messages
  SPI.begin();
  Serial.begin(9600);
  Serial.println();
  Serial.println("Process begin");
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(buttonPin, OUTPUT);

  // Initialize Wi-Fi connection
  Serial.println("Starting WiFi connection");
  WiFi.begin ( cId, cPassword );
  bool ledFlag = false;
  while ( WiFi.status() != WL_CONNECTED ) {
    if (ledFlag) {
      digitalWrite(ledRed, HIGH);
      digitalWrite(ledGreen, HIGH);
    } else {
      digitalWrite(ledRed, LOW);
      digitalWrite(ledGreen, LOW);
    }
    ledFlag = ! ledFlag;
    delay ( 500 );
    Serial.print(".");
  }


  // Set log message in serial after connection
  digitalWrite(ledRed, HIGH);
  digitalWrite(ledGreen, HIGH);
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(cId );
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Prepare hardware to next read
  mfrc522.PCD_Init();
  Serial.println();
  Serial.println("Waiting RFID scan or Button press...");

  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, HIGH);
}

// Main code (run repeatedly)
void loop() {

  // Check if any card was read
  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial()) {
    //    return;
  } else {
    // Read of RFID detected
    String sContent = "";
    char cTag[100] = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      sContent.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    sContent.toUpperCase();
    sContent.toCharArray(cTag, 100);
    Serial.print("Original RFID detected: ");
    Serial.println(cTag);

    // Convert Tag code to string and remove space character from first position
    String sTag = "";
    sTag = cTag;

    sTag.remove(7, 12);
    Serial.print("RFID detected: ");
    Serial.println(sTag);

    if (sTag == "278360A") {
      digitalWrite(ledRed, LOW);
      fnPOSTRequest("{\"EQUIP_ID\":1,\"EQUIP_STATUS\":0}", 0);
      
      if (iReturnRFID == 0) {
        digitalWrite(ledRed, LOW);
        digitalWrite(ledGreen, HIGH);
      } else {
        digitalWrite(ledRed, HIGH);
        digitalWrite(ledGreen, LOW);
      }
    }
  }

  byte actionButton = digitalRead(buttonPin);
  if (actionButton == HIGH) {
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledGreen, LOW);
    
    fnPOSTRequest("{\"EQUIP_ID\":1,\"EQUIP_STATUS\":1}", 1);
    
    if (iReturnButton == 0) {
      digitalWrite(ledRed, HIGH);
      digitalWrite(ledGreen, LOW);
    } else {
      digitalWrite(ledRed, LOW);
      digitalWrite(ledGreen, HIGH);
    }
  }
}

void fnPOSTRequest(String sPayload, int typeAction) {
  
  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  client.setInsecure();

  // Set URL for connection
  String sUrl = "https://brazilsaopauloaccenturetechnologysap.apimanagement.hana.ondemand.com:443/EquipStatus";
  Serial.print("Payload: ");
  Serial.println(sPayload);

  // Connect to SAP Server
  Serial.print("Connecting to SAP Cloud server: ");
  Serial.println(cHost);

  // Connect to SAP cloud
  if (client.connect(cHost, iHttpsPort)) {

    // Using HTTP/1.0 to send information
    Serial.println("Sending request");
    client.print(String("POST ") + sUrl + " HTTP/1.0\r\n" +
                 "Host: " + cHost + "\r\n" +
                 "Content-Type: application/json;charset=utf-8\r\n" +
                 "Content-Length: " + sPayload.length() + "\r\n\r\n" +
                 sPayload + "\r\n\r\n");

    // Set success message in the serial log
    String sResponse = client.readStringUntil('\n');
    Serial.println("Message sent");
    Serial.print("Response: ");
    Serial.print(sResponse);
    Serial.println(" ");
    if (typeAction == 0){
      iReturnRFID = 0;
    } else {
      iReturnButton = 0;
    }
    
  } else {
    // Set error message in the serial log
    Serial.println("Connection failed");
    if (typeAction == 0){
      iReturnRFID = 1;
    } else {
      iReturnButton = 1;
    }
  }

  // Prepare hardware to next read
  delay(800);
  Serial.println();
  Serial.println("Waiting command");
}

void fnGETRequest() {

}
