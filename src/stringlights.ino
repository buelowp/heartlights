/*
 * Project stringlights
 * Description:
 * Author:
 * Date:
 */
#include "MQTT.h"

#define APP_ID      14
#define LED_PIN             D1
#define DISCONNECT_PIN      D7
#define FOUR_HOURS  (1000 * 60 * 60 * 4)

int g_turnOff;
int g_connected;
int g_appid;
String g_lastTopic;
String g_name = "norah";
String g_mqttName = g_name + System.deviceID().substring(0, 8);
String g_on = "heartlights/" + g_name + "/on";
String g_off = "heartlights/" + g_name + "/off";
byte mqttServer[] = {172, 24, 1, 18};

void callback(char* topic, byte* payload, unsigned int length);
MQTT client(mqttServer, 1883, callback);

int turnLightsOn(String state)
{
    digitalWrite(LED_PIN, HIGH);
    g_turnOff = millis() + FOUR_HOURS;
    return 1;
}

int turnLightsOff(String state)
{
    digitalWrite(LED_PIN, LOW);
    g_turnOff = 0;
    return 1;
}

void callback(char* t, byte* p, unsigned int length)
{
    String topic(t);

    if (topic == g_on) {
        Serial.println("Turning lights on");
        digitalWrite(LED_PIN, HIGH);
        g_turnOff = millis() + FOUR_HOURS;
    }
    if (topic == g_off) {
        Serial.println("Turning lights off");
        digitalWrite(LED_PIN, LOW);
        g_turnOff = 0;
    }
}

void setup()
{
    g_turnOff = 0;
    g_connected = 0;
    g_appid = APP_ID;

    pinMode(LED_PIN, OUTPUT);
    pinMode(DISCONNECT_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(DISCONNECT_PIN, HIGH);

    client.connect(g_mqttName.c_str());
    if (client.isConnected()) {
        client.subscribe(g_on.c_str());
        client.subscribe(g_off.c_str());
        g_connected = 1;
        digitalWrite(DISCONNECT_PIN, LOW);
    }

    Particle.function("turnon", turnLightsOn);
    Particle.function("turnoff", turnLightsOff);
    Particle.variable("connected", g_connected);
    Particle.variable("appid", g_appid);
    Particle.variable("countdown", g_turnOff);
    Particle.variable("mqttname", g_mqttName);
}

void loop()
{
    if (client.isConnected()) {
        digitalWrite(DISCONNECT_PIN, LOW);
        client.loop();
    }
    else {
        digitalWrite(DISCONNECT_PIN, HIGH);
        g_connected = 0;
        client.connect(g_mqttName.c_str());
        if (client.isConnected()) {
            client.subscribe(g_on);
            client.subscribe(g_off);
            g_connected = 1;
        }
    }

    if (g_turnOff > 0) {
        if (millis() > g_turnOff) {
            digitalWrite(LED_PIN, LOW);
            g_turnOff = 0;
        }
    }
    delay(100);
}
