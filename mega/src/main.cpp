///////////////////////////////////////////////////////////////////
// #include <Arduino.h>
// #include <AccelStepper.h>

// // --- Robot Parameters ---
// const float wheelRadius = 7.5;   // cm
// const float trackWidth  = 59.0;  // cm
// const int ENCODER_PPR   = 1000;
// const float wheelCircumference = 2.0 * PI * wheelRadius;
// const float stepsPerCm = (200.0 * 16.0) / wheelCircumference;

// // --- Pins ---
// AccelStepper LeftWheel(AccelStepper::DRIVER, 26, 16);
// AccelStepper RightWheel(AccelStepper::DRIVER, 25, 27);
// #define ENC_L_A 2
// #define ENC_L_B 3
// #define ENC_R_A 14  // สลับมาพินนี้เพื่อหลบ Serial1
// #define ENC_R_B 15
// #define IMU_SERIAL Serial1

// // --- Variables ---
// volatile long encoderL = 0, encoderR = 0;
// long lastEncL = 0, lastEncR = 0;
// float x = 0, y = 0, theta = 0;
// float inputVx = 0, inputWz = 0;
// unsigned long lastCommand = 0;
// float imu_yaw = 0;
// bool imu_init = false;
// float initial_yaw = 0;

// // --- ISR ---
// void ISR_L() { (digitalRead(ENC_L_B)) ? encoderL++ : encoderL--; }
// void ISR_R() { (digitalRead(ENC_R_B)) ? encoderR++ : encoderR--; }

// void updateIMU() {
//     while (IMU_SERIAL.available()) {
//         static uint8_t buf[11], idx = 0;
//         uint8_t d = IMU_SERIAL.read();
//         if (idx == 0 && d != 0x55) continue;
//         buf[idx++] = d;
//         if (idx == 11) {
//             if (buf[1] == 0x53) {
//                 short raw = (short)(buf[7] << 8 | buf[6]);
//                 float val = (float)raw / 32768.0 * PI;
//                 if (!imu_init) { initial_yaw = val; imu_init = true; }
//                 imu_yaw = val - initial_yaw;
//             }
//             idx = 0;
//         }
//     }
// }

// void setup() {
//     Serial.begin(115200);   // ส่งไป ROS2
//     IMU_SERIAL.begin(9600); // รับจาก IMU
//     pinMode(ENC_L_A, INPUT_PULLUP); pinMode(ENC_L_B, INPUT_PULLUP);
//     pinMode(ENC_R_A, INPUT_PULLUP); pinMode(ENC_R_B, INPUT_PULLUP);
//     attachInterrupt(digitalPinToInterrupt(ENC_L_A), ISR_L, CHANGE);
//     attachInterrupt(digitalPinToInterrupt(ENC_R_A), ISR_R, CHANGE);
//     LeftWheel.setMaxSpeed(10000); RightWheel.setMaxSpeed(10000);
// }

// void loop() {
//     updateIMU();

//     // Read CMD_VEL from ROS2
//     if (Serial.available()) {
//         inputVx = Serial.parseFloat();
//         float dummy = Serial.parseFloat(); // skip vy
//         inputWz = Serial.parseFloat();
//         while(Serial.available() && Serial.read() != '\n'); // clear buffer
//         lastCommand = millis();
//     }

//     if (millis() - lastCommand > 500) { inputVx = 0; inputWz = 0; }

//     // Motor Control
//     float vL = (inputVx * 100.0) - (inputWz * trackWidth / 2.0);
//     float vR = (inputVx * 100.0) + (inputWz * trackWidth / 2.0);
//     LeftWheel.setSpeed(vL * stepsPerCm);
//     RightWheel.setSpeed(-vR * stepsPerCm);
//     LeftWheel.runSpeed();
//     RightWheel.runSpeed();

//     // Odom Calculation
//     static unsigned long lastOdom = 0;
//     if (millis() - lastOdom > 50) {
//         long cL = encoderL, cR = encoderR;
//         float dL = (cL - lastEncL) * (wheelCircumference / 100.0) / (ENCODER_PPR * 2);
//         float dR = (cR - lastEncR) * (wheelCircumference / 100.0) / (ENCODER_PPR * 2);
//         lastEncL = cL; lastEncR = cR;
//         float dS = (dL + dR) / 2.0;
//         theta = (imu_init) ? imu_yaw : theta + (dR - dL) / (trackWidth / 100.0);
//         x += dS * cos(theta); y += dS * sin(theta);

//         // บังคับส่งให้ชัดเจน
//         Serial.print("O "); Serial.print(x, 4); Serial.print(" ");
//         Serial.print(y, 4); Serial.print(" "); Serial.println(theta, 4);
//         lastOdom = millis();
//     }
// }
////////////////////////////////////best
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

// #include <Arduino.h>
// #include <AccelStepper.h>

