#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <ESPTelnet.h>

#include "config.h"
#include "Debug.h"
#include "Ecodan.h"
#include "MQTTService.h"

extern WiFiClient NetworkClient;
PubSubClient MQTTClient(NetworkClient);
extern ESPTelnet TelnetServer;
extern String getHostName();

MQTTService::MQTTService(void)
{
  _enabled = USE_MQTT;
}

void MQTTService::setup(const ECODAN& ecodan)
{
  if (!_enabled) {
    return;
  }

  MQTTClient.setServer(MQTT_SERVER, MQTT_PORT);

  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;
  MQTTClient.setCallback(std::bind( &MQTTService::processIncoming, this, _1,_2,_3));
}

void MQTTService::loop(void)
{
  if (!_enabled) {
    return;
  }

  if (!MQTTService::reconnect())
  {
    return;
  }
    
  //MQTTService::autodiscoverClimate();
  //MQTTService::autodiscoverSensors();

  MQTTClient.loop();
}

void MQTTService::connect(void)
{
  DEBUG_PRINTLN("MQTT ON CONNECT");

  MQTTClient.publish(MQTT_LWT, "online");

  MQTTClient.subscribe(MQTTService::CommandZone1TempSetpoint.c_str());
  MQTTClient.subscribe(MQTTService::CommandZone1FlowSetpoint.c_str());
  MQTTClient.subscribe(MQTTService::CommandZone1CurveSetpoint.c_str());
  MQTTClient.subscribe(MQTTService::CommandSystemHeatingMode.c_str());

  MQTTClient.subscribe(MQTTService::CommandHotwaterSetpoint.c_str());

  MQTTClient.subscribe(MQTTService::CommandSystemPower.c_str());
  MQTTClient.subscribe(MQTTService::CommandSystemTemp.c_str());
}

uint8_t MQTTService::reconnect(void)
{
  if (MQTTClient.connected()) {
    return 1;
  }

  DEBUG_PRINTLN("Attempting MQTT connection...");

  if (MQTTClient.connect(::getHostName().c_str(), MQTT_USER, MQTT_PASS, MQTT_LWT, 0, true, "offline")) {

    DEBUG_PRINTLN("MQTT Connected");
    MQTTService::connect();
    return 1;

  } else {

    switch (MQTTClient.state()) {
      case -4:
        DEBUG_PRINTLN("MQTT_CONNECTION_TIMEOUT");
        break;
      case -3:
        DEBUG_PRINTLN("MQTT_CONNECTION_LOST");
        break;
      case -2:
        DEBUG_PRINTLN("MQTT_CONNECT_FAILED");
        break;
      case -1:
        DEBUG_PRINTLN("MQTT_DISCONNECTED");
        break;
      case 0:
        DEBUG_PRINTLN("MQTT_CONNECTED");
        break;
      case 1:
        DEBUG_PRINTLN("MQTT_CONNECT_BAD_PROTOCOL");
        break;
      case 2:
        DEBUG_PRINTLN("MQTT_CONNECT_BAD_CLIENT_ID");
        break;
      case 3:
        DEBUG_PRINTLN("MQTT_CONNECT_UNAVAILABLE");
        break;
      case 4:
        DEBUG_PRINTLN("MQTT_CONNECT_BAD_CREDENTIALS");
        break;
      case 5:
        DEBUG_PRINTLN("MQTT_CONNECT_UNAUTHORIZED");
        break;
    }

    return 0;
  }

  return 0;
}

// void MQTTService::autodiscoverClimate(void)
// {
//   DEBUG_PRINTLN("MQTT RUN autodiscover climate");

//   String mac = ::getHostName();
//   const char* mqtt_discov_prefix    = "homeassistant";
//   const char* controller_sw_version = SOFTWARE_VERSION;

//   const char* min_temp              = "16";
//   const char* max_temp              = "31";
//   const char* temp_step             = "0.5"; // Ecodan supports temp_step 0.1
  
//   const String mqtt_discov_topic = String(mqtt_discov_prefix) + "/climate/" + mac + "/config";

//   DynamicJsonDocument rootDiscovery(16384);
//   char buffer[1024];

//   rootDiscovery["name"]                = DEVICE_NAME;
//   rootDiscovery["uniq_id"]             = String(mac) + "_hp";
//   rootDiscovery["~"]                   = MQTT_GATEWAY;
//   JsonObject device                    = rootDiscovery.createNestedObject("device");
//     device["name"]                     = DEVICE_NAME;
//     JsonArray ids = device.createNestedArray("ids");
//       ids.add(mac);
//     device["mf"]                       = "MitsuCon";
//     device["mdl"]                      = "Mitsubishi Heat Pump";
//     device["sw"]                       = controller_sw_version;
//   rootDiscovery["min_temp"]            = min_temp;
//   rootDiscovery["max_temp"]            = max_temp;
//   rootDiscovery["temp_step"]           = temp_step;
//   JsonArray modes                      = rootDiscovery.createNestedArray("modes");
//     modes.add("heat_cool");
//     modes.add("cool");
//     modes.add("dry");
//     modes.add("heat");
//     modes.add("fan_only");
//     modes.add("off");
//   rootDiscovery["curr_temp_t"]         = "~/tele/0x0B";
//   rootDiscovery["curr_temp_tpl"]       = "{{ value_json.gw_Zone1_Room_Temp|default(0) }}";
//   rootDiscovery["mode_cmd_t"]          = "~/cmnd";

