#include "common.h"
#include <RTCZero.h>
#include <ZeroRegs.h>
#include <samd.h>

RTCZero rtc;

//Standby sleep example for MoteinoM0 with external interrupt wakeup
//expected current in standby sleep: 7.5uA
const int wakePin = 0; // Pin to use as interrupt (can be any digital pin)

#include "SPIFlash.h" //get it here: https://www.github.com/lowpowerlab/spiflash
SPIFlash flash(SS_FLASHMEM, 0xEF30); //EF30 for 4mbit  Windbond chip (W25X40CL)

const Arduino_LoRaWAN::lmic_pinmap define_lmic_pins = {
  .nss = SS,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = LMIC_UNUSED_PIN,
  .dio = {9,LMIC_UNUSED_PIN,LMIC_UNUSED_PIN},
};

void standbySleep();

void setup()
{
  pinMode(SS, OUTPUT);
  pinMode(SS_FLASHMEM, OUTPUT);
  digitalWrite(SS, HIGH);
  digitalWrite(SS_FLASHMEM, HIGH);

  Serial.begin(115200);
  logSetup();

  for (int i=0; i<30 && !Serial; ++i) {
    blink(200);
  }

  if (Serial) {
    ZeroRegOptions opts = { Serial, true };
    printZeroRegs(opts);
  }

  delay(500); // Give us a chance to reset before going to sleep

  pinMode(LED_BUILTIN, OUTPUT);

  // Using internal pullup, makes wakePin active low
  // If you need active high, then attach to HIGH and declare wakePin as INPUT without PULLUP, then use external resistor from GND to wakePin to keep it from floating
  pinMode(wakePin, INPUT_PULLUP);

  attachInterrupt(wakePin, wakeupISR, LOW); //note: RISING and FALLING do not seem to work, use LOW or HIGH instead

  if (flash.initialize())
  {
    Log.Info("flash.initialize() OK, sleeping it..." CR);
    flash.sleep();
  }
  else {
    Log.Error("flash.initialize() FAIL" CR);
  }
  flash.sleep();

  loraSetup();
  setupRTC(rtc);

  for (byte count=0; count < 3; count++) {
    Log.Info(".");
    blink(500);
  }
  Log.Info("Entering standby sleep mode...");
  delay(100);
}

void loop()
{
  standbySleep();

  //interrupt happened: WAKE UP and blink the LED!
  for (byte count=0; count < 3; count++) {
    blink(250);
  }
}


#define WRITE8(x,y) *((uint8_t*)&(x)) = uint8_t(y)

void standbySleep() {
  //Disable USB (optional)
  //USBDevice.detach();

#if 0
// Attempt 1 - 2.6mA
  os_radio(RADIO_RST);

  //Standby - lowest power sleep mode
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  __DSB(); // Optimization barrier - wait for completion of prior instructions
  __WFI(); // Wait For Interrupt call
#elif 0
// Attempt 2 - 2.85mA
  os_radio(RADIO_RST);
  rtc.standbyMode();
#elif 0
// Attempt 3 - 1.87mA
  lowPowerPins1(1);

  USBDevice.detach();

  os_radio(RADIO_RST);

  rtc.standbyMode();
#elif 1
// Attempt 4 - 104µA !!!!!
  USBDevice.detach();

  os_radio(RADIO_RST);

  lowPowerPins1(1);

  disableClocks();

  // system_sleep();
  rtc.standbyMode();
#endif

  //Enable USB
  //USBDevice.attach();
}

// RFM95 - D1,2,3,4,5 and Reset are all not connected (NC)

