#include "LedControl.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

/* Wifi config
  configure SSID (2.4Ghz) and password
*/
// The SSID (name) of the Wi-Fi network you want to connect to
const char* ssid     = "Thisisyourwifi";
const char* password = "thisisyourwifipassword";

/* LED config
  pin 14/D5 is connected to the DataIn
  pin 12/D6 is connected to the CLK
  pin 13/D7 is connected to LOAD
  We have 1 MAX7219.
*/
LedControl lc = LedControl(14, 12, 13, 1);

/* GET request config
*/
const char* url = "http://api.coindesk.com/v1/bpi/currentprice.json";

unsigned long delayStart = 0;
bool delayRunning = false;
// Timer set to 5 minutes
unsigned long timerDelay = 300000;


StaticJsonDocument<1024> doc;

String bitcoinReadings;
float bitcoinReadingsArr[1];

void setup() {
  /*
    The MAX72XX is in power-saving mode on startup,
    we have to do a wakeup call
  */
  lc.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0, 8);
  /* and clear the display */
  lc.clearDisplay(0);


  // setup WIFI
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(5);
  Serial.println('\n');

  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer

  // setup delay
  delayStart = millis();

  // run once so we don't stare at a blank screen for 5 minutes
  Serial.println(bitcoinkurs());
  char str_bpi_eur_rate[8];
  dtostrf(bitcoinkurs(), 7, 2, str_bpi_eur_rate);

  for (int i = 7; i >= 0; i--)
  {
    setLcDigits(str_bpi_eur_rate[7 - i], i);
  }
}


void setLcDigits(char digit, int digitInArray) {
  lc.setChar(0, digitInArray, digit, false);
}


float bitcoinkurs() {
  HTTPClient http;
  http.begin(url); // Works with HTTP
  int httpCode = http.GET();
  String payload = http.getString();

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(F("Response:"));
    Serial.println(payload); // Print response
    deserializeJson(doc, payload);
  }

  // extract float EUR value
  float bpi_eur_rate = doc["bpi"]["EUR"]["rate_float"];
  return bpi_eur_rate;
}

void loop() {
  Serial.println("Next run in:");
  Serial.println((millis() - delayStart));
  Serial.println(timerDelay);
  if (((millis() - delayStart) >= timerDelay
  )) {
  delayStart = millis();
    Serial.println(bitcoinkurs());
    char str_bpi_eur_rate[8];
    dtostrf(bitcoinkurs(), 7, 2, str_bpi_eur_rate);

    for (int i = 7; i >= 0; i--)
    {
      setLcDigits(str_bpi_eur_rate[7 - i], i);
    }
  }
}
