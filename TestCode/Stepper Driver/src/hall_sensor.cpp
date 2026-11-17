#include "hall_sensor.h"
#include "pins.h"
#include "config.h"

HallSensor hallPan  = {"PAN ", TMAG_PAN_WIRE,  Adafruit_TMAG5273(), false, 0, 0};
HallSensor hallTilt = {"TILT", TMAG_TILT_WIRE, Adafruit_TMAG5273(), false, 0, 0};

void resetHallStats(HallSensor &s) {
  s.minMag = 1e9f;
  s.maxMag = 0.0f;
}

void logHall(HallSensor &s, const char *prefix) {
  float x = s.dev.readMagneticX() / 1000.0f;  // library returns uT -> mT
  float y = s.dev.readMagneticY() / 1000.0f;
  float z = s.dev.readMagneticZ() / 1000.0f;
  float mag = sqrtf(x * x + y * y + z * z);
  if (mag < s.minMag) s.minMag = mag;
  if (mag > s.maxMag) s.maxMag = mag;

  Serial.print(prefix);
  Serial.print(s.name);
  Serial.print(" X=");    Serial.print(x, 2);
  Serial.print("  Y=");   Serial.print(y, 2);
  Serial.print("  Z=");   Serial.print(z, 2);
  Serial.print("  |B|="); Serial.print(mag, 2);
  Serial.println(" mT");
}

void printHallStats(HallSensor &s, const char *label) {
  if (!s.ok) return;
  Serial.print(label);
  Serial.print(" TMAG ");
  Serial.print(s.name);
  Serial.print(" |B| min=");
  Serial.print(s.minMag, 2);
  Serial.print(" max=");
  Serial.print(s.maxMag, 2);
  Serial.print(" mT  (swing ");
  Serial.print(s.maxMag - s.minMag, 2);
  Serial.println(" mT)");
}

bool setupHall(HallSensor &s) {
  s.wire.begin();
  s.wire.setClock(TMAG_I2C_HZ);

  Serial.print("TMAG ");
  Serial.print(s.name);
  // begin() checks manufacturer ID (0x5449) and sets continuous XYZ mode
  if (!s.dev.begin(TMAG_ADDR, &s.wire)) {
    Serial.print(": FAILED, no response at 0x");
    Serial.print(TMAG_ADDR, HEX);
    Serial.println(" (check 3V3, SDA/SCL, pull-ups, address variant)");
    return false;
  }

  uint8_t devId = s.dev.getDeviceID();
  bool isX2 = (devId & 0x03) == 0x02;
  Serial.print(": OK  mfr=0x");
  Serial.print(s.dev.getManufacturerID(), HEX);
  Serial.print("  dev=0x");
  Serial.print(devId, HEX);
  Serial.print(isX2 ? " (x2, +/-133/266 mT)" : " (x1, +/-40/80 mT)");
  Serial.print("  T=");
  Serial.print(s.dev.getTemperature(), 1);
  Serial.println(" C");

  delay(10);                               // let first conversion finish
  resetHallStats(s);
  logHall(s, "  baseline ");
  return true;
}
