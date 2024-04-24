#include <Wire.h>
#include <VL53L1X.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define Timeout 2000

Adafruit_MPU6050 mpu;

const uint8_t sensorCount = 4;
const uint8_t xshutPins[sensorCount-1] = {2, 0, 16};

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

VL53L1X sensors[sensorCount];

uint16_t sensor_data[sensorCount];
int64_t last_reading_time[sensorCount]; // needs to be signed for later math to work
int64_t time_taken[sensorCount];

void initAllTOF() {
  Wire.begin();
  Wire.setClock(400000); // use 400 kHz I2C

  // Disable/reset all sensors by driving their XSHUT pins low.
  last_reading_time[0] = 10000000000;
  sensor_data[0] = 0;
  for (uint8_t i = 1; i < sensorCount; i++)
  {
    pinMode(xshutPins[i-1], OUTPUT);
    digitalWrite(xshutPins[i-1], LOW);
    last_reading_time[i] = 10000000000;
    sensor_data[i] = 0;
  }
  delay(10);

  // Enable, initialize, and start each sensor, one by one.
  initSensor(&sensors[0], 0);
  for (uint8_t i = 1; i < sensorCount; i++)
  {
    digitalWrite(xshutPins[i-1], HIGH); // enable sensor
    delay(10);

    initSensor(&sensors[i], i);
  }
}

void initSensor(VL53L1X *sensor, int i) {
  sensor->setTimeout(Timeout);
  if (!sensor->init())
  {
    Serial.print("Failed to detect and initialize sensor ");
    Serial.println(i);
    while (1);
  }
  sensor->setDistanceMode(VL53L1X::Short);
  sensor->setMeasurementTimingBudget(20000);

  sensor->setROISize(16, 16);
  sensor->setROICenter(SPAD_array[7][8]); // Default is 199
  
  sensor->setAddress(0x2A + i);

  sensor->startContinuous(20);
}

void setup()
{
  while (!Serial) {}
  Serial.begin(115200);

  initAllTOF();

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);
}

void loop(){
  // Update Sensor Vals
  for (uint8_t i = 0; i < sensorCount; i++)
  {
    if(sensors[i].dataReady()){
      sensors[i].read(false);
      sensor_data[i] = sensors[i].ranging_data.range_mm;
      time_taken[i] = millis() - last_reading_time[i];
      last_reading_time[i] = millis();
    }
    // if(i != 0 && millis() - last_reading_time[i] > Timeout){
    //   Serial.println("Sensor FAILURE!");
    //   pinMode(xshutPins[i-1], OUTPUT);
    //   digitalWrite(xshutPins[i-1], LOW);
    //   delay(10);
    //   pinMode(xshutPins[i-1], INPUT); // stop driving xSHUT
    //   delay(10);
    //   initSensor(&sensors[i], i);
    //   last_reading_time[i] = millis();
    //   // initAllTOF();
    // }
  }
  
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Print Sensor Vals
  for (uint8_t i = 0; i < sensorCount; i++)
  {
    Serial.print(sensor_data[i]/304.8);
    Serial.print("\t");
    Serial.print(time_taken[i]);
    Serial.print("\t");
  }
  Serial.println();

  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degC");

  delay(1000);
}
