#include <functional>

#include <Arduino.h>

#include <ParameterStore.h>
#include <RamStore.h>
#include "Logging.h"
#include <LoraStack.h>

class RTCZero;

void blink(int ms);
void wakeupISR(void);
// extern "C" void LMIC_DEBUG_PRINTF(const char *fmt, ...);
void loraSetup();
void logSetup();
void setupRTC(RTCZero &rtc);
void lowPowerPins1(uint16_t pinStart);
void disableClocks();

extern const Arduino_LoRaWAN::lmic_pinmap define_lmic_pins;