//   // TODO: Fix mode_cmd_tpl. (Currently testing mode_cmd_tpl with manually YAML configured entity)

//   // Doesn't work, no discovery msg
//   //rootDiscovery["mode_cmd_tpl"]        = "{\"SetACMode\":\"{% set values = {'heat':'0', 'heat_cool':'1', 'cool':'4'} %}{{values[value] if value in values.keys() else '0' }}\"}"; 
//   //rootDiscovery["mode_cmd_tpl"]        = "{\"SetACMode\":\"{% set values = {'heat':'0', 'heat_cool':'1', 'cool':'4'} %}{{values[value]}}\"}";
//   //rootDiscovery["mode_cmd_tpl"]        = "{\"SetACMode\":\"{% set values = {'heat':'0', 'heat_cool':'1', 'cool':'4'} %}\"}";
//   //rootDiscovery["mode_cmd_tpl"]        = "{% set values = {'heat':'0', 'heat_cool':'1', 'cool':'4'} %}{\"SetACMode\":\"\"}";
//   //rootDiscovery["mode_cmd_tpl"]        = "{% set values = {'heat':'0', 'heat_cool':'1', 'cool':'4'} %}{{'{"SetACMode":""}'}}";
//   //rootDiscovery["mode_cmd_tpl"]        = "{% set values = {'heat':'0', 'heat_cool':'1', 'cool':'4'} %}{{values[value]}}";
//   //rootDiscovery["mode_cmd_tpl"]        = "{% set values = {'heat':'0', 'heat_cool':'1', 'cool':'4'} %}{{values\[value\]}}";
//   //rootDiscovery["mode_cmd_tpl"]        = "{% set values = {'heat':'0', 'heat_cool':'1', 'cool':'4'} %}'{{values[value]}}'";
//   //rootDiscovery["mode_cmd_tpl"]        = "{{'{\"SetACMode\":\"0\"}' if value == 'heat' else '{\"SetACMode\":\"1\"}' if value == 'heat_cool' else '{\"SetACMode\":\"1\"}' if value == 'heat_cool' else '{\"SetACMode\":\"4\"}' if value == 'cool' }}";
//   //rootDiscovery["mode_cmd_tpl"]        = "{\"SetACMode\":\"{{'0' if value == 'heat' else '1' if value == 'heat_cool' else '4' if value == 'cool'}}\"}";

//   // Discovery msg is sent, but incomplete
//   //rootDiscovery["mode_cmd_tpl"]        = "{% set values = {'heat':'0', 'heat_cool':'1', 'cool':'4'} %}";
//   //rootDiscovery["mode_cmd_tpl"]          = "{% set values = {'heat':'0', 'heat_cool':'1', 'cool':'4'} %}{{}}";
//   //rootDiscovery["mode_cmd_tpl"]          = "{% set values = {'heat':'0', 'heat_cool':'1', 'cool':'4'} %}{{value}}";
//   //rootDiscovery["mode_cmd_tpl"]          = "{% set values = {'heat':'0', 'heat_cool':'1', 'cool':'4'} %}{{values}}";
//   //rootDiscovery["mode_cmd_tpl"]        = "{\"SetACMode\":\"{{'0' if value == 'heat'}}\"}";

//   rootDiscovery["mode_stat_t"]         = "~/tele/0x26";
//   rootDiscovery["mode_stat_tpl"]       = "{{ 'off' if value_json.gw_Power_State == 'Off' else value_json.gw_AC_Mode_Zone1 | replace('Heating Room Temp', 'heat') | replace('Heating Flow Temp', 'heat_cool') | replace('Cooling Flow Temp', 'cool') }}";
//   rootDiscovery["temp_cmd_t"]          = "~/cmnd";
//   rootDiscovery["temp_cmd_tpl"]        = "{\"SetRoomTemperature\":\"{{value}}\"}";
//   rootDiscovery["temp_stat_t"]         = "~/tele/0x09";
//   rootDiscovery["temp_stat_tpl"]       = "{{ value_json.gw_Zone1_Room_Temp_Setpoint|default(0) }}";
//   rootDiscovery["json_attr_t"]         = "~/tele/attr";

