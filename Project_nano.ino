#include <Wire.h>
#include <MPU6050.h>
#include <SPI.h>
#include <RF24.h>
#include <math.h>

MPU6050 mpu;
RF24 radio(9, 10); // CE, CSN pins

const byte address[6] = "00001";

struct DataPacket {
  char direction;
  int speed;
};
DataPacket data;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  }

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // Calculate pitch angle
  float pitch = atan2(ax, sqrt(ay * ay + az * az)) * 180.0 / PI;

  // Map pitch angle to speed: more tilt = more speed
  int speed = map(constrain(abs(pitch), 0, 45), 0, 45, 100, 255);
  speed = constrain(speed, 0, 255);

  // Determine direction based on ay and ax values
  if (ay > 10000) data.direction = 'F';   // Forward
  else if (ay < -10000) data.direction = 'B'; // Backward
  else if (ax > 10000) data.direction = 'L';  // Left
  else if (ax < -10000) data.direction = 'R'; // Right
  else data.direction = 'S'; // Stop

  data.speed = speed;

  // Send data
  radio.write(&data, sizeof(data));

  delay(100);
}
