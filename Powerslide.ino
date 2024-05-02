
#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include <Wire.h>
#include <VL53L1X.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>
#include <string>

const char* ssid = "PowerslideAP";
const char* password = "ChangeThisBeforeUse";
const int TIMEOUT = 500;

Adafruit_MPU6050 mpu;
long IMU_Last_Reading_Time = 0;
double IMU_Delta_T = 0;
double IMU_Theta = 0;

double control_theta = 0;

long last_update_time = 0;

// 0 - Manual
// 1 - Gyro
int DriveMode = 0;

AsyncWebServer server(80);

String controls = "------------";
// 0 - L_stick_x
// 1 - L_stick_y
// 2 - L_stick_x
// 3 - L_stick_y
// 4 - L_trigger
// 5 - L_trigger
// 6 - A
// 7 - B
// 8 - X
// 9 - Y
// 10- L_bumper
// 11- R_bumper

Servo MotorF;
Servo MotorR;
Servo MotorL;

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
uint16_t sensor_status[sensorCount];
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
  sensor->setTimeout(TIMEOUT);
  bool init_needed = sensor->init();
  sensor->setAddress(0x2A + i);
  if (init_needed){

    sensor->setDistanceMode(VL53L1X::Short);
    sensor->setMeasurementTimingBudget(20000);
    
    switch(i) {
      case 0:
        sensor->setROISize(6, 8);
        sensor->setROICenter(SPAD_array[7+4][8-5]);
        break;
      case 1:
        sensor->setROISize(6, 8);
        sensor->setROICenter(SPAD_array[7-4][8-5]);
        break;
      case 2:
        sensor->setROISize(6, 8);
        sensor->setROICenter(SPAD_array[7-4][8+5]);
        break;
      case 3:
        sensor->setROISize(6, 8);
        sensor->setROICenter(SPAD_array[7+4][8+5]);
        break;
      default:
      sensor->setROISize(16, 16); // y, x
      sensor->setROICenter(SPAD_array[7][8]); // x, y; Default is 199
    }

    sensor->startContinuous(15);
  }
}

void initIMU() {
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

void setup(){
  Serial.begin(115200);
  Serial.println();

  initAllTOF();

  initIMU();

  MotorF.attach(D7);
  MotorR.attach(D6);
  MotorL.attach(D8);

  for (int i = 0; i < 12; i++) {
    controls[i] = (char)128;
  }
  
  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    controls = request->header("Control");
    last_update_time = millis();
    request->send_P(200, "text/plain", "Output_here");
  });

  server.begin();
}

uint16_t getRobotDist() {
  uint16_t range_min = 65535;
  for (int i = 0; i < 4; i++) {
    if (sensor_status[i] == 0) { // only count soild readings
      if (sensor_data[i] < range_min) {
        range_min = sensor_data[i];
      }
    }
  }
  return range_min;
}

double OpponentLock() {
  String TOFbitmap = "0000";
  for (int i = 0; i < 4; i++) {
    bool RobotDetected = (sensor_data[i] < 1200) && (double)abs(getRobotDist() - sensor_data[i])/(double)sensor_data[i] < 0.3;
    TOFbitmap[i] = '0' + RobotDetected;
  }
  if (TOFbitmap == "0000") { // nothing detected
    return control_theta;
  } else if (TOFbitmap == "0001") {
    return IMU_Theta + 9*PI/180;
  } else if (TOFbitmap == "0010") { // opponent likely nearly centered
    return IMU_Theta + 2*PI/180;
  } else if (TOFbitmap == "0011") {
    return IMU_Theta + 6*PI/180;
  } else if (TOFbitmap == "0100") { // opponent likely nearly centered
    return IMU_Theta - 2*PI/180;
  } else if (TOFbitmap == "0110") { // opponent locked, stop rotation
    return IMU_Theta;
  } else if (TOFbitmap == "0111" || TOFbitmap == "0101") {
    return IMU_Theta + 3*PI/180;
  } else if (TOFbitmap == "1000") {
    return IMU_Theta - 9*PI/180;
  } else if (TOFbitmap == "1001") { //weird case
    return control_theta;
  } else if (TOFbitmap == "1011") { //weird case
    return control_theta;
  } else if (TOFbitmap == "1100") {
    return IMU_Theta - 6*PI/180;
  } else if (TOFbitmap == "1101") { //weird case
    return control_theta;
  } else if (TOFbitmap == "1110" || TOFbitmap == "1010") {
    return IMU_Theta - 3*PI/180;
  } else if (TOFbitmap == "1111") { // opponent locked, stop rotation
    return IMU_Theta;
  }
  return control_theta; // this should never be reached, but no nothing just in case
}

void UpdateTOF() {
  for (uint8_t i = 0; i < sensorCount; i++) {
    if(sensors[i].dataReady()){
      sensors[i].read(false);
      uint8_t range_status = sensors[i].ranging_data.range_status;
      uint16_t range = sensors[i].ranging_data.range_mm;
      if (range < 50) { // too close
        sensor_data[i] = 65535;
        range_status = 20;
      } else if (range_status == 0) { // if object detected
        sensor_data[i] = range;
      } else if ((range_status == 2 || range_status == 7) && (double)abs(getRobotDist() - range)/(double)range < 0.3) { // if close to other sensors
        sensor_data[i] = range;
        range_status = 21;
      } else { // other case
        sensor_data[i] = 65535;
      }
      // sensor_data[i] = range;

      sensor_status[i] = range_status;
      time_taken[i] = millis() - last_reading_time[i];
      last_reading_time[i] = millis();
    }
  }
}