#if 0
--------------------------- EIC
--disabled--
--------------------------- EVSYS
CTRL:
CHANNEL00:  --disabled--
CHANNEL01:  --disabled--
CHANNEL02:  --disabled--
CHANNEL03:  --disabled--
CHANNEL04:  --disabled--
CHANNEL05:  --disabled--
CHANNEL06:  --disabled--
CHANNEL07:  --disabled--
CHANNEL08:  --disabled--
CHANNEL09:  --disabled--
CHANNEL10:  --disabled--
CHANNEL11:  --disabled--
USER DMAC:0:  --disabled--
USER DMAC:1:  --disabled--
USER DMAC:2:  --disabled--
USER DMAC:3:  --disabled--
USER TCC0:EV0:  --disabled--
USER TCC0:EV1:  --disabled--
USER TCC0:MC0:  --disabled--
USER TCC0:MC1:  --disabled--
USER TCC0:MC2:  --disabled--
USER TCC0:MC3:  --disabled--
USER TCC1:EV0:  --disabled--
USER TCC1:EV1:  --disabled--
USER TCC1:MC0:  --disabled--
USER TCC1:MC1:  --disabled--
USER TCC2:EV0:  --disabled--
USER TCC2:EV1:  --disabled--
USER TCC2:MC0:  --disabled--
USER TCC2:MC1:  --disabled--
USER TC3:  --disabled--
USER TC4:  --disabled--
USER TC5:  --disabled--
USER TC6:  --disabled--
USER TC7:  --disabled--
USER ADC:START:  --disabled--
USER ADC:SYNC:  --disabled--
USER AC:COMP0:  --disabled--
USER AC:COMP1:  --disabled--
USER DAC:START:  --disabled--
USER PTC:STCONV:  --disabled--
USER AC1:COMP0:  --disabled--
USER AC1:COMP1:  --disabled--
--------------------------- GCLK
GCLK_MAIN:  GEN00 (always)
GCLK_DFLL48M_REF:  GEN01
GCLK_DPLL:  --disabled--
GCLK_DPLL_32K:  --disabled--
GCLK_WDT:  --disabled--
GCLK_RTC:  --disabled--
GCLK_EIC:  --disabled--
GCLK_USB:  GEN00
GCLK_EVSYS_CHANNEL_0:  --disabled--
GCLK_EVSYS_CHANNEL_1:  --disabled--
GCLK_EVSYS_CHANNEL_2:  --disabled--
GCLK_EVSYS_CHANNEL_3:  --disabled--
GCLK_EVSYS_CHANNEL_4:  --disabled--
GCLK_EVSYS_CHANNEL_5:  --disabled--
GCLK_EVSYS_CHANNEL_6:  --disabled--
GCLK_EVSYS_CHANNEL_7:  --disabled--
GCLK_EVSYS_CHANNEL_8:  --disabled--
GCLK_EVSYS_CHANNEL_9:  --disabled--
GCLK_EVSYS_CHANNEL_10:  --disabled--
GCLK_EVSYS_CHANNEL_11:  --disabled--
GCLK_SERCOMx_SLOW:  --disabled--
GCLK_SERCOM0_CORE:  --disabled--
GCLK_SERCOM1_CORE:  --disabled--
GCLK_SERCOM2_CORE:  --disabled--
GCLK_SERCOM3_CORE:  --disabled--
GCLK_SERCOM4_CORE:  --disabled--
GCLK_SERCOM5_CORE:  --disabled--
GCLK_TCC0_TCC1:  --disabled--
GCLK_TCC2_TC3:  --disabled--
GCLK_TC4_TC5:  --disabled--
GCLK_TC6_TC7:  --disabled--
GCLK_ADC:  GEN00
GCLK_AC_DIG:  --disabled--
GCLK_AC_ANA:  --disabled--
GCLK_DAC:  GEN00
GCLK_PTC:  --disabled--
GCLK_I2S_0:  --disabled--
GCLK_I2S_1:  --disabled--
GEN00:  DFLL48M IDC
GEN01:  XOSC32K
GEN02:  OSCULP32K
GEN03:  OSC8M
GEN04:  --disabled--
GEN05:  --disabled--
GEN06:  --disabled--
GEN07:  --disabled--
GEN08:  --disabled--
--------------------------- NVMCTRL
CTRLB:  MANW RWS=1 SLEEPPRM=WAKEONACCESS READMODE=NO_MISS_PENALTY
PARAM:  NVMP=4096 PSZ=64 RWWEEPROM=
STATUS:
LOCK:  1111111111111111
user row:  BOOTPROT=8k EEPROM_SIZE=0 REGION_LOCKS=1111111111111111
serial # 0x60FF2D36 0x504E4B4C 0x302E3120 0xFF15212A
--------------------------- PAC
PAC0:
PAC1:  DSU
PAC2:  23?
--------------------------- PM
SLEEP:  IDLE=CPU
CPUSEL:  CPUDIV=1
APBASEL:  APBADIV=1
APBBSEL:  APBBDIV=1
APBCSEL:  APBCDIV=1
AHBMASK:  CLK_HPBA_AHB CLK_HPBB_AHB CLK_HPBC_AHB CLK_DSU_AHB CLK_NVMCTRL_AHB CLK_DMAC_AHB CLK_USB_AHB
APBAMASK:  CLK_PAC0_APB CLK_PM_APB CLK_SYSCTRL_APB CLK_GCLK_APB CLK_WDT_APB CLK_RTC_APB CLK_EIC_APB
APBBMASK:  CLK_PAC1_APB CLK_DSU_APB CLK_NVMCTRL_APB CLK_PORT_APB CLK_DMAC_APB CLK_USB_APB
APBCMASK:  CLK_SERCOM0_APB CLK_SERCOM1_APB CLK_SERCOM2_APB CLK_SERCOM3_APB CLK_SERCOM4_APB CLK_SERCOM5_APB CLK_TCC0_APB CLK_TCC1_APB CLK_TCC2_APB CLK_TC3_APB CLK_TC4_APB CLK_TC5_APB CLK_ADC_APB CLK_DAC_APB
RCAUSE:  SYST
--------------------------- PORT
PA11 D0/RX:  DIR=IN INEN
PA10 D1/TX:  DIR=IN INEN
PA14 D2:  DIR=IN INEN
PA09 D3:  DIR=IN INEN
PA08 D4:  DIR=IN INEN
PA15 D5:  DIR=IN INEN
PA20 D6:  DIR=IN INEN
PA21 D7:  DIR=IN INEN
PA06 D8:  DIR=IN INEN
PA07 D9:  DIR=IN INEN
PA18 D10:  DIR=IN INEN
PA16 D11:  DIR=IN INEN
PA19 D12:  DIR=IN INEN
PA17 D13:  DIR=IN INEN
PA02 A0:  DIR=IN INEN
PB08 A1:  DIR=IN INEN
PB09 A2:  DIR=IN INEN
PA04 A3:  DIR=IN INEN
PA05 A4:  DIR=IN INEN
PB02 A5:  DIR=IN INEN
PA22 SDA:  --disabled--
PA23 SCL:  --disabled--
PA12 SPI_MISO:  PMUX=SERCOM4:0
PB10 SPI_MOSI:  PMUX=SERCOM4:2
PB11 SPI_SCK:  PMUX=SERCOM4:3
PB03 LED_RX:  DIR=OUT INEN
PA27 LED_TX:  DIR=OUT INEN
PA28 USB_HOST_EN:  --disabled--
PA24 USB_DM:  PMUX=USB:DM
PA25 USB_DP:  PMUX=USB:DP
PB22 EDBG_TX:  --disabled--
PB23 EDBG_RX:  --disabled--
PA22 EDBG_SDA:  --disabled--
PA23 EDBG_SCL:  --disabled--
PA19 EDBG_MISO:  DIR=IN INEN
PA16 EDBG_MOSI:  DIR=IN INEN
PA18 EDBG_SS:  DIR=IN INEN
PA17 EDBG_SCK:  DIR=IN INEN
PA13 EDBG_GPIO0:  --disabled--
PA21 EDBG_GPIO1:  DIR=IN INEN
PA06 EDBG_GPIO2:  DIR=IN INEN
PA07 EDBG_GPIO3:  DIR=IN INEN
PA03 AREF:  --disabled--
--------------------------- RTC MODE0
--disabled--
--------------------------- SERCOM0
--disabled--
--------------------------- SERCOM1
--disabled--
--------------------------- SERCOM2
--disabled--
--------------------------- SERCOM3
--disabled--
--------------------------- SERCOM4 SPI master
CTRLA:  MISO=PAD0 MOSI=PAD2 SCK=PAD3 FORM=SPI CPHA=LEADING CPOL=LOW DORD=MSB
CTRLB:  CHSIZE=8bit AMODE=MASK RXEN
BAUD:  0x2
--------------------------- SERCOM5
--disabled--
--------------------------- SYSCTRL
PCLKSR:  XOSC32KRDY OSC8MRDY DFLLRDY DFLLLCKF DFLLLCKC BOD33RDY B33SRDY
XOSC:  --disabled--
XOSC32K:  XTALEN EN32K STARTUP=2000092us
OSC32K:  --disabled--
OSCULP32K:  CALIB=0x10
OSC8M:  PRESC=1 CALIB=0x80B FRANGE=8-11MHz
DFLL:  MODE QLDIS WAITLOCK
BOD33:  ACTION=RESET PSEL=1 LEVEL=0x7
VREG:
VREF:  CALIB=0x90
DPLL:  --disabled--
--------------------------- WDT
--disabled--

#endif