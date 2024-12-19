#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <DHT.h>

#define DHTPIN 5
#define MQ2PIN A0
#define MQ2D2PIN 8
#define SOUNDANALOG A1
#define SOUNDD3PIN 12
#define MQ9A2PIN A2
#define MQ9D2PIN 10
#define MQ8PIN A7
#define MQ8D2PIN 30
#define DHTTYPE DHT11

SoftwareSerial espSerial(3, 2);
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 20, 4);

int mqAnalog;
int mqDigital;
int soundAnalog;
int soundDigital;
int mq9Analog;
int mq9Digital;
int mq8Analog;
int mq8Digital;
float temperature, humidity;
float dustDensityPM25;
float dustDensityPM10;

// Dust Sensor Pins and Variables
int measurePinPM25 = 11; // Pin for PM2.5 Dust Sensor
int measurePinPM10 = 12; // Pin for PM10 Dust Sensor
int ledPowerPM25 = 43;   // Connect PM2.5 LED driver pin to Arduino D23
int ledPowerPM10 = 45;   // Connect PM10 LED driver pin to Arduino D24

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  dht.begin();
  lcd.init();
  lcd.backlight();

  pinMode(ledPowerPM25, OUTPUT);
  pinMode(ledPowerPM10, OUTPUT);
}

void loop() {
  delay(2000);

  readDHTData();
  readMQ2Data();
  readSoundData();

  updateLCD();

  delay(1000); // Wait for 1 second

  readMQ9Data();
  readMQ8Data();

  updateLCDNextSlide();

  sendToESP();
  delay(2000);
}

void readDHTData() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  }
}

void readMQ2Data() {
  mqAnalog = analogRead(MQ2PIN);
  mqDigital = digitalRead(MQ2D2PIN);
}

void readSoundData() {
  soundAnalog = analogRead(SOUNDANALOG);
  soundDigital = digitalRead(SOUNDD3PIN);
}

void readMQ9Data() {
  mq9Analog = analogRead(MQ9A2PIN);
  mq9Digital = digitalRead(MQ9D2PIN);
}

void readMQ8Data() {
  mq8Analog = analogRead(MQ8PIN);
  mq8Digital = digitalRead(MQ8D2PIN);
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temperature: ");
  lcd.print(temperature);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print(" %");

  lcd.setCursor(0, 2);
  lcd.print("MQ2 Analog: ");
  lcd.print(mqAnalog);

  lcd.setCursor(0, 3);
  lcd.print("Sound Analog: ");
  lcd.print(soundAnalog);
}

void updateLCDNextSlide() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MQ9 Analog: ");
  lcd.print(mq9Analog);

  lcd.setCursor(0, 1);
  lcd.print("MQ8 Analog: ");
  lcd.print(mq8Analog);

  lcd.setCursor(0, 2);
  lcd.print("PM2.5: ");
  lcd.print(dustDensityPM25);
  lcd.print(" ug/m3");

  lcd.setCursor(0, 3);
  lcd.print("PM10: ");
  lcd.print(dustDensityPM10);
  lcd.print(" ug/m3");
}

float readDustDensity(int measurePin) {
  digitalWrite(ledPowerPM25, LOW); // Turn on the LED for the respective dust sensor
  digitalWrite(ledPowerPM10, LOW);
  delayMicroseconds(280);

  float voMeasured = analogRead(measurePin); // Read dust value

  delayMicroseconds(40);
  delayMicroseconds(9680);

  // Map 0-5V to 0-1023 integer values and recover voltage
  float calcVoltage = voMeasured * (5.0 / 1024.0);

  // Linear equation derived from http://www.howmuchsnow.com/arduino/airquality/
  // Chris Nafis (c) 2012
  float dustDensity = 170 * calcVoltage - 0.1;

  digitalWrite(ledPowerPM25, HIGH); // Turn off the LED
  digitalWrite(ledPowerPM10, HIGH);

  return dustDensity;
}

void sendToESP() {
  // Read PM2.5 Dust Density
  dustDensityPM25 = readDustDensity(measurePinPM25);

  delay(1000); // Wait for 1 second

  // Read PM10 Dust Density
  dustDensityPM10 = readDustDensity(measurePinPM10);

  // Construct data string
  String postData = "temperature=" + String(temperature) +
                    "&humidity=" + String(humidity) +
                    "&mq2_analog=" + String(mqAnalog) +
                    "&mq2_digital=" + String(mqDigital) +
                    "&sound_analog=" + String(soundAnalog) +
                    "&sound_digital=" + String(soundDigital) +
                    "&mq9_analog=" + String(mq9Analog) +
                    "&mq9_digital=" + String(mq9Digital) +
                    "&mq8_analog=" + String(mq8Analog) +
                    "&mq8_digital=" + String(mq8Digital) +
                    "&dust_density_pm25=" + String(dustDensityPM25) +
                    "&dust_density_pm10=" + String(dustDensityPM10);

  Serial.println("Sending data to ESP: " + postData);
  espSerial.println(postData);

  // Wait for response from ESP8266
  while (espSerial.available() > 0) {
    char responseChar = espSerial.read();
    Serial.print(responseChar);
  }
}
