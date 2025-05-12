#include <WiFi.h>
#include <esp_system.h>

#define button1 22
#define button2 23
#define securityButton 21
//WiFi credentials
const char* ssid = ""; //wifi login 
const char* password = ""; //wifi password

//PRTG server details
const char* server = "192.168.1.75"; //laptop IP
const int port = 5050; //default port
const char* token = "70128D29-1A9D-428B-A429-97186AD8AF60"; //sensor token

WiFiClient client;
QueueHandle_t telemetryQueue;

//Struct to hold telemetry data
typedef struct {
  float cpuLoad;
  int freeHeap;
  unsigned long uptime;
  char securityEvent[64];
  int button;
} telemetry_t;
telemetry_t data;
//update telemetry data
void telemetryTask(void* pvParameters) {

  while (true) {
    data.cpuLoad = (float)uxTaskGetNumberOfTasks();
    data.freeHeap = esp_get_free_heap_size();
    data.uptime = millis() / 1000;
    xQueueOverwrite(telemetryQueue, &data);
    if (detectUnexpectedReset() || digitalRead(securityButton)) {
      strcpy(data.securityEvent, "Unexpected reset detected");
    }
    else
    {
      strcpy(data.securityEvent, "No security flag");
    }
    vTaskDelay(pdMS_TO_TICKS(2500));
  }
}

//send data to network
void networkTask(void* pvParameters) {

  while (true) {
    if (xQueuePeek(telemetryQueue, &data, portMAX_DELAY)) {
      if (client.connect(server, port)) {

        String xmlPayload = "<prtg>";
        xmlPayload += "<result><channel>CPULoad</channel><value>" + String(data.cpuLoad) + "</value><float>1</float></result>";
        xmlPayload += "<result><channel>FreeHeap</channel><value>" + String(data.freeHeap) + "</value></result>";
        xmlPayload += "<result><channel>Uptime</channel><value>" + String(data.uptime) + "</value></result>";
        xmlPayload += "<result><channel>Button</channel><value>" + String(data.button) + "</value></result>";
        xmlPayload += "<text>" + String(data.securityEvent) + "</text>";
        xmlPayload += "</prtg>";

        client.println("POST /" + String(token) + " HTTP/1.1");
        client.println("Host: " + String(server));
        client.println("Content-Type: application/xml");
        client.print("Content-Length: "); client.println(xmlPayload.length());
        client.println("Connection: close\n");
        client.println(xmlPayload);

        client.stop();
      }
    }
    data.button = 0;
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}
//security check, iffy
bool detectUnexpectedReset()
{
  esp_reset_reason_t reason = esp_reset_reason();
  return (reason != ESP_RST_POWERON && reason != ESP_RST_SW);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");

  telemetryQueue = xQueueCreate(1, sizeof(telemetry_t));

  xTaskCreate(telemetryTask, "Telemetry Task", 4096, NULL, 1, NULL);
  xTaskCreate(networkTask, "Network Task", 8192, NULL, 1, NULL);

}
//polling button presses
void loop() {
    if (digitalRead(button1))
    {
      data.button = 1;
    }
    else if (digitalRead(button2))
    {
      data.button = 2;
    }
}