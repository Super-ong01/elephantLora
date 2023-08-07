// Import libraries
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <LoRa.h>
//#include "EEPROM.h"
#include "time.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600*7;
const int   daylightOffset_sec = 3600*7;
char timeHour[3];
char timeMinute[3];

//int addr_ph = 0;
//int addr_ec = 1;

// Replace with your network credentials
const char* ssid     = "A_CHARTTHAI_2.4G";
const char* password = "ZAQ1XSW2";

String serverName = "https://somsri.sltraininggroup.com/api/Values/moontemp/";

// LoRa Module pin definition
// define the pins used by the LoRa transceiver module
#define ss 5
#define rst 15
#define dio0 2

// Initialize variables to get and save LoRa data
int rssi;

String message;              // outgoing message
byte msgCount = 0;            // count of outgoing messages
//byte localAddress_master = 0xBB;     // address of this device
//byte localAddress_water = 0x01;     // address of this device
//byte localAddress_air = 0x02;     // address of this device
//byte destination = 0xAA;      // destination to send to

String localAddress_master = "1";     // address of this device
String localAddress_water = "2";     // address of this device
String localAddress_air = "3";     // address of this device
String destination = "0xAA";      // destination to send to

String loRaMessage;
String readingID;
String Local_ID;

String temperature_air;
String humidity;
String light;
String batteryLevel_air;

String temperature_water;
String batteryLevel_water;
String ph;
String ec;

//float ph_value;
//float ec_value;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

bool Send_API_AIR = false;
bool Send_API_WATER = false;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
//  ph_value = EEPROM.read(addr_ph);
//  ec_value = EEPROM.read(addr_ec);
//  Serial.print("PH : "); Serial.println(ph_value);
//  Serial.print("EC : "); Serial.println(ec_value);
//  EEPROM.write(addr_ph, 6.00);
//  EEPROM.write(addr_ec, 1.7);
//  EEPROM.commit();
//  ph_value = EEPROM.read(addr_ph);
//  ec_value = EEPROM.read(addr_ec);
//  Serial.print("PH : "); Serial.println(ph_value);
//  Serial.print("EC : "); Serial.println(ec_value);

  // Initialize LoRa
  //replace the LoRa.begin(---E-) argument with your location's frequency
  LoRa.setPins(ss, rst, dio0);
  LoRa.begin(433E6);
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }
  // Change sync word (0xF3) to match the sender
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xAA);
  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("LoRa Initializing OK!");

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //  server.begin();

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  getLocalTime();
}

void loop() {
  if (Send_API_AIR == true) {
    send_API_AIR();
    Send_API_AIR = false;
  }
  if (Send_API_WATER == true) {
    send_API_WATER();
    Send_API_WATER = false;
  }
  
  // control output
  if (temperature_water > "25") {

  } else if (temperature_water < "24") {

  }
}

void getLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
//  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
//  Serial.print("Day of week: ");
//  Serial.println(&timeinfo, "%A");
//  Serial.print("Month: ");
//  Serial.println(&timeinfo, "%B");
//  Serial.print("Day of Month: ");
//  Serial.println(&timeinfo, "%d");
//  Serial.print("Year: ");
//  Serial.println(&timeinfo, "%Y");
//  Serial.print("Hour: ");
//  Serial.println(&timeinfo, "%H");
//  Serial.print("Hour (12 hour format): ");
//  Serial.println(&timeinfo, "%I");
//  Serial.print("Minute: ");
//  Serial.println(&timeinfo, "%M");
//  Serial.print("Second: ");
//  Serial.println(&timeinfo, "%S");

//  Serial.println("Time variables");
//  char timeHour[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  Serial.print(timeHour);Serial.print(":");
//  char timeMinute[3];
  strftime(timeMinute, 3, "%M", &timeinfo);
  Serial.println(timeMinute);

  
//  char timeWeekDay[10];
//  strftime(timeWeekDay, 10, "%A", &timeinfo);
//  Serial.println(timeWeekDay);
  Serial.println();
}

