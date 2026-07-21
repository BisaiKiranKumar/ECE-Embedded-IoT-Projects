#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

#define POT_PIN A0
#define DHT_PIN D4
#define DHT_TYPE DHT11      // Change to DHT22 if you have DHT22
#define VIB_PIN D5
#define BUZZER_PIN D6
#define SWITCH_PIN D7

const char* ssid = "Industrial_Monitor";
const char* password = "12345678";

ESP8266WebServer server(80);
DHT dht(DHT_PIN, DHT_TYPE);
float voltage = 0;
float temperature = 0;

bool vibration = false;
bool alarm = false;
bool alarmMuted = false;

String cableStatus = "";

void handleRoot() { 
String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>

<meta charset="UTF-8">
<meta http-equiv="refresh" content="2">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Industrial Monitoring Dashboard</title>

<style>

*{
margin:0;
padding:0;
box-sizing:border-box;
font-family:Arial,sans-serif;
}

body{

background:linear-gradient(135deg,#0f172a,#1e293b,#0f172a);

color:white;

min-height:100vh;

padding:30px;

}

h1{

text-align:center;

font-size:38px;

color:#38bdf8;

margin-bottom:30px;

}

.container{

display:flex;

justify-content:center;

gap:25px;

flex-wrap:wrap;

}

.card{

width:360px;

background:#1e293b;

border-radius:15px;

padding:20px;

box-shadow:0 0 20px rgba(0,255,255,.25);

transition:.3s;

}

.card:hover{

transform:scale(1.02);

}

.card h2{

text-align:center;

color:#22d3ee;

margin-bottom:20px;

}

.row{

display:flex;

justify-content:space-between;

padding:12px 0;

border-bottom:1px solid rgba(255,255,255,.15);

font-size:18px;

}

.status{

font-weight:bold;

}

.footer{

margin-top:40px;

text-align:center;

color:#94a3b8;

font-size:15px;

}

</style>

</head>

<body>

<h1>Industrial Monitoring Dashboard</h1>

<div class="container">
<div class="card">

<h2>Cable Monitoring</h2>

<div class="row">
<span>Voltage</span>
<span class="status">%VOLTAGE%</span>
</div>

<div class="row">
<span>Fault Status</span>
<span class="status">%FAULT%</span>
</div>

<div class="row">
<span>Condition</span>
<span class="status">%CONDITION%</span>
</div>

</div>

<div class="card">

<h2>Machine Health</h2>

<div class="row">
<span>Temperature</span>
<span class="status">%TEMP%</span>
</div>

<div class="row">
<span>Vibration</span>
<span class="status">%VIBRATION%</span>
</div>

<div class="row">
<span>Alarm</span>
<span class="status">%ALARM%</span>
</div>

</div>
<div class="card">

<h2>System Status</h2>

<div class="row">
<span>Overall Status</span>
<span class="status">%SYSTEM%</span>
</div>

<div class="row">
<span>Device</span>
<span class="status">ESP8266</span>
</div>

<div class="row">
<span>Dashboard</span>
<span class="status">LIVE</span>
</div>

</div>
</div>
</body>
</html>

)rawliteral";
html.replace("%VOLTAGE%", String(voltage,1) + " V");
html.replace("%FAULT%", cableStatus);

if(alarm)
{
  html.replace("%CONDITION%", "FAULT DETECTED");
}
else
{
  html.replace("%CONDITION%", "NORMAL");
}
html.replace("%TEMP%", String(temperature,1) + " °C");

if(vibration)
  html.replace("%VIBRATION%", "Detected");
else
  html.replace("%VIBRATION%", "Normal");

if(alarm)
  html.replace("%ALARM%", "ACTIVE");
else
  html.replace("%ALARM%", "OFF");
if(alarm)
{
  html.replace("%SYSTEM%", "FAULT DETECTED");
}
else
{
  html.replace("%SYSTEM%", "SYSTEM HEALTHY");
}

server.send(200, "text/html", html);
}
 
void setup() {

  Serial.begin(115200);

  pinMode(VIB_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  dht.begin();

  // Create ESP8266 Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  // Start Web Server
  server.on("/", handleRoot);
  server.begin();

  Serial.println();
  Serial.println("=================================");
  Serial.println("Access Point Started");
  Serial.print("SSID: ");
  Serial.println(ssid);

  Serial.print("Password: ");
  Serial.println(password);

  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  Serial.println("Web Server Started");
  Serial.println("=================================");
}
void loop() {

 server.handleClient();

 // Alarm Acknowledge Button
if (digitalRead(SWITCH_PIN) == LOW)
{
    alarmMuted = true;
}

  // Read Sensors
 int adc = analogRead(POT_PIN);

 voltage = (adc * 3.3 / 1023.0) * 100;

 temperature = dht.readTemperature();

 vibration = (digitalRead(VIB_PIN) == HIGH);

 alarm = false;

 if (voltage > 280)
    cableStatus = "Normal";
 else if (voltage > 200)
    cableStatus = "Level 1 Fault";
 else if (voltage > 120)
    cableStatus = "Level 2 Fault";
 else
    cableStatus = "Critical Fault";

  // ---------- Machine Monitor ----------

  if (temperature > 60)
    alarm = true;

  if (vibration)
    alarm = true;

  if (voltage <= 280)
    alarm = true;
  // Reset acknowledge when faults are cleared
  if (!alarm)
  {
    alarmMuted = false;
  }
  if (alarm && !alarmMuted)
  {
    tone(BUZZER_PIN, 1000);
  }
  else
  {
    noTone(BUZZER_PIN);
  }


  // ---------- Serial Monitor ----------
  Serial.println("------------------------");
  Serial.print("Voltage     : ");
  Serial.println(voltage);

  Serial.print("Cable Status: ");
  Serial.println(cableStatus);

  Serial.print("Temperature : ");
  Serial.println(temperature);

  Serial.print("Vibration   : ");
  Serial.println(vibration ? "YES" : "NO");

  delay(1000);
}