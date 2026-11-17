#include "motors.h"
#include "pins.h"
#include "config.h"

TMC2209Stepper pan(&TMC_SERIAL, R_SENSE, ADDR_PAN);
TMC2209Stepper tilt(&TMC_SERIAL, R_SENSE, ADDR_TILT);

void motorsInitPins() {
  pinMode(PIN_EN, OUTPUT);
  digitalWriteFast(PIN_EN, HIGH);          // keep drivers off during setup
  pinMode(PIN_STEP_PAN, OUTPUT);
  pinMode(PIN_DIR_PAN, OUTPUT);
  pinMode(PIN_STEP_TILT, OUTPUT);
  pinMode(PIN_DIR_TILT, OUTPUT);
}

void motorsEnable(bool on) {
  digitalWriteFast(PIN_EN, on ? LOW : HIGH);   // ENN is active LOW
}

bool setupDriver(TMC2209Stepper &drv, const char *name) {
  drv.begin();
  drv.pdn_disable(true);        // PDN_UART pin is UART only
  drv.mstep_reg_select(true);   // microsteps from register (MS pins = address)
  drv.I_scale_analog(false);    // REQUIRED: VREF is tied to GND on this board
  drv.senddelay(2);             // needed with two drivers on one bus
  drv.toff(4);                  // enable the output stage
  drv.en_spreadCycle(false);    // StealthChop
  drv.pwm_autoscale(true);
  drv.rms_current(RUN_CURRENT_MA, HOLD_MULTIPLIER);
  drv.microsteps(MICROSTEPS);

  uint8_t result = drv.test_connection();  // 0 = OK
  Serial.print(name);
  Serial.print(": UART ");
  Serial.print(result == 0 ? "OK" : "FAILED");
  Serial.print("  version=0x");
  Serial.print(drv.version(), HEX);        // expect 0x21
  Serial.print("  IRUN=");
  Serial.println(drv.irun());
  return result == 0;
}
