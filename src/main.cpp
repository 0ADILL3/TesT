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

char AP_name[32] = "AP Hygrometer Cold_Storage";
char AP_password[32] = "+ResM#2306.!";

char MQTT_server[40]   = "192.168.8.119";
char MQTT_port[6]      = "1883";
char MQTT_username[32] = "";
char MQTT_password[32] = "";
char MQTT_client_ID[64] = "Hygrometer Cold_Storage";
char MQTT_pub_topic[80] = "rsmd/environment/cold_storage/realtime";

float temperature = 0.00;
float humidity    = 0.00;

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
  
  if (client_MQTT.connect(MQTT_client_ID, MQTT_username, MQTT_password))
  {
    Serial.println("OK");
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
  if (!client_MQTT.connected()) {return;}

  JsonDocument doc;

  char MQTT_pub_payload[128];
  doc["TEMP"] = temperature;
  doc["HUM"] = humidity;
  
  size_t MQTT_pub_payload_length = serializeJson(doc, MQTT_pub_payload);

  client_MQTT.publish(MQTT_pub_topic, MQTT_pub_payload, MQTT_pub_payload_length);
}

void Temperature_Humidity_sensor()
{
  uint8_t result = node.readHoldingRegisters(0x0000, 2);
  
  if (result == node.ku8MBSuccess)
  {
    humidity = node.getResponseBuffer(0) / 10.0;
    temperature = int16_t(node.getResponseBuffer(1)) / 10.0;
    
    Serial.println();
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

void load_config()
{
  prefs.begin("mqtt_conf", true);

  prefs.getString("server", MQTT_server).toCharArray(MQTT_server, sizeof(MQTT_server));
  prefs.getString("port", MQTT_port).toCharArray(MQTT_port, sizeof(MQTT_port));
  prefs.getString("username", MQTT_username).toCharArray(MQTT_username, sizeof(MQTT_username));
  prefs.getString("password", MQTT_password).toCharArray(MQTT_password, sizeof(MQTT_password));
  prefs.getString("client_id", MQTT_client_ID).toCharArray(MQTT_client_ID, sizeof(MQTT_client_ID));
  prefs.getString("pub_topic", MQTT_pub_topic).toCharArray(MQTT_pub_topic, sizeof(MQTT_pub_topic));

  prefs.end();

  Serial.println();
  Serial.println("CONFIGURATION LOADED");
  Serial.println("MQTT Server       : " + String(MQTT_server));
  Serial.println("MQTT Port         : " + String(MQTT_port));
  Serial.println("MQTT Username     : " + String(MQTT_username));
  Serial.println("MQTT Client ID    : " + String(MQTT_client_ID));
  Serial.println("MQTT Topic        : " + String(MQTT_pub_topic));
}

void save_config()
{
  prefs.begin("mqtt_conf", false);
  
  prefs.putString("server", MQTT_server);
  prefs.putString("port", MQTT_port);
  prefs.putString("username", MQTT_username);
  prefs.putString("password", MQTT_password);
  prefs.putString("client_id", MQTT_client_ID);
  prefs.putString("pub_topic", MQTT_pub_topic);
  
  prefs.end();
  
  Serial.println();
  Serial.println("CONFIGURATION SAVED");
  Serial.println("MQTT Server       : " + String(MQTT_server));
  Serial.println("MQTT Port         : " + String(MQTT_port));
  Serial.println("MQTT Username     : " + String(MQTT_username));
  Serial.println("MQTT Client ID    : " + String(MQTT_client_ID));
  Serial.println("MQTT Topic        : " + String(MQTT_pub_topic));
}

void reset_config()
{
  prefs.begin("mqtt_conf", false);
  prefs.clear();
  prefs.end();

  Serial.println();
  Serial.println("CONFIGURATION RESET");
}

StepRunner connect_MQTT_task(connect_MQTT, 5000);
StepRunner publish_MQTT_task(publish_MQTT, 60000);
StepRunner Temperature_Humidity_sensor_task(Temperature_Humidity_sensor, 5000);

void setup()
{
  pinMode(0, INPUT_PULLUP);
  Serial.begin(115200);
  
  Serial2.begin(4800, SERIAL_8N1, 16, 17);
  node.begin(1, Serial2);   // Slave ID = 1

  WiFiManager wm;
  
  load_config();
  
  WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", MQTT_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", MQTT_port, 6);
  WiFiManagerParameter custom_mqtt_user("username", "MQTT Username", MQTT_username, 32);
  WiFiManagerParameter custom_mqtt_pass("password", "MQTT Password", MQTT_password, 32);
  WiFiManagerParameter custom_mqtt_client_ID("client_id", "MQTT Client ID", MQTT_client_ID, 64);
  WiFiManagerParameter custom_mqtt_pub_topic( "pub_topic", "MQTT Publish Topic", MQTT_pub_topic, 80);
  
  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_mqtt_user);
  wm.addParameter(&custom_mqtt_pass);
  wm.addParameter(&custom_mqtt_client_ID);
  wm.addParameter(&custom_mqtt_pub_topic);
  
  Serial.println("...");
  delay(3000);
  if (digitalRead(0) == 0)
  {
    Serial.println();
    Serial.println("CONFIG MODE");
    
    reset_config();
    wm.startConfigPortal(AP_name, AP_password);
    
    strcpy(MQTT_server, custom_mqtt_server.getValue());
    strcpy(MQTT_port, custom_mqtt_port.getValue());
    strcpy(MQTT_username, custom_mqtt_user.getValue());
    strcpy(MQTT_password, custom_mqtt_pass.getValue());
    strcpy(MQTT_client_ID, custom_mqtt_client_ID.getValue());
    strcpy(MQTT_pub_topic, custom_mqtt_pub_topic.getValue());
    
    save_config();
    ESP.restart();
  }
  else if (!wm.autoConnect(AP_name, AP_password))
  {
    Serial.println("Failed connect to WiFi");
    ESP.restart();
  }
  Serial.println();
  Serial.println("AUTOCONNECT MODE");
  
  Serial.println();
  Serial.println("WiFi Connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
  
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