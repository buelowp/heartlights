/*
 * Project stringlights
 * Description:
 * Author:
 * Date:
 */
#include "MQTT.h"

#define APP_ID      8
#define FOUR_HOURS  (1000 * 60 * 60 * 4)

int g_turnOff;
int g_connected;
int g_appid;
String g_lastTopic;
String g_mqttName = "maddie" + System.deviceID();
byte mqttServer[] = {172, 24, 1, 18};

void callback(char* topic, byte* payload, unsigned int length);
MQTT client(mqttServer, 1883, callback);

int turnLightsOn(String state)
{
    digitalWrite(D1, HIGH);
    g_turnOff = millis() + FOUR_HOURS;
    return 1;
}

int turnLightsOff(String state)
{
    digitalWrite(D1, LOW);
    g_turnOff = 0;
    return 1;
}

void callback(char* t, byte* p, unsigned int length)
{
    String topic(t);
    if (topic == "heartlights/maddie/on") {
        Serial.println("Turning lights on");
        digitalWrite(D1, HIGH);
        g_turnOff = millis() + FOUR_HOURS;
    }
    if (topic == "heartlights/maddie/off") {
        Serial.println("Turning lights off");
        digitalWrite(D1, LOW);
        g_turnOff = 0;
    }
}

void setup()
{
    g_turnOff = 0;
    g_connected = 0;
    g_appid = APP_ID;

    pinMode(D1, OUTPUT);
    pinMode(D7, OUTPUT);
    digitalWrite(D1, LOW);
    digitalWrite(D7, LOW);

    client.connect(g_mqttName.c_str());
    if (client.isConnected()) {
        client.subscribe("heartlights/maddie/on");
        client.subscribe("heartlights/maddie/off");
        g_connected = 1;
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
        digitalWrite(D7, LOW);
        client.loop();
    }
    else {
        digitalWrite(D7, HIGH);
        g_connected = 0;
        client.connect(g_mqttName.c_str());
        if (client.isConnected()) {
            client.subscribe("heartlights/maddie/on");
            client.subscribe("heartlights/maddie/off");
            g_connected = 1;
        }
    }

    if (g_turnOff > 0) {
        if (millis() > g_turnOff) {
            digitalWrite(D1, LOW);
            g_turnOff = 0;
        }
    }
    delay(100);
}
