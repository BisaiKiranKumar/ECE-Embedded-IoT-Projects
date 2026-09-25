#include <ArduinoJson.h>
#include <WiFi.h>
#include "rpc.h"
#include "network.h"
#include "state.h"
#include "config.h"
#include "attributes.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("[WiFi] Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("[WiFi] Connected: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("[MQTT] Connecting");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str(), TB_TOKEN, NULL)) {
      Serial.println(" OK");
      requestSharedAttributes();
      return;
    }

    Serial.print(" failed, rc=");
    Serial.print(mqtt.state());
    Serial.println(". retrying in 5s");
    delay(5000);
  }
}

// ---------------------------------------------------------------------
// Single MQTT callback, dispatched by topic: attribute push/response vs.
// RPC request. This is where Phase 6's two device-side features live.
// ---------------------------------------------------------------------

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  char buf[400];
  unsigned int n = length < sizeof(buf) - 1 ? length : sizeof(buf) - 1;
  memcpy(buf, payload, n);
  buf[n] = '\0';
  Serial.print("[MQTT <<] ");
  Serial.print(topicStr);
  Serial.print(" ");
  Serial.println(buf);

  if (topicStr.startsWith("v1/devices/me/rpc/request/")) {
    String requestId = topicStr.substring(topicStr.lastIndexOf('/') + 1);
    handleRpc(requestId, buf);
    return;
  }

  // Both attribute topics carry attribute key/value pairs; the /response/
  // topic nests them one level under "shared", the push topic does not.
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, buf);
  if (err) return;

  JsonVariant shared = doc["shared"];
  JsonObject attrs = shared.is<JsonObject>() ? shared.as<JsonObject>() : doc.as<JsonObject>();
  applySharedAttributes(attrs);
}