/*
This example shows how to take simple range measurements with the VL53L1X. The
range readings are in units of mm.
*/

#include <Wire.h>
#include <VL53L1X.h>

VL53L1X sensor;

uint8_t SPAD_array[16][16] = {
        {128, 136, 144, 152, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248},
        {129, 137, 145, 153, 161, 169, 177, 185, 193, 201, 209, 217, 225, 233, 241, 249},
        {130, 138, 146, 154, 162, 170, 178, 186, 194, 202, 210, 218, 226, 234, 242, 250},
        {131, 139, 147, 155, 163, 171, 179, 187, 195, 203, 211, 219, 227, 235, 243, 251},
        {132, 140, 148, 156, 164, 172, 180, 188, 196, 204, 212, 220, 228, 236, 244, 252},
        {133, 141, 149, 157, 165, 173, 181, 189, 197, 205, 213, 221, 229, 237, 245, 253},
        {134, 142, 150, 158, 166, 174, 182, 190, 198, 206, 214, 222, 230, 238, 246, 254},
        {135, 143, 151, 159, 167, 175, 183, 191, 199, 207, 215, 223, 231, 239, 247, 255},
        {127, 119, 111, 103, 95, 87, 79, 71, 63, 55, 47, 39, 31, 23, 15, 7},
        {126, 118, 110, 102, 94, 86, 78, 70, 62, 54, 46, 38, 30, 22, 14, 6},
        {125, 117, 109, 101, 93, 85, 77, 69, 61, 53, 45, 37, 29, 21, 13, 5},
        {124, 116, 108, 100, 92, 84, 76, 68, 60, 52, 44, 36, 28, 20, 12, 4},
        {123, 115, 107, 99, 91, 83, 75, 67, 59, 51, 43, 35, 27, 19, 11, 3},
        {122, 114, 106, 98, 90, 82, 74, 66, 58, 50, 42, 34, 26, 18, 10, 2},
        {121, 113, 105, 97, 89, 81, 73, 65, 57, 49, 41, 33, 25, 17, 9, 1},
        {120, 112, 104, 96, 88, 80, 72, 64, 56, 48, 40, 32, 24, 16, 8, 0}
    };

void initSensor() {
  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1);
  }
  sensor.setDistanceMode(VL53L1X::Short);
  sensor.setMeasurementTimingBudget(20000);
  sensor.startContinuous(1);

  sensor.setROISize(16, 4);
  // sensor.setROISize(16, 16);
  sensor.setROICenter(SPAD_array[7][8]); // Default is 199
  // sensor.setROICenter(SPAD_array[1][8]);
}

void setup()
{
  while (!Serial) {}
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000); // use 400 kHz I2C

  initSensor();
}

void loop()
{
  for (int i = 1; i < 14; i+=1) {
    sensor.setROICenter(SPAD_array[i][8]);
    sensor.read();
    // Serial.print(SPAD_array[7][i]);
    // Serial.print(':');
    Serial.print(sensor.read());
    Serial.print('\t');

  }
  Serial.print('\n');

  // sensor.setROICenter(151);
  // sensor.read();
  // Serial.print(sensor.read());
  // Serial.print("\t");
  // sensor.setROICenter(247);
  // sensor.read();
  // Serial.print(sensor.read());

  // Serial.print("\t");
  // sensor.setROICenter(193);
  // sensor.read();
  // Serial.print(sensor.read());
  // Serial.print("\t");
  // sensor.setROICenter(58);
  // sensor.read();
  // Serial.print(sensor.read());

  if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); initSensor();}

  Serial.println();
  // delay(1000);
}
