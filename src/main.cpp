#include <ModbusMaster.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <StepRunner.h>

ModbusMaster node;
WiFiClient ESP32_S3_Client;
PubSubClient client_MQTT(ESP32_S3_Client);
Preferences prefs;

char MQTT_server[40]   = "192.168.8.119";
char MQTT_port[6]      = "1883";
char MQTT_username[32] = "adill";
char MQTT_password[32] = "00111100";

float temperature = 0.00;
float humidity    = 0.00;

bool shouldSaveConfig = false;

void connect_MQTT()
{
  if (client_MQTT.connected()) {return;}
  
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println();
    Serial.println("WiFi Disconnected");
    return;
  }
  Serial.println();
  Serial.println("WiFi Connected");
  
  Serial.println();
  Serial.print("Connecting to MQTT...");
  
  const char* client_ID = "ESP32-S3-Temperature_and_Humidity-RS-WS-N01-6";
  if (client_MQTT.connect(client_ID, MQTT_username, MQTT_password))
  {
    Serial.println("OK");
    // client_MQTT.subscribe("rsmd/#");
  }
  else
  {
    Serial.print("Failed connect to MQTT. rc=");
    Serial.println(client_MQTT.state());
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

  client_MQTT.publish("rsmd/environment/cold_storage/realtime", payload, payload_length);
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

void loadConfig()
{
  prefs.begin("mqtt_conf", true);

  prefs.getString("server", MQTT_server).toCharArray(MQTT_server, sizeof(MQTT_server));
  prefs.getString("port", MQTT_port).toCharArray(MQTT_port, sizeof(MQTT_port));
  prefs.getString("user", MQTT_username).toCharArray(MQTT_username, sizeof(MQTT_username));
  prefs.getString("pass", MQTT_password).toCharArray(MQTT_password, sizeof(MQTT_password));

  prefs.end();
}

void saveConfig()
{
  prefs.begin("mqtt_conf", false);
  
  prefs.putString("server", MQTT_server);
  prefs.putString("port", MQTT_port);
  prefs.putString("user", MQTT_username);
  prefs.putString("pass", MQTT_password);
  
  prefs.end();
}

void saveConfigCallback()
{
  Serial.println();
  Serial.println("Configuration changed");
  shouldSaveConfig = true;
}

StepRunner connect_MQTT_task(connect_MQTT, 5000);
StepRunner publish_MQTT_task(publish_MQTT, 60000);
StepRunner Temperature_Humidity_sensor_task(Temperature_Humidity_sensor, 5000);

void setup()
{
  Serial.begin(115200);
  
  Serial2.begin(4800, SERIAL_8N1, 16, 17);
  node.begin(1, Serial2);   // Slave ID = 1

  WiFiManager wm;
  
  loadConfig();
  Serial.println();
  Serial.println("Configuration loaded");
  
  WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", MQTT_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", MQTT_port, 6);
  WiFiManagerParameter custom_mqtt_user("user", "MQTT Username", MQTT_username, 32);
  WiFiManagerParameter custom_mqtt_pass("pass", "MQTT Password", MQTT_password, 32);
  
  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_mqtt_user);
  wm.addParameter(&custom_mqtt_pass);
  
  wm.setSaveConfigCallback(saveConfigCallback);
  
  wm.startConfigPortal("ESP32-S3-WiFi_Manager");
  if (!wm.autoConnect("ESP32-S3-WiFi_Manager"))
  {
    Serial.println("Failed connect to WiFi");
    ESP.restart();
  }
  
  Serial.println();
  Serial.println("WiFi Connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
  
  strcpy(MQTT_server, custom_mqtt_server.getValue());
  strcpy(MQTT_port, custom_mqtt_port.getValue());
  strcpy(MQTT_username, custom_mqtt_user.getValue());
  strcpy(MQTT_password, custom_mqtt_pass.getValue());
  
  if (shouldSaveConfig)
  {
    saveConfig();
    Serial.println();
    Serial.println("Configuration saved");
  }
  
  client_MQTT.setServer(MQTT_server, atoi(MQTT_port));
  client_MQTT.setCallback(callback_MQTT);
  client_MQTT.setKeepAlive(120);
}

void loop()
{
  connect_MQTT_task.run();
  client_MQTT.loop();
  Temperature_Humidity_sensor_task.run();
  publish_MQTT_task.run();
}