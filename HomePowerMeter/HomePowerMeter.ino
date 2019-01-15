#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>

float demultiplier = 6.15;

Adafruit_ADS1115 ads1(0x48);
Adafruit_ADS1115 ads2(0x49);

ESP8266WebServer server(80);

double calcIrms(int8_t ads, int8_t input_pin )
{
  int16_t val_min = 0, val_maxim = 0, current_val;
  for (int8_t i = 0; i <= 50; i++) {
    if (ads == 1) {
      current_val = ads1.readADC_SingleEnded(input_pin);
    } else if (ads == 2) {
      current_val = ads2.readADC_SingleEnded(input_pin);
    }
    if (val_maxim == 0 && val_min == 0) {
      val_maxim = current_val;
      val_min = current_val;
    }
    if (current_val > val_maxim) {
      val_maxim = current_val;
    } else if (current_val < val_min) {
      val_min = current_val;
    }
  }

  
  return (val_maxim - val_min) / demultiplier;
}

void setup(void)
{
  Serial.begin(115200);
  WiFi.hostname("powermeter");
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(120);
  wifiManager.autoConnect("powermeter");
  Serial.println();
  Serial.println();
  Serial.print("Connecting...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
  }

  ArduinoOTA.begin();

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV


  ads1.setGain(GAIN_FOUR);
  ads2.setGain(GAIN_FOUR);

  ads1.begin();
  ads2.begin();
  server.on("/", []() {
    server.send(200, "text/plain", "{\"adc0\": " + (String)calcIrms(1, 0) + ", \"adc1\": " + (String)calcIrms(1, 1) + ", \"adc2\": " + (String)calcIrms(1, 2) + ", \"adc3\": " + (String)calcIrms(1, 3) + ", \"adc4\": " + (String)calcIrms(2, 0) + ", \"adc5\": " + (String)calcIrms(2, 1) + ", \"adc6\": " + (String)calcIrms(2, 2) + ", \"adc7\": " + (String)calcIrms(2, 3) + "}");
  });
  server.begin();
}

void loop(void)
{
  ArduinoOTA.handle();
  server.handleClient();
}