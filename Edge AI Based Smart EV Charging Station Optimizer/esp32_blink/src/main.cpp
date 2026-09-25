#include <Arduino.h>
#include <WiFi.h>
#include "State.h"
#include "config.h"
#include "Peripherals.h"
#include "Network.h"
#include "Telemetry.h"
#include "model.h"
#include "edge_ai.h"
#include "optimization.h"
#include "attributes.h"
#include "rpc.h"

void setup()
{

    Serial.begin(115200);
    dht.begin();  // initialise sesnor
    //config esp32 with real time
    configTime(0,0,"pool.ntp.org", "time.nist.gov");
    pinMode(BTN_PLUGIN, INPUT_PULLUP);
    pinMode(BTN_PLUGOUT, INPUT_PULLUP);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);

    connectWiFi();
    // Configure MQTT server
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(mqttCallback);
    mqtt.setBufferSize(512);
    connectMQTT();
   
}

unsigned long now;
unsigned long last_print;

void loop()
{
    // Keep bay state in sync with the physical plug buttons at all times.
    plug_status();

    //push the data every 5 sec
    now = millis();
    if((now - last_print) > 5000)
    {
         last_print = now;
        //raed dataa from the sensor
         sample_sensor();
        // run AI to get predection 
         runEdgeAIInference();
         if(manualOverrideActive == 0)
         {
             runOptimization();
         }
        //publish the data
         publishTelemetry();
    }

    if(manualOverrideActive == 0)
    {
        applyRelayDutyCycle();
    }

    update_led_status();
}

