#define BLYNK_TEMPLATE_ID "TMPL3zLAwA-EB"
#define BLYNK_TEMPLATE_NAME "Home Automation 2"
#define BLYNK_AUTH_TOKEN "D3BogyqK9itXmMPPIbXMMyKkzSr5V5YG"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

BlynkTimer timer;

#define switch1_pin 26
#define switch2_pin 25
#define led1_pin 13
#define led2_pin 12

#define vpin_led1 V1
#define vpin_led2 V2

int led1_state = 0;
int led2_state = 0;
int last_switch1_state = HIGH;
int last_switch2_state = HIGH;

BLYNK_CONNECTED() {
  Blynk.syncVirtual(vpin_led1);
  Blynk.syncVirtual(vpin_led2);
}

BLYNK_WRITE(vpin_led1) {
  led1_state = param.asInt();
  digitalWrite(led1_pin, led1_state);
}

BLYNK_WRITE(vpin_led2) {
  led2_state = param.asInt();
  digitalWrite(led2_pin, led2_state);
}

void checkSwitches() {
  int current_switch1 = digitalRead(switch1_pin);
  int current_switch2 = digitalRead(switch2_pin);

  if (current_switch1 == LOW && last_switch1_state == HIGH) {
    led1_state = !led1_state;
    digitalWrite(led1_pin, led1_state);
    Blynk.virtualWrite(vpin_led1, led1_state);
  }
  if (current_switch2 == LOW && last_switch2_state == HIGH) {
    led2_state = !led2_state;
    digitalWrite(led2_pin, led2_state);
    Blynk.virtualWrite(vpin_led2, led2_state);
  }
  last_switch1_state = current_switch1;
  last_switch2_state = current_switch2;
}

void setup() {
  Serial.begin(115200);
  pinMode(switch1_pin, INPUT_PULLUP);
  pinMode(switch2_pin, INPUT_PULLUP);
  pinMode(led1_pin, OUTPUT);
  pinMode(led2_pin, OUTPUT);
  digitalWrite(led1_pin, LOW);
  digitalWrite(led2_pin, LOW);

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(200L, checkSwitches);
}

void loop() {
  Blynk.run();
  timer.run();
}
