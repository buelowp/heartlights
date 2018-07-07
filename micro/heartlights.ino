#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define WIFI_SSID     "LivingRoom"
#define WIFI_PASS     "Motorazr2V8"
#define MQTT_SERVER   "172.24.1.10"
#define FOUR_HOURS    (1000 * 60 * 60 * 4)

WiFiClient espClient;
PubSubClient client(MQTT_SERVER, 1883, espClient);
bool g_indicator;
unsigned long g_turnOff;

void callback(char* t, byte* p, unsigned int length) 
{
  String topic(t);
  if (topic == "heartlights/norah/on") {
    Serial.println("Turning lights on");
    digitalWrite(D5, HIGH);
    g_turnOff = millis() + FOUR_HOURS;
  }
  if (topic == "heartlights/norah/off") {
    Serial.println("Turning lights off");
    digitalWrite(D5, LOW);
    g_turnOff = 0;
  }
}

void reconnect()
{
  String name = "heartlights";
  
  name += random(100000);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection for ");
    Serial.print(name.c_str());
    Serial.print("...");
    // Attempt to connect
    if (client.connect(name.c_str())) {
      client.subscribe("heartlights/norah/on");
      client.subscribe("heartlights/norah/off");
      Serial.println("connected");
      digitalWrite(BUILTIN_LED, HIGH);
    }
    else {
      digitalWrite(BUILTIN_LED, LOW);
      Serial.print(".");
      delay(1000);
    }
  }
}

void setup() 
{
  Serial.begin(115200);
  pinMode(D5, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);  // set onboard LED as output
  digitalWrite(D5, LOW);
  digitalWrite(BUILTIN_LED, LOW);
  randomSeed(analogRead(A0));
  g_turnOff = 0;
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("connected with IP: ");
  Serial.println(WiFi.localIP());
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
}

void loop() 
{  
  if (!client.connected()) {
    Serial.println("reconnecting");
    reconnect();
  }
  client.loop();

  if (g_turnOff > 0) {
    if (millis() > g_turnOff) {
      digitalWrite(D5, LOW);
      g_turnOff = 0;
    }
  }
}