// // --- Robot Parameters ---
// const float wheelRadius = 7.5;   // cm
// const float trackWidth  = 47.9;  // cm
// const int ENCODER_PPR   = 1000;  // Pulse Per Revolution
// const float wheelCircumference = 2.0 * PI * wheelRadius;
// const float gearRatio = 5.0;
// // const float stepsPerCm = (200.0 * 16.0) / wheelCircumference;
// const float stepsPerCm = (200.0 * 16.0 * gearRatio) / wheelCircumference;
// // --- Pins (Arduino Mega Interrupts: 2, 3, 18, 19, 20, 21) ---
// AccelStepper LeftWheel(AccelStepper::DRIVER, 26, 15);
// AccelStepper RightWheel(AccelStepper::DRIVER, 25, 27);

// #define ENC_L_A 2   // Interrupt Pin
// #define ENC_L_B 3
// #define ENC_R_A 18  // Interrupt Pin (ย้ายจาก 14)
// #define ENC_R_B 19  // (ย้ายจาก 15)
// #define IMU_SERIAL Serial2

// // --- Variables ---
// volatile long encoderL = 0, encoderR = 0;
// long lastEncL = 0, lastEncR = 0;
// float x = 0, y = 0, theta = 0;
// float inputVx = 0, inputWz = 0;
// unsigned long lastCommand = 0;
// float imu_yaw = 0;
// bool imu_init = false;
// float initial_yaw = 0;

// // --- ISR (Interrupt Service Routines) ---
// void ISR_L() {
//     if (digitalRead(ENC_L_A) == digitalRead(ENC_L_B)) encoderL--;
//     else encoderL++;
// }

// void ISR_R() {
//     if (digitalRead(ENC_R_A) == digitalRead(ENC_R_B)) encoderR++;
//     else encoderR--;
// }

// void updateIMU() {
//     while (IMU_SERIAL.available()) {
//         static uint8_t buf[11], idx = 0;
//         uint8_t d = IMU_SERIAL.read();
//         if (idx == 0 && d != 0x55) continue;
//         buf[idx++] = d;
//         if (idx == 11) {
//             if (buf[1] == 0x53) {
//                 short raw = (short)(buf[7] << 8 | buf[6]);
//                 float val = (float)raw / 32768.0 * PI;
//                 if (!imu_init) {
//                     initial_yaw = val;
//                     imu_init = true;
//                 }
//                 imu_yaw = val - initial_yaw;
//                 // Normalize to -PI to PI
//                 if (imu_yaw > PI) imu_yaw -= 2 * PI;
//                 if (imu_yaw < -PI) imu_yaw += 2 * PI;
//             }
//             idx = 0;
//         }
//     }
// }

// void setup() {
//     Serial.begin(115200);
//     IMU_SERIAL.begin(9600);

//     pinMode(ENC_L_A, INPUT_PULLUP);
//     pinMode(ENC_L_B, INPUT_PULLUP);
//     pinMode(ENC_R_A, INPUT_PULLUP);
//     pinMode(ENC_R_B, INPUT_PULLUP);

//     // ใช้ digitalPinToInterrupt เพื่อความถูกต้อง
//     attachInterrupt(digitalPinToInterrupt(ENC_L_A), ISR_L, CHANGE);
//     attachInterrupt(digitalPinToInterrupt(ENC_R_A), ISR_R, CHANGE);

//     LeftWheel.setMaxSpeed(10000);
//     RightWheel.setMaxSpeed(10000);
// }

// void loop() {
//     updateIMU();

//     // --- รับคำสั่งจาก Serial (Vx, Dummy, Wz) ---
//     if (Serial.available()) {
//         inputVx = Serial.parseFloat();
//         float dummy = Serial.parseFloat();
//         inputWz = Serial.parseFloat();
//         while(Serial.available() && Serial.read() != '\n');
//         lastCommand = millis();
//     }

//     // Safety Timeout
//     if (millis() - lastCommand > 500) {
//         inputVx = 0;
//         inputWz = 0;
//     }

//     // --- Motor Control ---
//     float vL = (inputVx * 100.0) - (inputWz * trackWidth / 2.0);
//     float vR = (inputVx * 100.0) + (inputWz * trackWidth / 2.0);

//     LeftWheel.setSpeed(-vL * stepsPerCm);
//     RightWheel.setSpeed(vR * stepsPerCm); // ล้อขวากลับทิศทางมอเตอร์

//     LeftWheel.runSpeed();
//     RightWheel.runSpeed();

//     // --- Odometry Calculation (20Hz) ---
//     static unsigned long lastOdom = 0;
//     if (millis() - lastOdom > 50) {
//         noInterrupts();
//         long currentL = encoderL;
//         long currentR = encoderR;
//         interrupts();

