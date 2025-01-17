#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <DHT.h>

int motorPin = 12;
int heatPin = 13;

int auto_light = 0;
int light_val = 0;
int light_status = 0;
int vent_status = 0;
int heat_status = 0;

#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const char *ssid = "sch1517";       // имя вашей wifi точки доступа
const char *password = "Gfhjvyfz "; // пароль wifi
const IPAddress host(172, 16, 101, 176);
const int httpPort = 5000;
DynamicJsonDocument doc(1024);

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  dht.begin();

  pinMode(motorPin, OUTPUT);
  pinMode(heatPin, OUTPUT);
  pinMode(A0, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) // подключение к точке
  {
    delay(500);
  }
}


void loop()
{
  String c;
  WiFiClient client;
  if (!client.connect(host, httpPort))
  {
    Serial.println("connection failed");
    return;
  }
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();
  Serial.println(hum);
  Serial.println(temp);
  client.print("GET /second_floor?light_val=" + String(light_val) + "&hum=" + String(hum, 2) + "&temp=" + String(temp, 2) + "\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (millis() - timeout > 1000)
    {
      Serial.println("Client Timeout");
      client.stop();
      return;
    }
  }

  bool new_data = true;
  while (client.available() && new_data)
  {
    String line = client.readStringUntil('}');
    c += line;
    c += '}';
    new_data = false;
  }
  String a = c.substring(c.indexOf('{'));
  deserializeJson(doc, a);

  Serial.println(a);
  int red = doc["red2"];
  int green = doc["green2"];
  int blue = doc["blue2"];
  auto_light = doc["auto_light2"];
  light_status = doc["light_status2"];
  vent_status = doc["vent"];
  heat_status = doc["heat"];

  vent(vent_status);

  heat(heat_status);

  lights(red, green, blue, light_status, auto_light);
}

void heat(int h_s) {
  if (h_s == 1)
  {
    digitalWrite(13, HIGH);
  }
  else
  {
    digitalWrite(13, LOW);
  }
}

void vent(int v_s)
{
  if (v_s == 1)
  {
    digitalWrite(12, HIGH);
  }
  else
  {
    digitalWrite(12, LOW);
  }
}


void lights(int r, int g, int b, int l_s, int a_l)
{
  light_val = analogRead(A0);
  if (a_l == 1)
  {
    if (light_val < 750) {
      l_s = 0;
    }
    else {
      l_s = 1;
    }
  }

  Wire.beginTransmission(8);
  Wire.write(r);
  Wire.write(g);
  Wire.write(b);
  Wire.write(l_s);
  Wire.endTransmission();
  return;
}