void sendMessage(String message) {
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize) {
  Serial.print("Received packet '");
  // read packet
  while (LoRa.available()) {
    String LoRaData = LoRa.readString();
    // LoRaData format: readingID/temperature&humidity$light#batterylevel
    // String example: 1/27.43&654$555#95.34
    Serial.print(LoRaData);
    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.print(LoRa.packetRssi());
    Serial.print(", Snr: ");
    Serial.println(LoRa.packetSnr());

    int pos1 = LoRaData.indexOf('@');
    int pos2 = LoRaData.indexOf('/');
    Local_ID = LoRaData.substring(pos1 + 1, pos2);
    //    Serial.print("     "); Serial.print(Local_ID);
    if (Local_ID == "2") {
      Serial.println("Water ID '2'");
      //          int pos1 = LoRaData.indexOf('/');
      int pos3 = LoRaData.indexOf('&');
      int pos4 = LoRaData.indexOf('$');
      int pos5 = LoRaData.indexOf('#');
      //          readingID = LoRaData.substring(pos2+1, pos3);
      ph = LoRaData.substring(pos2 + 1, pos3);
      ec = LoRaData.substring(pos3 + 1, pos4);
      temperature_water = LoRaData.substring(pos4 + 1, pos5);
      batteryLevel_water = LoRaData.substring(pos5 + 1, LoRaData.length());
      Serial.print("PH : "); Serial.println(ph);
      Serial.print("EC : "); Serial.println(ec);
      Serial.print("Temp : "); Serial.println(temperature_water);
      Serial.print("BATT : "); Serial.println(batteryLevel_water);
      Send_API_WATER = true;
    }
    else if (Local_ID == "3") {
      Serial.println("Air ID '3'");
      int pos3 = LoRaData.indexOf('&');
      int pos4 = LoRaData.indexOf('$');
      int pos5 = LoRaData.indexOf('#');
      //          readingID = LoRaData.substring(pos2+1, pos3);
      temperature_air = LoRaData.substring(pos2 + 1, pos3);
      humidity = LoRaData.substring(pos3 + 1, pos4);
      light = LoRaData.substring(pos4 + 1, pos5);
      batteryLevel_air = LoRaData.substring(pos5 + 1, LoRaData.length());
      Serial.print("Temp : "); Serial.println(temperature_air);
      Serial.print("Humid : "); Serial.println(humidity);
      Serial.print("Light : "); Serial.println(light);
      Serial.print("BATT : "); Serial.println(batteryLevel_air);
      Send_API_AIR = true;
    }
  }
}

void send_API_AIR() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    //      String T18 = String(Temp_DS18B20);
    //      T18.replace(".","@");
    String ID = String(Local_ID);
    ID.replace(".", "@");
    String B_P = String(batteryLevel_air);
    B_P.replace(".", "@");
    String L17 = String(light);
    L17.replace(".", "@");
    String T23 = String(temperature_air);
    T23.replace(".", "@");
    String H23 = String(humidity);
    H23.replace(".", "@");

    String text_All = ID + "|" + T23 + "|" + H23 + "|" + L17 + "|" + B_P;

    String serverPath = serverName + text_All;//เอาค่า sensor มาใส่ตรงนี้ครับ

    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}

void send_API_WATER() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    //      String T18 = String(Temp_DS18B20);
    //      T18.replace(".","@");
    String ID = String(Local_ID);
    ID.replace(".", "@");
    String B_P = String(batteryLevel_water);
    B_P.replace(".", "@");
    String PH = String(ph);
    PH.replace(".", "@");
    String EC = String(ec);
    EC.replace(".", "@");
    String T18 = String(temperature_water);
    T18.replace(".", "@");

    String text_All = ID + "|" + PH + "|" + EC + "|" + T18 + "|" + B_P;

    String serverPath = serverName + text_All;//เอาค่า sensor มาใส่ตรงนี้ครับ

    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}