//         float dL = (float)(currentL - lastEncL) * (wheelCircumference / 100.0) / (ENCODER_PPR * 2);
//         float dR = (float)(currentR - lastEncR) * (wheelCircumference / 100.0) / (ENCODER_PPR * 2);

//         // float dL = (float)(currentL - lastEncL) * (wheelCircumference / 100.0) / (ENCODER_PPR * 2 * gearRatio);
//         // float dR = (float)(currentR - lastEncR) * (wheelCircumference / 100.0) / (ENCODER_PPR * 2 * gearRatio);
//         lastEncL = currentL;
//         lastEncR = currentR;

//         float dS = (dL + dR) / 2.0;

//         if (imu_init) {
//             theta = imu_yaw;
//         } else {
//             theta += (dR - dL) / (trackWidth / 100.0);
//         }

//         x += dS * cos(theta);
//         y += dS * sin(theta);

//         Serial.print("O ");
//         Serial.print(x, 4); Serial.print(" ");
//         Serial.print(y, 4); Serial.print(" ");
//         Serial.println(theta, 4);

//         lastOdom = millis();
//     }
// }

// #include <Arduino.h>
// #include <AccelStepper.h>

// AccelStepper LeftWheel(AccelStepper::DRIVER, 26, 15);
// AccelStepper RightWheel(AccelStepper::DRIVER, 25, 27);
// #define ENC_L_A 2
// #define ENC_L_B 3
// #define ENC_R_A 18
// #define ENC_R_B 19
// #define IMU_SERIAL Serial2

// volatile long encoderL = 0, encoderR = 0;
// float imu_yaw = 0, initial_yaw = 0;
// bool imu_init = false;

// void ISR_L() { (digitalRead(ENC_L_A) == digitalRead(ENC_L_B)) ? encoderL-- : encoderL++; }
// void ISR_R() { (digitalRead(ENC_R_A) == digitalRead(ENC_R_B)) ? encoderR++ : encoderR--; }

// // แยกฟังก์ชันอัปเดต IMU ออกมา
// void updateIMU() {
//     while (IMU_SERIAL.available()) {
//         static uint8_t buf[11], idx = 0;
//         uint8_t d = IMU_SERIAL.read();
//         if (idx == 0 && d != 0x55) continue;
//         buf[idx++] = d;
//         if (idx == 11) {
//             if (buf[1] == 0x53) { // 0x53 คือมุม (Euler angles)
//                 short raw = (short)(buf[7] << 8 | buf[6]);
//                 float val = (float)raw / 32768.0 * PI;
//                 if (!imu_init) { initial_yaw = val; imu_init = true; }
//                 imu_yaw = val - initial_yaw;
//                 // Normalize yaw -PI to PI
//                 if (imu_yaw > PI) imu_yaw -= 2*PI;
//                 if (imu_yaw < -PI) imu_yaw += 2*PI;
//             }
//             idx = 0;
//         }
//     }
// }

// void setup() {
//     Serial.begin(115200);
//     IMU_SERIAL.begin(9600);
//     pinMode(ENC_L_A, INPUT_PULLUP); pinMode(ENC_L_B, INPUT_PULLUP);
//     pinMode(ENC_R_A, INPUT_PULLUP); pinMode(ENC_R_B, INPUT_PULLUP);
//     attachInterrupt(digitalPinToInterrupt(ENC_L_A), ISR_L, CHANGE);
//     attachInterrupt(digitalPinToInterrupt(ENC_R_A), ISR_R, CHANGE);
//     LeftWheel.setMaxSpeed(20000); RightWheel.setMaxSpeed(20000);
// }

// void loop() {
//     // 1. รับค่าความเร็ว (ใช้ if เช็คเพื่อไม่ให้ Block imu)
//     if (Serial.available() > 0) {
//         char startChar = Serial.peek();
//         if (startChar != 'E') { // ป้องกันอ่านค่าที่ตัวเองส่งออกไป (ถ้ามี Loopback)
//             float l_rpm = Serial.parseFloat();
//             float r_rpm = Serial.parseFloat();
//             while(Serial.available() && Serial.read() != '\n');

//             float factor = (200.0 * 16.0) / 60.0;
//             LeftWheel.setSpeed(l_rpm * factor);
//             RightWheel.setSpeed(r_rpm * factor);
//         } else {
//             Serial.read(); // เคลียร์ตัว E ทิ้งถ้าหลงเข้ามา
//         }
//     }

//     LeftWheel.runSpeed();
//     RightWheel.runSpeed();

//     updateIMU(); // เรียกอัปเดต IMU ทุกรอบ Loop

//     static unsigned long lastSend = 0;
//     if (millis() - lastSend > 50) {
//         Serial.print("E ");
//         Serial.print(encoderL); Serial.print(" ");
//         Serial.print(encoderR); Serial.print(" ");
//         Serial.println(imu_yaw, 4);
//         lastSend = millis();
//     }
// }