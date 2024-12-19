#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char *ssid = "Tp";  // Your WiFi SSID, no password
const char *serverURL = "http://192.168.43.184/dht11_project/test_data.php";  // Replace with your server IP

void setup() {
  Serial.begin(9600);
  Serial.println("hii");
  connectWiFi();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Check for incoming data from Arduino
    if (Serial.available() > 0) {
      Serial.println("Data available from Arduino");
      String arduinoData = Serial.readStringUntil('\n');
      Serial.println("Arduino Data: " + arduinoData); // Add this line for debugging
      sendToDatabase(arduinoData);
    }
  } else {
    Serial.println("Not connected to WiFi");
    connectWiFi();
  }
}


void connectWiFi() {
  WiFi.begin(ssid);
  Serial.println("Connecting to WiFi");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi");
    Serial.println("\nhii");
    
  } else {
    Serial.println("\nFailed to connect to WiFi");
  }
}

void sendToDatabase(String data) {
  Serial.print("Sending data to database: ");
  Serial.println("hii");
  Serial.println(data);

  WiFiClient client;

  HTTPClient http;
  bool beginResult = http.begin(client, serverURL);  // Store the result of begin() for debugging
  Serial.println("HTTP Client Begin: " + String(beginResult));

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST(data);
  Serial.println("HTTP Code: " + String(httpCode));

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.print("Response: ");
    Serial.println(payload);
  } else {
    Serial.println("HTTP request failed");
  }

  http.end();
}
