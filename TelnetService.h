
#ifndef TelnetService_h
#define TelnetService_h

  #include <Arduino.h>
  
  #include "IPAddress.h"

  #include "config.h"
  #include "Ecodan.h"

  class TelnetService
  {
    public:
      TelnetService();

      void setup();
      void loop();
      void post(String payload);

    private:
      bool        _enabled;

      void onConnect(String ip);
      void onConnectionAttempt(String ip);
      void onReconnect(String ip);
      void onDisconnect(String ip);
      
  };

#endif