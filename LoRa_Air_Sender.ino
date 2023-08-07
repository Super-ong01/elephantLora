// Libraries for LoRa Module
#include <SPI.h>            
#include <LoRa.h>

//DS18B20 libraries
//#include <OneWire.h>
//#include <DallasTemperature.h>

#include <Wire.h>
#include <BH1750.h>

#include <Adafruit_AHTX0.h>

// LoRa Module pin definition
// define the pins used by the transceiver module
#define ss 5
#define rst 15
#define dio0 2

// LoRa message variable
String message;
String localAddress = "3";

BH1750 lightMeter;

Adafruit_AHTX0 aht;

// Save reading number on RTC memory
RTC_DATA_ATTR int readingID = 0;

// Define deep sleep options
uint64_t uS_TO_S_FACTOR = 1000000;  // Conversion factor for micro seconds to seconds
// Sleep for 30 minutes = 0.5 hours  = 1800 seconds
uint64_t TIME_TO_SLEEP = 59;

// Data wire is connected to ESP32 GPIO15
//#define ONE_WIRE_BUS 15
// Setup a oneWire instance to communicate with a OneWire device
//OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
//DallasTemperature sensors(&oneWire);

// Moisture Sensor variables
//const int moisturePin = 26;
//const int moisturePowerPin = 12;
//int soilMoisture;

// Temperature Sensor variables
//float tempC;
//float tempF;
float light;
float Temp;
float Humid;

//Variable to hold battery level;
float batteryLevel;
const int batteryPin = 39;

void setup() {
//  pinMode(moisturePowerPin, OUTPUT);
  
  // Start serial communication for debugging purposes
  Serial.begin(115200);
  Wire.begin();
  // Enable Timer wake_up
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  // Start the DallasTemperature library
//  sensors.begin();
  
  // Initialize LoRa
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //note: the frequency should match the sender's frequency
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  LoRa.setPins(ss, rst, dio0);
  Serial.println("initializing LoRa");
  
  int counter = 0;  
  while (!LoRa.begin(433E6) && counter < 30) {
    Serial.print(".");
    counter++;
    delay(500);
  }
  if (counter == 30) {
    // Increment readingID on every new reading
    readingID++; 
    // Start deep sleep
    Serial.println("Failed to initialize LoRa. Going to sleep now");
    esp_deep_sleep_start();
  }
  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xAA);
  Serial.println("LoRa initializing OK!");

  getReadings();
  Serial.print("Battery level = "); 
  Serial.println(batteryLevel, 2);
  Serial.print("Light LUX = ");
  Serial.println(light);
  Serial.print("Temperature Celsius = ");
  Serial.println(Temp);
  Serial.print("Humidity = ");
  Serial.println(Humid);
  Serial.print("Reading ID = ");
  Serial.println(readingID);
  
  sendReadings();
  Serial.print("Message sent = ");
  Serial.println(message);
  message = "";

  // Increment readingID on every new reading
  readingID++;
  
  // Start deep sleep
  Serial.println("DONE! Going to sleep now.");
  esp_deep_sleep_start(); 
} 

void loop() {

}

void getReadings() {
  // Measure Light
  lightMeter.begin();
  float lux = lightMeter.readLightLevel();
  light = lux;

  // Measure temperature, humid
  aht.begin();
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  Temp = temp.temperature;
  Humid = humidity.relative_humidity;

  //Measure battery level
//  batteryLevel = map(analogRead(batteryPin), 0.0f, 4095.0f, 0, 100);
  float analog_volt = 0;
  for (int i = 0; i < 20; i++)
  {
    analog_volt += analogRead(batteryPin);
    delay(50);
  }
  float batteryVolt = ((analog_volt/20)/4095)*4.2;
  analog_volt = 0;
  batteryLevel = (batteryVolt/4.2)*100;
}

void sendReadings() {
  // Send packet data
  // Send temperature in Celsius
  message = String(readingID) + "@" + String(localAddress) + "/" + String(Temp) + "&" + 
            String(Humid) + "$" + String(light) + "#" + String(batteryLevel);

  delay(1000);
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
}
