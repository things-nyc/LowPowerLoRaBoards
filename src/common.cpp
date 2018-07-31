#include "common.h"
#include <RTCZero.h>

void blink(int ms) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(ms);
  digitalWrite(LED_BUILTIN, LOW);
  delay(ms);
}

void wakeupISR(void)
{
  //nothing here, just a placeholder
}

void logPrintFn(const char *s) {
  if (Serial) {
    Serial.print(s);
  }
}
char logBuffer[1000];
LogBufferedPrinter logPrinter(logPrintFn, logBuffer, sizeof(logBuffer));
void logSetup() {
  Log.Init(LOG_LEVEL_VERBOSE, logPrinter);
}

// Lorawan Device ID, App ID, and App Key
const char *devEui = "0084EA9F319565CE";
const char *appEui = "70B3D57EF0003952";
const char *appKey = "76718976B3250A516D89E8E47457AECC";

const int STORE_SIZE = 2000;
RamStore<STORE_SIZE> byteStore;
ParameterStore gParameters(byteStore);
LoraStack_LoRaWAN lorawan(define_lmic_pins, gParameters);
LoraStack node(lorawan, gParameters, TTN_FP_US915);

void loraSetup() {
  bool status = byteStore.begin();
  if (!status) {
      Log.Error(F("Failed to initialize byteStore!" CR));
      while (1);
  }

  status = gParameters.begin();
  if (!status) {
      Log.Error(F("Failed to initialize Parameter Store!" CR));
      while (1);
  }

  Log.Debug(F("Setting lorawan debug mask." CR));
  lorawan.SetDebugMask(Arduino_LoRaWAN::LOG_BASIC | Arduino_LoRaWAN::LOG_ERRORS | Arduino_LoRaWAN::LOG_VERBOSE);

  Log.Debug(F("Provisioning node!" CR));
  status = node.provision(appEui, devEui, appKey);
  if (!status) {
      Log.Error(F("Failed to personalize device!" CR));
      while (1);
  }

  status = node.begin();
  if (!status) {
      Log.Error(F("Failed to start LoRaWAN node!" CR));
      while (1);
  }
  status = node.join();
  if (!status) {
      Log.Error(F("Failed to start LoRaWAN join!" CR));
      while (1);
  }
  Log.Info(F("Finished initializing!" CR));
}

void alarmMatch()
{
  digitalWrite(LED_BUILTIN, HIGH);
}

void setupRTC(RTCZero &rtc) {
  /* Change these values to set the current initial time */
  const byte seconds = 0;
  const byte minutes = 00;
  const byte hours = 17;

  /* Change these values to set the current initial date */
  const byte day = 17;
  const byte month = 11;
  const byte year = 15;

  rtc.begin();

  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

  rtc.setAlarmTime(17, 00, 10);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);

  rtc.attachInterrupt(alarmMatch);
}

void lowPowerPins1(uint16_t pinStart) {
  for (uint16_t pin=pinStart; pin<NUM_DIGITAL_PINS; ++pin) {
    pinMode(pin, OUTPUT);
  }

  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
}

#define WRITE8(x,y) *((uint8_t*)&(x)) = uint8_t(y)

void disableClockGenerator(int genid) {
  WRITE8(GCLK->GENCTRL.reg, genid);
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) {}
  GCLK->GENCTRL.bit.GENEN = 0;
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) {}
}

void disableClock(int gclkid) {
  WRITE8(GCLK->CLKCTRL.reg, gclkid);
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) {}
  GCLK->CLKCTRL.bit.CLKEN = 0;
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) {}
}

void system_sleep(void) {
  SCB->SCR |=  SCB_SCR_SLEEPDEEP_Msk;
	__DSB();
	__WFI();
}

void system_cpu_clock_set_divider(
		uint32_t divider)
{
	PM->CPUSEL.reg = (uint32_t)divider;
}

void disableClocks() {
  // Disable CLK_USB_AHB
  PM->AHBMASK.reg &= ~(PM_AHBMASK_USB
				| PM_AHBMASK_DSU
				| PM_AHBMASK_HPB1
				| PM_AHBMASK_HPB2
				| PM_AHBMASK_DMAC
				/* These clocks should remain enabled on this bus
				PM_AHBMASK_HPB1
				| PM_AHBMASK_HPB2
				| PM_AHBMASK_HPB0
				| PM_AHBMASK_NVMCTRL
				*/
			);
  PM->APBAMASK.reg &= ~(PM_APBAMASK_WDT
				| PM_APBAMASK_PAC0
				| PM_APBAMASK_EIC
				| PM_APBAMASK_GCLK
				/* These clocks should remain enabled on this bus
				| PM_APBAMASK_SYSCTRL
				| PM_APBAMASK_PM
				| PM_APBAMASK_RTC
				*/
				);
  PM->APBBMASK.reg &= ~(PM_APBBMASK_PAC1
				| PM_APBBMASK_PORT
				| PM_APBBMASK_DSU
				| PM_APBBMASK_DMAC
				| PM_APBBMASK_USB
				| PM_APBBMASK_NVMCTRL
				/* These clocks should remain enabled on this bus
				*/
				);
  PM->APBCMASK.reg &= ~(PM_APBCMASK_ADC
				| PM_APBCMASK_PAC2
				| PM_APBCMASK_DAC
				| PM_APBCMASK_AC
				| PM_APBCMASK_TC7
				| PM_APBCMASK_TC6
				| PM_APBCMASK_TC5
				| PM_APBCMASK_TC4
				| PM_APBCMASK_TC3
				| PM_APBCMASK_TCC2
				| PM_APBCMASK_TCC1
				| PM_APBCMASK_TCC0
				| PM_APBCMASK_SERCOM5
				| PM_APBCMASK_SERCOM4
				| PM_APBCMASK_SERCOM3
				| PM_APBCMASK_SERCOM2
				| PM_APBCMASK_SERCOM1
				| PM_APBCMASK_SERCOM0
				| PM_APBCMASK_EVSYS
				| PM_APBCMASK_I2S
				);

  // Disable SPI
  SERCOM4->USART.CTRLA.bit.ENABLE=0;

  SYSCTRL->BOD33.reg = 0;

  // Disable unused clocks
  disableClock(6); // GCLK_USB
  disableClock(30); // GCLK_ADC
  disableClock(33); // GCLK_DAC

  // Disable unused clock generators
  disableClockGenerator(2);
  disableClockGenerator(3);

  // system_cpu_clock_set_divider(7);
}
