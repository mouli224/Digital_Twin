#include "HX711.h"
#include "AS5600.h"
#include "Wire.h"

AS5600 as5600;   //  use default Wire

long enc_offset = 0;

long weight = 0;
float enc_height_resolution = 0.00097656; //8/8192 =0.00195312;
float enc_ang_resolution = 0.04394531; //360/8192=0.04394531
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 13;

long lastTime = millis();
long last_send = millis();
long ticks = 0;
HX711 scale;
void setup() {


  // Setup Serial Monitor
  Serial.begin(115200);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(454.126984);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();
  as5600.begin(14);  //  set direction pin.
  as5600.setDirection(AS5600_COUNTERCLOCK_WISE);  // default, just be explicit.

  delay(1000);
  enc_offset = as5600.getCumulativePosition();
}

void loop() {

  if (millis() - lastTime >= 100)
  {
    lastTime = millis();
    ticks = as5600.getCumulativePosition() - enc_offset;
  }
  if (scale.is_ready()) {
    weight = scale.get_units();
    //    Serial.print("HX711 reading: ");
    //    Serial.println(weight);
  }

  if (millis() - last_send >= 200) {
    float height = ticks * enc_height_resolution;
    if (height < 0) height = 0;
    float ang = ticks * enc_ang_resolution;
    if (ang < 0) ang = 0;
    String output_str = String(ang) + ";" + String(height) + ";" + String(weight);
    Serial.println(output_str);
    last_send = millis();
  }
  delay(1);
}
