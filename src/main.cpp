// Web update of ESP8266 flash - (c) Joachim Banzhaf 2018
// TODO CONTROL_PIN and disconnection timer to start temp AP for STA config


#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>


// Network stuff, might already be defined by the build tools
#ifndef SSID
  #define SSID      "MyWlanSsid"
#endif
#ifndef PASS
  #define PASS      "MyWlanPass"
#endif
#ifndef NAME
  #define NAME      "WebUpdate"
#endif
#ifndef VERSION
  #define VERSION   NAME " " __DATE__ " " __TIME__
#endif
#ifndef PORT
  #define PORT      80
#endif


ESP8266WebServer web_server(PORT);
ESP8266HTTPUpdateServer esp_updater;


// Initiate connection to Wifi but dont wait for it to be established
void setupWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.hostname(NAME);
  WiFi.begin(SSID, PASS);
  #ifdef ONLINE_PIN
    if( ONLINE_PIN >= 0 ) {
      pinMode(ONLINE_PIN, OUTPUT);
      digitalWrite(ONLINE_PIN, HIGH);
    }
    else {
      pinMode(-ONLINE_PIN, OUTPUT);
      digitalWrite(-ONLINE_PIN, LOW); // led on if pin low
    }
  #endif
  #ifdef CONTROL_PIN
    if( CONTROL_PIN >= 0 ) {
      pinMode(CONTROL_PIN, INPUT_PULLUP); // low on key press
    }
  #endif
}


// Define web pages for update, reset or for configuring parameters
void setupWebserver() {
  // Call this page to see the ESPs firmware version
  web_server.on("/version", []() {
    web_server.send(200, "text/plain", "ok: " VERSION "\n");
  });

  // Call this page to reset the ESP
  web_server.on("/reset", []() {
    web_server.send(200, "text/plain", "ok: reset\n");
    delay(200);
    ESP.restart();
  });

  // Catch all page, gives a hint on valid URLs
  web_server.onNotFound([]() {
    web_server.send(404, "text/plain", "error: use "
      "/reset, /version or post image to /update\n");
  });

  web_server.begin();

  MDNS.addService("http", "tcp", PORT);
}


// Handle online web updater, initialize it after Wifi connection is established
void handleUpdater() {
  static bool updater_needs_setup = true;

  if( WiFi.status() == WL_CONNECTED ) {
    if( updater_needs_setup ) {
      // Init once after connection is (re)established
      #ifdef ONLINE_PIN
        if( ONLINE_PIN >= 0 ) {
          digitalWrite(ONLINE_PIN, HIGH);
        }
        else {
          digitalWrite(-ONLINE_PIN, LOW);
        }
      #endif
      Serial.print("WLAN '");
      Serial.print(WLAN_SSID);
      Serial.print("' connected with IP ");
      Serial.println(WiFi.localIP());

      MDNS.begin(NAME);

      esp_updater.setup(&web_server);
      setupWebserver();

      Serial.print("Update with curl -F 'image=@firmware.bin' ");
      Serial.print(NAME);
      Serial.print(".local");
      if( PORT != 80 ) {
        Serial.print(":");
        Serial.print(PORT);
      }
      Serial.println("/update");

      updater_needs_setup = false;
    }
    web_server.handleClient();
  }
  else {
    if( ! updater_needs_setup ) {
      // Cleanup once after connection is lost
      #ifdef ONLINE_PIN
        if( ONLINE_PIN >= 0 ) {
          digitalWrite(ONLINE_PIN, LOW);
        }
        else {
          digitalWrite(-ONLINE_PIN, HIGH);
        }
      #endif
      updater_needs_setup = true;
    }
  }
}


// Setup on boot
void setup() {
  Serial.begin(115200);

  // Initiate network connection (but dont wait for it)
  setupWifi();

  Serial.print("\nBooted ");
  Serial.println(VERSION);
}


// Worker loop: handling connection status and web requests
void loop() {
  // Online web update
  handleUpdater();
}
