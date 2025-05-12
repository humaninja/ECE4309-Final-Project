#include <WiFi.h> //Reads button data with http push sensor
#define button 23
// WiFi credentials
const char* ssid = "mulberry";
const char* password = "9098370903";

// PRTG server details
const char* server = "192.168.1.75"; //Laptop's IP address
const int port = 5050;               //Default port for PRTG HTTP push
const char* token = "93E47C77-81B0-4B5E-A5B4-9EFD56D0F53A"; //Unique sensor token

WiFiClient client;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  int sensorValue = digitalRead(button);
  Serial.println("Sensor Reading OK.");

  String sensorMessage = "Sensor Reading OK.";
  if (client.connect(server, port)) {
    Serial.println("Connected to PRTG server.");

    client.print("GET /");
    client.print(token);
    client.print("?value=");
    client.print(sensorValue);
    client.print("&text=");
    client.print(sensorMessage);
    client.println(" HTTP/1.1");

    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();

    Serial.println("Data sent.");
  } else {
    Serial.println("Connection failed.");
  }

  client.stop();

  delay(10000);
}