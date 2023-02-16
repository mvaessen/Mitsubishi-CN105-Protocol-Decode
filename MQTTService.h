
#ifndef MQTTService_h
#define MQTTService_h

  #include <Arduino.h>
  
  #include "IPAddress.h"

  #include "config.h"
  #include "Ecodan.h"

  class MQTTService
  {
    public:
      MQTTService();

      void setup(const ECODAN& ecodan);
      void loop();

      void publish(const char* topic, const char* payload);

    private:
      bool        _enabled;
      ECODAN      ecodan;

      String CommandZone1TempSetpoint = MQTT_COMMAND_ZONE1_TEMP_SETPOINT;
      String CommandZone1FlowSetpoint = MQTT_COMMAND_ZONE1_FLOW_SETPOINT;
      String CommandZone1CurveSetpoint = MQTT_COMMAND_ZONE1_CURVE_SETPOINT;

      String CommandHotwaterSetpoint = MQTT_COMMAND_HOTWATER_SETPOINT;

      String CommandSystemHeatingMode = MQTT_COMMAND_SYSTEM_HEATINGMODE;
      String CommandSystemPower = MQTT_COMMAND_SYSTEM_POWER;
      String CommandSystemTemp = MQTT_COMMAND_SYSTEM_TEMP;

      void bind();
      void connect();
      uint8_t reconnect();

      void autodiscoverClimate();
      void autodiscoverSensors();

      void processIncoming(char* topic, byte* payload, unsigned int length);
      
  };

#endif