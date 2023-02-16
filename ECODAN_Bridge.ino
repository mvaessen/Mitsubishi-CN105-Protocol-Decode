/*
    Copyright (C) <2020>  <Mike Roberts>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ArduinoOTA.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include <ESPTelnet.h>

#include "MQTTService.h"
//#include "ReporterService.h"
#include "Ecodan.h"
#include "config.h"
#include "TimerCallBack.h"
#include "Debug.h"
#include "OTA.h"

ECODAN HeatPump;
//ReporterService Reporter;
MQTTService MQTT;

ESPTelnet TelnetServer;

SoftwareSerial SwSerial;
WiFiClient NetworkClient;
String HostName;

void HeatPumpQueryStateEngine(void);
void HeatPumpKeepAlive(void);
void Zone1Report(void);
void Zone1Report(void);
void HotWaterReport(void);
void SystemReport(void);
void TestReport(void);

void wifiManager(void);

TimerCallBack HeatPumpQuery1(500, HeatPumpQueryStateEngine);
TimerCallBack HeatPumpQuery2(60 *  1000, HeatPumpKeepAlive);

void setup()
{

  //Reporter.setup();
  
  HEATPUMP_STREAM.begin(SERIAL_BAUD, SERIAL_CONFIG, D3, D4); // Rx, Tx
  pinMode(D3, INPUT_PULLUP);

  HeatPump.SetStream(&HEATPUMP_STREAM);

  MQTT.setup(HeatPump);

  wifiManager();

  // setupTelnet();
  
  if(USE_OTA) {
    OTASetup(HostName.c_str());
  }
}

void loop()
{
  HeatPumpQuery1.Process();
  HeatPumpQuery2.Process();
  HeatPump.Process();

  // TelnetServer.loop();

  MQTT.loop();

  //todo init class for algo
  if(USE_OTA) {
    ArduinoOTA.handle();
  }
}

void HeatPumpKeepAlive(void)
{
  HeatPump.KeepAlive();
  HeatPump.TriggerStatusStateMachine();
}

void HeatPumpQueryStateEngine(void)
{
  HeatPump.StatusStateMachine();

  if (HeatPump.UpdateComplete()) {

    DEBUG_PRINTLN("Update Complete");

    Zone1Report();
    Zone2Report();
    HotWaterReport();
    SystemReport();
    TestReport();
  }
}


void Zone1Report(void)
{
  StaticJsonDocument<256> doc;
  char Buffer[256];

  doc[F("Temperature")] = HeatPump.Status.Zone1Temperature;
  doc[F("Setpoint")] = HeatPump.Status.Zone1TemperatureSetpoint;

  serializeJson(doc, Buffer);

  MQTT.publish(MQTT_STATUS_ZONE1, Buffer);

  //Reporter.report();
}

void Zone2Report(void)
{
  StaticJsonDocument<256> doc;
  char Buffer[256];

  doc[F("Temperature")] = HeatPump.Status.Zone2Temperature;
  doc[F("Setpoint")] = HeatPump.Status.Zone2TemperatureSetpoint;

  serializeJson(doc, Buffer);
  
  MQTT.publish(MQTT_STATUS_ZONE2, Buffer);

  //Reporter.report(, Buffer);
}

void HotWaterReport(void)
{
  StaticJsonDocument<512> doc;
  char Buffer[512];

  doc["Temperature"] = HeatPump.Status.HotWaterTemperature;
  doc["Setpoint"] = HeatPump.Status.HotWaterSetpoint;
  doc["HotWaterBoostActive"] = HotWaterBoostStr[HeatPump.Status.HotWaterBoostActive];
  doc["HotWaterTimerActive"] = HeatPump.Status.HotWaterTimerActive;
  doc["HotWaterControlMode"] = HowWaterControlModeString[HeatPump.Status.HotWaterControlMode];
  doc["LegionellaSetpoint"] = HeatPump.Status.LegionellaSetpoint;
  doc["HotWaterMaximumTempDrop"] = HeatPump.Status.HotWaterMaximumTempDrop;

  serializeJson(doc, Buffer);

  MQTT.publish(MQTT_STATUS_HOTWATER, Buffer);
  //Reporter.report(MQTT_STATUS_HOTWATER, Buffer);
}

void SystemReport(void)
{
  StaticJsonDocument<256> doc;
  char Buffer[256];

  doc["HeaterFlow"] = HeatPump.Status.HeaterOutputFlowTemperature;
  doc["HeaterReturn"] = HeatPump.Status.HeaterReturnFlowTemperature;
  doc["HeaterSetpoint"] = HeatPump.Status.HeaterFlowSetpoint;
  doc["OutsideTemp"] = HeatPump.Status.OutsideTemperature;
  doc["HeaterPower"] = HeatPump.Status.OutputPower;
  doc["SystemPower"] = SystemPowerModeString[HeatPump.Status.SystemPowerMode];
  doc["SystemOperationMode"] = SystemOperationModeString[HeatPump.Status.SystemOperationMode];
  doc["HeatingControlMode"] = HeatingControlModeString[HeatPump.Status.HeatingControlMode];
  doc["FlowRate"] = HeatPump.Status.PrimaryFlowRate;

  serializeJson(doc, Buffer);

  MQTT.publish(MQTT_STATUS_SYSTEM, Buffer);

  //Reporter.report(MQTT_STATUS_SYSTEM, Buffer);
}

void TestReport(void)
{
  StaticJsonDocument<512> doc;
  char Buffer[512];

  doc["Z1FSP"] = HeatPump.Status.Zone1FlowTemperatureSetpoint;
  doc["CHEAT"] = HeatPump.Status.ConsumedHeatingEnergy;
  doc["CDHW"] = HeatPump.Status.ConsumedHotWaterEnergy;
  doc["DHEAT"] = HeatPump.Status.DeliveredHeatingEnergy;
  doc["DDHW"] = HeatPump.Status.DeliveredHotWaterEnergy;
  doc["Compressor"] = HeatPump.Status.CompressorFrequency;
  doc["RunHours"] = HeatPump.Status.RunHours;
  doc["FlowTMax"] = HeatPump.Status.FlowTempMax;
  doc["FlowTMin"] = HeatPump.Status.FlowTempMin;
  doc["Unknown5"] = HeatPump.Status.UnknownMSG5;
  doc["Defrost"] = HeatPump.Status.Defrost;

  serializeJson(doc, Buffer);

  MQTT.publish(MQTT_STATUS_TEST, Buffer);

  //Reporter.report(MQTT_STATUS_TEST, Buffer);
}

String getHostName()
{
  HostName = "EcodanBridge-";
  HostName += String(ESP.getChipId(), HEX);

  return HostName;
}

void wifiManager()
{
  WiFiManager MyWifiManager;

  //wifiManager.resetSettings(); //reset settings - for testing

  MyWifiManager.setTimeout(180);

  if (!MyWifiManager.autoConnect("Ecodan Bridge AP")) {
    ESP.reset();
    delay(5000);
  }

  WiFi.hostname(getHostName());
}





void setupTelnet()
{
  TelnetServer.onConnect(onTelnetConnect);
  TelnetServer.onConnectionAttempt(onTelnetConnectionAttempt);
  TelnetServer.onReconnect(onTelnetReconnect);
  TelnetServer.onDisconnect(onTelnetDisconnect);

  DEBUG_PRINT("Telnet: ");

  if (TelnetServer.begin()) {
    DEBUG_PRINTLN("running");
  } else {
    DEBUG_PRINTLN("error.");
    //errorMsg("Will reboot...");
  }
}

void onTelnetConnect(String ip)
{
  DEBUG_PRINT("Telnet: ");
  DEBUG_PRINT(ip);
  DEBUG_PRINTLN(" connected");

  TelnetServer.println("\nWelcome " + TelnetServer.getIP());
  TelnetServer.println("(Use ^] + q  to disconnect.)");
}

void onTelnetDisconnect(String ip)
{
  DEBUG_PRINT("Telnet: ");
  DEBUG_PRINT(ip);
  DEBUG_PRINTLN(" disconnected");
}

void onTelnetReconnect(String ip)
{
  DEBUG_PRINT("Telnet: ");
  DEBUG_PRINT(ip);
  DEBUG_PRINTLN(" reconnected");
}

void onTelnetConnectionAttempt(String ip)
{
  DEBUG_PRINT("Telnet: ");
  DEBUG_PRINT(ip);
  DEBUG_PRINTLN(" tried to connected");
}