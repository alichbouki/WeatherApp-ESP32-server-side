#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <LightSensor.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define SPEED 115200
#define DHT_TYPE DHT11

#define DHT_PIN 19
#define INTER 23
#define RAIN_PIN 13

#define R_PIN 27
#define G_PIN 26
#define B_PIN 25

#define RED 0
#define GREEN 1
#define BLUE 2

#define PWM_FRE 5000
#define PWM_RES 8

const char *ssid = "TP-LINK_AP_1DCC";
const char *password = "86714846";

DHT dht(DHT_PIN, DHT_TYPE);
SparkFun_APDS9960 sens = SparkFun_APDS9960();
IPAddress local_IP(192, 168, 0, 210);
IPAddress gateway(192, 168, 0, 23);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);
AsyncWebServer server(8080);
AsyncWebSocket ws("/");

uint16_t light, newLight;
float tempC, humd, tempF;
bool rainVal, tempMode = random(0, 2);

uint64_t tTime;
String data;
String oldData;
String sendedStr;

uint8_t redVal, greenVal, blueVal;

String formatData()
{
  return "{\"light\":\"" + String(light) + "\",\"temp\":{\"F\":\"" + String(tempF) + "\",\"C\":\"" + String(tempC) + "\"},\"humd\":\"" + String(humd) + "\", \"rain\":\"" + String(!rainVal) + "\", \"colors\":{\"red\":"+ String(redVal) +",\"green\":"+String(greenVal)+",\"blue\":"+String(blueVal)+"}}";
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    String message = (char *)data;
    int index = message.indexOf(',');
    redVal = message.substring(0, index).toInt();
    int index2 = message.lastIndexOf(',');
    greenVal = message.substring(index+1, index2).toInt();
    blueVal  = message.substring(index2+1).toInt();
    Serial.println("R:" + String(redVal) + ",G:" + String(greenVal) + ",B:" + String(blueVal));
  }
}

void notifyClients(String sendedStr)
{
  if (ws.enabled())
    ws.textAll(sendedStr);
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
    break;
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup()
{
  Serial.begin(SPEED);
  Serial.println("Setup start");
  initLightSensor(INTER, sens);

  pinMode(DHT_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  dht.begin();

  ledcSetup(RED, PWM_FRE, PWM_RES);
  ledcSetup(GREEN, PWM_FRE, PWM_RES);
  ledcSetup(BLUE, PWM_FRE, PWM_RES);

  ledcAttachPin(R_PIN, RED);
  ledcAttachPin(G_PIN, GREEN);
  ledcAttachPin(B_PIN, BLUE);

  tTime = millis() + 5000;

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
  {
    Serial.println("STA Failed to configure");
  }
  WiFi.begin(ssid, password);

  Serial.println(String("Connecting to ") + ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    Serial.print(".");
  }

  Serial.println("\nConnected, IP address: ");
  Serial.println(WiFi.localIP());
  initWebSocket();
  server.begin();
  Serial.println("Setup End");
}

void loop()
{
  newLight = getLight(sens);
  light = newLight != 0 ? newLight : light;
  tempC = dht.readTemperature();
  tempF = dht.readTemperature(true);
  humd = dht.readHumidity();
  rainVal = digitalRead(RAIN_PIN);

  if (millis() >= tTime)
  {
    tTime = millis() + 5000;
    tempMode = !tempMode;
  }

  data = formatData();

  ledcWrite(RED, redVal);
  ledcWrite(GREEN, greenVal);
  ledcWrite(BLUE, blueVal);

  if (data != oldData)
  {
    notifyClients(data);
    oldData = data;
  }

  delay(500);
}