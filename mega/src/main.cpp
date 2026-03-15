
#include <Arduino.h>
#include <AccelStepper.h>

// --- Robot Parameters ---
const float wheelRadius = 7.5;   // cm
const float trackWidth  = 59.0;  // cm
const int ENCODER_PPR   = 1000;
const float wheelCircumference = 2.0 * PI * wheelRadius;
const float stepsPerCm = (200.0 * 16.0) / wheelCircumference;

// --- Pins ---
AccelStepper LeftWheel(AccelStepper::DRIVER, 26, 15);
AccelStepper RightWheel(AccelStepper::DRIVER, 25, 27);
#define ENC_L_A 2
#define ENC_L_B 3
#define ENC_R_A 18
#define ENC_R_B 19
#define IMU_SERIAL Serial2

// --- Variables ---
volatile long encoderL = 0, encoderR = 0;
long lastEncL = 0, lastEncR = 0;
float x = 0, y = 0, theta = 0;
float inputVx = 0, inputWz = 0;
unsigned long lastCommand = 0;
float imu_yaw = 0;
bool imu_init = false;
float initial_yaw = 0;

// --- ISR (ตรวจสอบทิศทางการนับตรงนี้) ---
// ถ้าเข็นรถไปข้างหน้า แล้วค่า X ติดลบ ให้สลับเครื่องหมาย ++ เป็น --
void ISR_L() { (digitalRead(ENC_L_B)) ? encoderL++ : encoderL--; }
void ISR_R() { (digitalRead(ENC_R_B)) ? encoderR-- : encoderR++; } // สลับล้อขวาให้สัมพันธ์กับมอเตอร์

void updateIMU() {
    while (IMU_SERIAL.available()) {
        static uint8_t buf[11], idx = 0;
        uint8_t d = IMU_SERIAL.read();
        if (idx == 0 && d != 0x55) continue;
        buf[idx++] = d;
        if (idx == 11) {
            if (buf[1] == 0x53) {
                short raw = (short)(buf[7] << 8 | buf[6]);
                float val = (float)raw / 32768.0 * PI;
                if (!imu_init) { initial_yaw = val; imu_init = true; }
                imu_yaw = val - initial_yaw;
            }
            idx = 0;
        }
    }
}

void setup() {
    Serial.begin(115200);
    IMU_SERIAL.begin(9600);
    pinMode(ENC_L_A, INPUT_PULLUP); pinMode(ENC_L_B, INPUT_PULLUP);
    pinMode(ENC_R_A, INPUT_PULLUP); pinMode(ENC_R_B, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENC_L_A), ISR_L, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_R_A), ISR_R, CHANGE);
    LeftWheel.setMaxSpeed(10000); RightWheel.setMaxSpeed(10000);
}

void loop() {
    updateIMU();

    if (Serial.available()) {
        inputVx = Serial.parseFloat();
        float dummy = Serial.parseFloat();
        inputWz = Serial.parseFloat();
        while(Serial.available() && Serial.read() != '\n');
        lastCommand = millis();
    }

    if (millis() - lastCommand > 500) { inputVx = 0; inputWz = 0; }

    // --- Motor Control (ปรับทิศทางล้อ) ---
    float vL = (inputVx * 100.0) - (inputWz * trackWidth / 2.0);
    float vR = (inputVx * 100.0) + (inputWz * trackWidth / 2.0);

    LeftWheel.setSpeed(-vL * stepsPerCm);
    RightWheel.setSpeed(vR * stepsPerCm); // ล้อขวาติดลบเพราะมอเตอร์หันหน้าออกคนละด้าน

    LeftWheel.runSpeed();
    RightWheel.runSpeed();

    // --- Odom Calculation ---
    static unsigned long lastOdom = 0;
    if (millis() - lastOdom > 50) {
        long cL = encoderL, cR = encoderR;
        // คำนวณระยะทางที่เปลี่ยนไป (Unit: Meters)
        float dL = (cL - lastEncL) * (wheelCircumference / 100.0) / (ENCODER_PPR * 2);
        float dR = (cR - lastEncR) * (wheelCircumference / 100.0) / (ENCODER_PPR * 2);
        lastEncL = cL; lastEncR = cR;

        float dS = (dL + dR) / 2.0;
        // ใช้ IMU Yaw เป็นตัวกำหนดทิศทางหลัก
        if (imu_init) {
            theta = imu_yaw;
        } else {
            theta += (dR - dL) / (trackWidth / 100.0);
        }

        x += dS * cos(theta);
        y += dS * sin(theta);

        Serial.print("O "); Serial.print(x, 4); Serial.print(" ");
        Serial.print(y, 4); Serial.print(" "); Serial.println(theta, 4);
        lastOdom = millis();
    }
}