int clampPulse(int pulse) {
  int upper_edge = 1517; // 1512
  int mid = 1487;
  int lower_edge = 1457; // 1462
  int deadzone = 25;
  if (upper_edge > pulse && mid < pulse) {pulse = upper_edge;}
  if (lower_edge < pulse && mid > pulse) {pulse = lower_edge;}
  if (abs(mid - pulse) < deadzone) {pulse = mid;}
  return pulse;
}

void DriveMotors(double F, double R, double L) {
  if (millis() - last_update_time > TIMEOUT){ // Stop the motors if communication drop
    MotorF.writeMicroseconds(1487);
    MotorR.writeMicroseconds(1487);
    MotorL.writeMicroseconds(1487);
  } else {
    F = constrain(F, -1, 1);
    R = constrain(R, -1, 1);
    L = -1*constrain(L, -1, 1);
    // if (0.04 > F && 0 < F) {F = 0.04;}
    // if (-0.04 < F && 0 > F) {F = -0.04;}
    // MotorF.write((F*90)+90);
    // MotorR.write((R*90)+90);
    // MotorL.write((L*90)+90);
    // MotorF.writeMicroseconds(1500 + 255*F);
    // MotorR.writeMicroseconds(1500 + 255*R);
    // MotorL.writeMicroseconds(1500 + 255*L);
    int pulseF = clampPulse(1487 + 470*F);
    int pulseR = clampPulse(1487 + 470*R);
    int pulseL = clampPulse(1487 + 470*L);
    MotorF.writeMicroseconds(pulseF);
    MotorR.writeMicroseconds(pulseR);
    MotorL.writeMicroseconds(pulseL);
  }
}

void ManualControl() {
  uint8_t x_in = (int)controls[0];
  uint8_t y_in = (int)controls[1];
  uint8_t theta_in = (int)controls[2];
  double x = (x_in - 128)/127.0;
  double y = (y_in - 128)/-127.0;
  double theta = (theta_in - 128)/-127.0;
  if (sqrt(x*x + y*y) < 0.06) { // dead zone
    x = 0;
    y = 0;
  }
  if (abs(theta) < 0.06) { // dead zone
    theta = 0;
  }
  // control scale
  x = (x > 0) ? pow(x, 2) : -1 * pow(x, 2);
  y = (y > 0) ? pow(y, 2) : -1 * pow(y, 2);
  theta = (theta > 0) ? pow(theta, 2) : -1 * pow(theta, 2);
  double control_power = sqrt(x*x + y*y);
  double control_angle = atan2 (y,x);

  double motorF_AngleDiff = (control_angle + PI/2);
  double motorPowerF = control_power*sin(motorF_AngleDiff) - theta;

  double motorR_AngleDiff = (control_angle + 7*PI/6);
  double motorPowerR = control_power*sin(motorR_AngleDiff) - theta;

  double motorL_AngleDiff = (control_angle - PI/6);
  double motorPowerL = control_power*sin(motorL_AngleDiff) - theta;
  DriveMotors(motorPowerF, motorPowerR, motorPowerL);
}

void AssistedControl() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  IMU_Delta_T = (millis() - IMU_Last_Reading_Time)/1000.0;
  IMU_Last_Reading_Time = millis();
  IMU_Theta += g.gyro.z*IMU_Delta_T;

  uint8_t x_in = (int)controls[0];
  uint8_t y_in = (int)controls[1];
  uint8_t theta_in = (int)controls[2];
  double x = (x_in - 128)/127.0;
  double y = (y_in - 128)/-127.0;
  double theta = (theta_in - 128)/-127.0;
  if (sqrt(x*x + y*y) < 0.06) { // dead zone
    x = 0;
    y = 0;
  }
  if (abs(theta) < 0.06) { // dead zone
    theta = 0;
  }
  // control scale
  x = (x > 0) ? pow(x, 2) : -1 * pow(x, 2);
  y = (y > 0) ? pow(y, 2) : -1 * pow(y, 2);
  theta = (theta > 0) ? pow(theta, 2) : -1 * pow(theta, 2);

  control_theta += theta;
  control_theta += 0.02*(IMU_Theta - control_theta);
  if ((int)controls[10] > 128) { // left bumper pressed
    control_theta = OpponentLock();
    if (y > 0.25) { // attack detection
      x = 0;
    }
  }
  double control_power = sqrt(x*x + y*y);
  double control_angle = atan2(y,x);

  double drive_theta = 0.3*(control_theta - IMU_Theta);

  // stop control if close enough
  if (abs(drive_theta) < 0.01) {
    drive_theta = 0;
  }

  double motorF_AngleDiff = (control_angle + PI/2);
  double motorPowerF = control_power*sin(motorF_AngleDiff) - drive_theta;

  double motorR_AngleDiff = (control_angle + 7*PI/6);
  double motorPowerR = control_power*sin(motorR_AngleDiff) - drive_theta;

  double motorL_AngleDiff = (control_angle - PI/6);
  double motorPowerL = control_power*sin(motorL_AngleDiff) - drive_theta;
  DriveMotors(motorPowerF, motorPowerR, motorPowerL);
}

void loop(){
  UpdateTOF();
  if ((int)controls[6] > 128) { // A button pressed
    DriveMode = 0;
  } else if ((int)controls[7] > 128) { // B button pressed
    DriveMode = 1;
    control_theta = IMU_Theta;
  }
  if (DriveMode == 0) {
    ManualControl();
  } else if (DriveMode == 1) {
    AssistedControl();
  }
}
