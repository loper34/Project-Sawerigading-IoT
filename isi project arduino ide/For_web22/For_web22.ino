#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>         
#include <ArduinoJson.h>        
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>


#define DHTPIN 5
#define DHTTYPE DHT11
#define ONE_WIRE_BUS 4
DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define DHTPIN1 25
#define DHTTYPE1 DHT11
#define ONE_WIRE_BUS1 26
DHT dht1(DHTPIN1, DHTTYPE1);
OneWire oneWire1(ONE_WIRE_BUS1);
DallasTemperature sensors1(&oneWire1);

LiquidCrystal_I2C lcd(0x27, 16, 2);


char ssid[] = "CONGKI";     
char pass[] = "prsse24us";  


const char* serverName = "http://172.20.10.5:5000/update_sensor";


const int AirValue = 2620;  
const int WaterValue = 1180; 
const int AirValue1 = 2620; 
const int WaterValue1 = 1180; 

int soilMoistureValue = 0;
int soilmoist = 0;
float humi, temp, tempsoil;

int soilMoistureValue1 = 0;
int soilmoist1 = 0;
float humi1, temp1, tempsoil1;

int SP_LOW = 70;
int SP_HIGH = 80;
int pump = 12;
int pump1 = 14;

#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;

void read_DHT11() {
  humi = dht.readHumidity();
  temp = dht.readTemperature();
  if (isnan(humi) || isnan(temp)) {
    Serial.println(F("Failed to read from DHT sensor 1!"));
    humi = 0;   
    temp = 0;  
  }
}

void read_DHT111() {
  humi1 = dht1.readHumidity();
  temp1 = dht1.readTemperature();
  if (isnan(humi1) || isnan(temp1)) {
    Serial.println(F("Failed to read from DHT sensor 2!"));
    humi1 = 0;
    temp1 = 0;
  }
}

void read_SoilMoist() {
  soilMoistureValue = analogRead(A6);
  soilmoist = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  soilmoist = constrain(soilmoist, 0, 100);

  Serial.print("Soil Moisture 1:");
  Serial.print(soilmoist);
  Serial.println("%");
}

void read_SoilMoist1() {
  soilMoistureValue1 = analogRead(A7);
  soilmoist1 = map(soilMoistureValue1, AirValue1, WaterValue1, 0, 100);
  soilmoist1 = constrain(soilmoist1, 0, 100);
}

void read_DS1820() {
  sensors.requestTemperatures();
  tempsoil = sensors.getTempCByIndex(0);
  if (tempsoil == DEVICE_DISCONNECTED_C) {
    Serial.println("Failed to read from DS18B20 sensor 1!");
    tempsoil = -999;
}

void read_DS18201() {
  sensors1.requestTemperatures();
  tempsoil1 = sensors1.getTempCByIndex(0);
  if (tempsoil1 == DEVICE_DISCONNECTED_C) {
    Serial.println("Failed to read from DS18B20 sensor 2!");
    tempsoil1 = -999;
  }
}

// --- SETUP ---
void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2); 
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("= Smart Garden =");
  lcd.setCursor(0, 1);
  lcd.print(" IoT ESP32 GPS  ");
  dht.begin();
  dht1.begin();
  delay(2000);

  pinMode(pump, OUTPUT);
  pinMode(pump1, OUTPUT);
  digitalWrite(pump, HIGH); 
  digitalWrite(pump1, HIGH); 


  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());


  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  Serial.println("GPS Module Reading...");

  
  lcd.clear();
  lcd.print("Mst1=  % T1=  C"); 
  lcd.setCursor(0, 1);
  lcd.print("Mst2=  % T2=  C"); 
}


void loop() {
  read_DHT11();
  read_SoilMoist();
  read_DHT111();
  read_SoilMoist1();

  lcd.setCursor(5, 0); lcd.print(soilmoist); lcd.print("% ");
  lcd.setCursor(13, 0); lcd.print(tempsoil); lcd.print("C ");

  lcd.setCursor(5, 1); lcd.print(soilmoist1); lcd.print("% ");
  lcd.setCursor(13, 1); lcd.print(tempsoil1); lcd.print("C ");


  if (soilmoist < SP_LOW) {
    digitalWrite(pump, LOW); 
  } else if (soilmoist > SP_HIGH) {
    digitalWrite(pump, HIGH); 
  }

  
  if (soilmoist1 < SP_LOW) {
    digitalWrite(pump1, LOW); 
  } else if (soilmoist1 > SP_HIGH) {
    digitalWrite(pump1, HIGH); 
  }



  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  double latitude = gps.location.isValid() ? gps.location.lat() : -1;
  double longitude = gps.location.isValid() ? gps.location.lng() : -1;
  double altitude = gps.altitude.isValid() ? gps.altitude.meters() : -1;
  int satellites = gps.satellites.isValid() ? gps.satellites.value() : -1;


  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");


    StaticJsonDocument<512> doc; 
    doc["device_id"] = "plant_area_1"; 
    
    doc["latitude"] = latitude;
    doc["longitude"] = longitude;
    doc["altitude"] = altitude;
    doc["satellites"] = satellites;

    JsonObject sensor1_obj = doc.createNestedObject("sensor1");
    sensor1_obj["humidity_air"] = humi;
    sensor1_obj["temperature_air"] = temp;
    sensor1_obj["temperature_soil"] = tempsoil;
    sensor1_obj["moisture_soil"] = soilmoist;
    sensor1_obj["pump_status"] = (digitalRead(pump) == LOW) ? "ON" : "OFF";


    JsonObject sensor2_obj = doc.createNestedObject("sensor2");
    sensor2_obj["humidity_air"] = humi1;
    sensor2_obj["temperature_air"] = temp1;
    sensor2_obj["temperature_soil"] = tempsoil1;
    sensor2_obj["moisture_soil"] = soilmoist1;
    sensor2_obj["pump_status"] = (digitalRead(pump1) == LOW) ? "ON" : "OFF";


    String jsonString;
    serializeJson(doc, jsonString);

    Serial.print("Sending JSON: ");
    Serial.println(jsonString);

    int httpResponseCode = http.POST(jsonString);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      Serial.println("Failed to send HTTP POST request to Flask.");
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected. Attempting to reconnect...");
    WiFi.begin(ssid, pass);
  }
  delay(5000);
}