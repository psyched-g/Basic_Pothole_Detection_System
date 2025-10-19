#include <Wire.h>
#include <MPU6050.h>
#include <LiquidCrystal_I2C.h>

MPU6050 mpu(0x68);                     
LiquidCrystal_I2C lcd(0x27, 16, 2);   

// ---------------- Pins ----------------
const int TRIG_PIN = 6;      
const int ECHO_PIN = 7;      
const int BUZZER_PIN = 11;    

// ---------------- Settings ----------------
float baselineZ = 0;          
int accelThreshold = 15000;   
int calibrationSamples = 100;
int readDelay = 100;         

float bumpDistanceThreshold = 20.0; // Distance in cm to trigger buzzer

void setup() {
  Serial.begin(9600);
  Wire.begin();
  delay(200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  // Initialize MPU6050
  mpu.initialize();
  delay(100);
  Serial.println("MPU6050 initialized. Calibrating baseline...");

  // Calibrate Z-axis
  long sumZ = 0;
  int16_t ax, ay, az;
  for (int i = 0; i < calibrationSamples; i++) {
    mpu.getAcceleration(&ax, &ay, &az);
    sumZ += az;
    delay(5);
  }
  baselineZ = sumZ / calibrationSamples;
  Serial.print("Baseline Z: "); Serial.println(baselineZ);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pothole Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Ready...");
}

void loop() {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  long diffZ = abs(az - baselineZ);

  // Measure distance with ultrasonic sensor
  long duration;
  float distanceCm;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  distanceCm = duration * 0.034 / 2;

  // Detect pothole using MPU6050 (for display only)
  if (diffZ > accelThreshold) {
    lcd.setCursor(0, 0);
    lcd.print("⚠ Pothole Detected ");
    Serial.println("⚠ Pothole Detected!");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Road Normal       ");
  }

  // Turn on buzzer if ultrasonic detects a bump
  if (distanceCm < bumpDistanceThreshold) {
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.print("🔊 Bump Detected! Distance: "); Serial.print(distanceCm); Serial.println(" cm");
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Display distance
  lcd.setCursor(0, 1);
  lcd.print("Dist: ");
  lcd.print(distanceCm, 1);
  lcd.print(" cm        ");

  // Print accelerometer values and distance in Serial Monitor
  Serial.print("Accel X: "); Serial.print(ax);
  Serial.print(" | Y: "); Serial.print(ay);
  Serial.print(" | Z: "); Serial.print(az);
  Serial.print(" | ΔZ: "); Serial.print(diffZ);
  Serial.print(" | Distance: "); Serial.print(distanceCm);
  Serial.println(" cm");

  delay(readDelay);
}
