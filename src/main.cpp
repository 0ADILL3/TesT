#include <ModbusMaster.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <StepRunner.h>
#include <ArduinoJson.h>

ModbusMaster node;
WiFiClient ESP32_S3_Client;
PubSubClient client_MQTT(ESP32_S3_Client);

const char* MQTT_server   = "192.168.22.173";
const int   MQTT_port     = 1883;
const char* MQTT_username = "adill";
const char* MQTT_password = "00111100";

float temperature = 0.00;
float humidity    = 0.00;

void connect_MQTT()
{
  if (client_MQTT.connected()) {return;}

  String client_ID = "ESP32-S3-Temperature_and_Humidity-RS-WS-N01-6";

  Serial.println();
  Serial.println((WiFi.status() == WL_CONNECTED) ? "WiFi Connected" : "WiFi Disconnected");
  Serial.print("Connecting to MQTT...");

  if (client_MQTT.connect(client_ID.c_str(), MQTT_username, MQTT_password))
  {
    Serial.println("OK");
    // client_MQTT.subscribe("TesT/ESP32-S3/control");
  }
  else
  {
    Serial.print("Failed connect to MQTT. rc=");
    Serial.println(client_MQTT.state());
    delay(3000);
  }
}

void callback_MQTT(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Topic: ");
  Serial.println(topic);

  String message = "";
  for (int i = 0; i < length; i++) {message += char(payload[i]);}

  Serial.println(message);
}

void publish_MQTT()
{
  JsonDocument doc;

  char payload[128];
  doc["TEMP"] = temperature;
  doc["HUM"] = humidity;
  
  size_t payload_length = serializeJson(doc, payload);

  client_MQTT.publish("TesT/ESP32-S3/data", payload, payload_length);
}

void Temperature_Humidity_sensor()
{
  uint8_t result = node.readHoldingRegisters(0x0000, 2);
  
  if (result == node.ku8MBSuccess)
  {
    humidity = node.getResponseBuffer(0) / 10.0;
    temperature = int16_t(node.getResponseBuffer(1)) / 10.0;
    
    Serial.print("Temp     : ");
    Serial.print(temperature);
    Serial.println(" C");

    Serial.print("Humidity : ");
    Serial.print(humidity);
    Serial.println(" %RH");
  }
  else
  {
    Serial.print("Modbus Error: ");
    Serial.println(result);
  }
}

StepRunner publish_MQTT_task(publish_MQTT, 60000);
StepRunner Temperature_Humidity_sensor_task(Temperature_Humidity_sensor, 5000);

void setup()
{
  Serial.begin(115200);
  
  Serial2.begin(4800, SERIAL_8N1, 16, 17);
  node.begin(1, Serial2);   // Slave ID = 1

  WiFiManager wm;

  if (!wm.autoConnect("ESP32-S3-WiFi_Manager"))
  {
    Serial.println("Failed connect to WiFi");
    ESP.restart();
  }

  Serial.println();
  Serial.println("WiFi Connected");
  Serial.println(WiFi.localIP());

  client_MQTT.setServer(MQTT_server, MQTT_port);
  client_MQTT.setCallback(callback_MQTT);
  client_MQTT.setKeepAlive(120);
}

void loop()
{
  connect_MQTT();
  client_MQTT.loop();
  Temperature_Humidity_sensor_task.run();
  publish_MQTT_task.run();
}