//   // 19 Jan 2020, Home Assistant marks the unit as always "Unavailable" because it advertises LWT. So let us not advertise it
//   // [FEB20] upstream has the advertisement of avty back in, but no pl_avail / pl_not_avail.
//   //rootDiscovery["avty_t"]              = "~/status";
//   //rootDiscovery["pl_avail"]            = "Online";
//   //rootDiscovery["pl_not_avail"]        = "Offline";

//   rootDiscovery["frc_upd"] = true;
//   //doc["retain"]         = "true";

//   size_t n = serializeJson(rootDiscovery, buffer);

//   MQTTClient.publish(mqtt_discov_topic.c_str(), buffer, n);
// }

// void MQTTService::autodiscoverSensors(void)
// {
//   DEBUG_PRINTLN("MQTT RUN autodiscover sensors");

//   String mac = ::getHostName();
//   const char* mqtt_discov_prefix    = "homeassistant";

//   //TODO: waarom moet doc zo groot zijn? Indien kleiner worden de discovery msg's niet goed gevuld voor de laatste sensoren
//   DynamicJsonDocument doc(16384);
//   char buffer[1024];
  
//   char command[5];
//   char entity[40];
//   char uniq_id[50];

//   byte i = 0;
//   while (items[i].PacketType != 0)
//   {
//     sprintf(command, "0x%02X", items[i].Command);
//     sprintf(entity, "%s_%s", MQTT_GATEWAY, items[i].noSpacesName);
//     //sprintf(uniq_id, "%s_%s", mac, entity);

//     String discoveryTopic = String(mqtt_discov_prefix) + "/sensor/" + String(command) + "/" + String(entity) + "/config";
//     doc["name"]                          = String(entity);
//     doc["uniq_id"]                       = String(mac) + "_" + String(entity);
//     doc["~"]                             = MQTT_GATEWAY;
//     JsonObject device                    = doc.createNestedObject("device");
//       device["name"]                     = DEVICE_NAME;
//       JsonArray ids = device.createNestedArray("ids");
//         ids.add(mac);
//       device["mf"]                       = "MitsuCon";
//       device["mdl"]                      = "Mitsubishi Heat Pump";
//       device["sw"]                       = SOFTWARE_VERSION;

//     doc["stat_t"]   = "~/tele/" + String(command);
//     doc["unit_of_meas"] = String(items[i].Unit);
//     doc["dev_cla"] = String(items[i].DevClass);
//     doc["val_tpl"] = "{{ value_json." + String(entity) + "|default(0) }}";
//     doc["frc_upd"] = true;
//     //doc["retain"]         = "true";

//     size_t n = serializeJson(doc, buffer);

//     MQTTClient.publish(discoveryTopic.c_str(), buffer, n);
//     i++;
//   }
// }

void MQTTService::publish(const char* topic, const char* payload)
{
  MQTTClient.publish(topic, payload);
}

void MQTTService::processIncoming(char* topic, byte* payload, unsigned int length)
{
  payload[length] = 0;
  String Topic = topic;
  String Payload = (char*)payload;

  DEBUG_PRINT("Recieved ");
  DEBUG_PRINT(Topic.c_str());
  DEBUG_PRINT("Payload ");
  DEBUG_PRINTLN(Payload.c_str());

  if (Topic == MQTTService::CommandZone1TempSetpoint) {
    DEBUG_PRINTLN("MQTT Set Z1 temp set point");
    MQTTService::ecodan.SetZoneTempSetpoint(Payload.toInt(), BOTH);
  }

  if (Topic == MQTTService::CommandZone1FlowSetpoint) {
    DEBUG_PRINTLN("MQTT Set Z1 flow set point");
    MQTTService::ecodan.SetZoneFlowSetpoint(Payload.toInt(), BOTH);
  }

  if (Topic == MQTTService::CommandZone1CurveSetpoint) {
    DEBUG_PRINTLN("MQTT Set Z1 curve set point");
    MQTTService::ecodan.SetZoneCurveSetpoint(Payload.toInt(), BOTH);
  }

  if (Topic == MQTTService::CommandHotwaterSetpoint) {
    DEBUG_PRINTLN("MQTT Set HW Setpoint");
    MQTTService::ecodan.SetHotWaterSetpoint(Payload.toInt());
  }

  if (Topic == MQTTService::CommandSystemHeatingMode) {
    DEBUG_PRINTLN("MQTT Set Heating Mode");
    MQTTService::ecodan.SetHeatingControlMode(&Payload, BOTH);
  }

  if (Topic == MQTTService::CommandSystemPower) {
    DEBUG_PRINTLN("MQTT Set System Power Mode");
    MQTTService::ecodan.SetSystemPowerMode(&Payload);
  }

  if (Topic == MQTTService::CommandSystemTemp) {
    DEBUG_PRINTLN("Temp Trigger");
    MQTTService::ecodan.Scratch(Payload.toInt());
  }
  //MQTTService::ecodan.TriggerStatusStateMachine();
}