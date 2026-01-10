#ifndef SERIAL_DEBUG_H
#define SERIAL_DEBUG_H


#ifndef DEBUG
#define DEBUG 1
#endif

#if (DEBUG == 1) // debug
#define DEBUG_PRINT(...) Serial.println(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif


#ifndef UART_SPD
#define UART_SPD 115200
#endif

#endif // SERIAL_DEBUG_H