#include <Arduino.h>
#include <DHT.h>
#include "State.h"
#include "Peripherals.h"
#include "config.h"


DHT   dht(DHT_PIN , DHT_TYPE);


float mapFloat(long x, long inMin, long inMax, float outMin, float outMax) 
{
  return (x - inMin) * (outMax - outMin) / (float)(inMax - inMin) + outMin;
}

void sample_sensor(void)
{
    int raw_current = analogRead(CURRENT_PIN); // 0 to 4095
    int raw_voltage = analogRead(VOLTAGE_PIN); // 0 to 4095
    
    voltage = mapFloat(raw_voltage, 0, 4095, 0, 250);
    
    if( bayStatus == "CHARGING" )
    {
    current = mapFloat(raw_current, 0 , 4095, 0, 250);
    }
    else
    {
        current = 0;
    }

    power = voltage * current ;

    // Force the red LED if either measured value crosses the 120V / 120A warning threshold.
    overloadActive = (voltage > 120.0f) || (current > 120.0f);
    
    //to read temperature 
    float t = dht.readTemperature(DHT_PIN );
    if(!(isnan(t))) temperature = t;



}

float recentAvgCurrent(void)
{
  float sum = 0;
  for(int i=0;i<5;i++)
  {
   sum = sum + current; 
  }

   return sum/5;
}



bool plugin_flag_once = 1;
bool plugout_flag_once = 1;

bool plug_status(void)
{
  bool stateChanged = false;
   bool pluginReading = digitalRead(BTN_PLUGIN);
   // detect the sw is pressed
   if( pluginReading == LOW && plugin_flag_once )
    { 
      sessionStartMs = millis();  // record the start time of the session      
       // plug in switch is pressed
       plugin_flag_once = 0;
       // change bay_status FREE to charging
       if( bayStatus == "FREE" )
        {
          bayStatus = "CHARGING";
          stateChanged = true;
          Serial.println("Bay1 plugin detected, Bay is Charging");
          digitalWrite( RELAY_PIN, HIGH );
        }
      //update leds
    }
    if(pluginReading == HIGH)
    {
       plugin_flag_once = 1;
    }
    // plug out switch is pressed
    bool plugoutReading = digitalRead(BTN_PLUGOUT);
    // detect the sw is pressed
   if( plugoutReading == LOW && plugout_flag_once )
    {
            
       // plug in switch is pressed
      plugout_flag_once = 0;
       // change bay_status charging to FREE
      if( bayStatus == "CHARGING" )
        {
            bayStatus = "FREE";
            stateChanged = true;
            Serial.println("Bay1 plugout detected, Bay is Free");
        }
      //update leds
    }
    if(plugoutReading == HIGH)
    {
        plugout_flag_once = 1;
    }

    return stateChanged;
} 
void update_led_status(void)
{
    if (overloadActive)
    {
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, LOW);
        return;
    }

    digitalWrite(LED_RED, LOW);

    if( bayStatus == "FREE" )
    {
         digitalWrite(LED_GREEN, HIGH);
         digitalWrite(LED_YELLOW, LOW);
    }
    else
    {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, HIGH);
    }
}