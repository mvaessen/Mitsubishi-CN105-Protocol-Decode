
#ifndef ReporterService_h
#define ReporterService_h

  #include <Arduino.h>

  class ReporterService
  {
    public:
      ReporterService();

      void setup();
      void loop();
      void report(const char* topic, const char* payload);

    private:
      bool _serial;
      bool _telnet;
  };
  
#endif