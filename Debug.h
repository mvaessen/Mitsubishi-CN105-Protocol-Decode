#define DEBUG 1
#define VERBOSE 0

#define DEBUGPORT TelnetServer
#define DEBUGBAUD 115200

#ifdef  DEBUG=1
#define DEBUG_PRINT(x) DEBUGPORT.print(x)
#define DEBUG_PRINTLN(x) DEBUGPORT.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif