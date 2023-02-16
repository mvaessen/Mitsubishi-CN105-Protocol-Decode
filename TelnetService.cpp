#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <ESPTelnet.h>

#include "TelnetService.h"
#include "config.h"
#include "Debug.h"

ESPTelnet TelnetServer;

TelnetService::TelnetService(void)
{
  _enabled = USE_TELNET;
}

void TelnetService::setup(void)
{
  if (!_enabled) {
    return;
  }

  DEBUG_PRINTLN("Telnet setup");

  using std::placeholders::_1;

  TelnetServer.onConnect(TelnetService::onConnect);
  // en
  TelnetServer.onConnect(std::bind(TelnetService::onConnect, this, _1));

  // TelnetServer.onConnectionAttempt(std::bind( &TelnetService::onConnectionAttempt, this, _1));
  // TelnetServer.onReconnect(std::bind( &TelnetService::onReconnect, this, _1));
  // TelnetServer.onDisconnect(std::bind( &TelnetService::onDisconnect, this, _1));

  DEBUG_PRINT("Telnet: ");

  if (TelnetServer.begin()) {
    DEBUG_PRINTLN("running");
  } else {
    DEBUG_PRINTLN("error.");
    //errorMsg("Will reboot...");
  }
}

void TelnetService::loop(void)
{
  if (!_enabled) {
    return;
  }

  TelnetServer.loop();
}

void TelnetService::post(String payload)
{
  TelnetServer.println(payload);
}

void TelnetService::onConnect(String ip)
{
  DEBUG_PRINT("Telnet: ");
  DEBUG_PRINT(ip);
  DEBUG_PRINTLN(" connected");

  TelnetService::post("\nWelcome " + TelnetServer.getIP());
  TelnetService::post("(Use ^] + q  to disconnect.)");
}

void TelnetService::onConnectionAttempt(String ip)
{
  DEBUG_PRINT("Telnet: ");
  DEBUG_PRINT(ip);
  DEBUG_PRINTLN(" tried to connected");
}

void TelnetService::onReconnect(String ip)
{
  DEBUG_PRINT("Telnet: ");
  DEBUG_PRINT(ip);
  DEBUG_PRINTLN(" reconnected");
}

void TelnetService::onDisconnect(String ip)
{
  DEBUG_PRINT("Telnet: ");
  DEBUG_PRINT(ip);
  DEBUG_PRINTLN(" disconnected");
}
