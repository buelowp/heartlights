#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define WIFI_SSID     "Office"
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
  if (topic == "heartlights/maddie/on") {
    Serial.println("Turning lights on");
    digitalWrite(D5, HIGH);
    g_turnOff = millis() + FOUR_HOURS;
  }
  if (topic == "heartlights/maddie/off") {
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
      client.subscribe("heartlights/maddie/on");
      client.subscribe("heartlights/maddie/off");
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

void printStatus(int status)
{
    WiFi.printDiag(Serial);

  switch (status) {
    case WL_NO_SHIELD:
      Serial.println("No Shield Present");
      break;
    case WL_IDLE_STATUS:
      Serial.println("Idle Status");
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("assigned when no SSID are available");
      break;
    case WL_SCAN_COMPLETED: 
      Serial.println("assigned when the scan networks is completed");
      break;
    case WL_CONNECT_FAILED: 
      Serial.println("assigned when the connection fails for all the attempts");
      break;
    case WL_CONNECTION_LOST: 
      Serial.println("assigned when the connection is lost");
      break;
    case WL_DISCONNECTED: 
      Serial.println("assigned when disconnected from a network");
      break;
    default:
      Serial.print("Unknown error number ");
      Serial.println(status);
      break;
  }  
}

void printScanResult(int networksFound)
{
  Serial.printf("%d network(s) found\n", networksFound);
  for (int i = 0; i < networksFound; i++)
  {
    Serial.printf("%d: %s, Ch:%d (%ddBm) %d\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i));
  }
}

void setup() 
{
  int status = 0;
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(5000);
  pinMode(D5, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);  // set onboard LED as output
  digitalWrite(D5, LOW);
  digitalWrite(BUILTIN_LED, LOW);
  randomSeed(analogRead(A0));
  g_turnOff = 0;

  if (WiFi.status() == WL_CONNECTED)
    Serial.println("Connected to wemos");
  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while ((status = WiFi.status()) != WL_CONNECTED) {
    printStatus(status);
    delay(1000);
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
