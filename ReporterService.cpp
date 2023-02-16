// #include <Arduino.h>

// #include "ReporterService.h"
// #include "Debug.h"
// #include "config.h"


// // ReporterService::ReporterService(void)
// // {
// //   _telnet = false;
// //   _serial = USE_SERIAL;
// // }

// void ReporterService::setup(void)
// {

//   //DEBUGPORT.begin(DEBUGBAUD);

//   if(USE_SERIAL) {
//     Serial.begin(DEBUGBAUD); 
//   }

//   //setupTelnet();
// }

// void ReporterService::loop(void)
// {
//   // TelnetServer.loop();
// }

// void ReporterService::report(const char* topic, const char* payload)
// {
//   // if (USE_MQTT) {
//   //   MQTTReconnect();
//   //   MQTTClient.publish(MQTT_STATUS_ZONE2, Buffer);
//   // }
  
//   if(DEBUG) {
//     Serial.println(Buffer);
//   }

// }

// void setupTelnet()
// {
//   TelnetServer.onConnect(onTelnetConnect);
//   TelnetServer.onConnectionAttempt(onTelnetConnectionAttempt);
//   TelnetServer.onReconnect(onTelnetReconnect);
//   TelnetServer.onDisconnect(onTelnetDisconnect);

//   DEBUG_PRINT("Telnet: ");

//   if (TelnetServer.begin()) {
//     DEBUG_PRINTLN("running");
//   } else {
//     DEBUG_PRINTLN("error.");
//     //errorMsg("Will reboot...");
//   }
// }

// void onTelnetConnect(String ip)
// {
//   DEBUG_PRINT("Telnet: ");
//   DEBUG_PRINT(ip);
//   DEBUG_PRINTLN(" connected");

//   TelnetServer.println("\nWelcome " + TelnetServer.getIP());
//   TelnetServer.println("(Use ^] + q  to disconnect.)");
// }

// void onTelnetDisconnect(String ip)
// {
//   DEBUG_PRINT("Telnet: ");
//   DEBUG_PRINT(ip);
//   DEBUG_PRINTLN(" disconnected");
// }

// void onTelnetReconnect(String ip)
// {
//   DEBUG_PRINT("Telnet: ");
//   DEBUG_PRINT(ip);
//   DEBUG_PRINTLN(" reconnected");
// }

// void onTelnetConnectionAttempt(String ip)
// {
//   DEBUG_PRINT("Telnet: ");
//   DEBUG_PRINT(ip);
//   DEBUG_PRINTLN(" tried to connected");
// }