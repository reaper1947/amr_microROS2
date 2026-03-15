// #include <Arduino.h>
// #include <micro_ros_platformio.h>
// #include <AccelStepper.h>

// #include <rcl/rcl.h>
// #include <rclc/rclc.h>
// #include <rclc/executor.h>

// #include <geometry_msgs/msg/twist.h>
// #include <nav_msgs/msg/odometry.h>

// // --- Pins Config (WROOM-32) ---
// #define STEP_L 26
// #define DIR_L  25
// #define STEP_R 33
// #define DIR_R  32
// #define EN_PIN 27

// // --- Robot Parameters ---
// const float track_width = 0.59;
// const float steps_per_meter = 6790.6;

// // --- Objects ---
// AccelStepper LeftWheel(AccelStepper::DRIVER, STEP_L, DIR_L);
// AccelStepper RightWheel(AccelStepper::DRIVER, STEP_R, DIR_R);

// // --- micro-ROS Objects ---
// rcl_subscription_t subscriber;
// geometry_msgs__msg__Twist msg_cmd_vel;
// rcl_publisher_t odom_publisher;
// nav_msgs__msg__Odometry odom_msg;
// rclc_executor_t executor;
// rclc_support_t support;
// rcl_allocator_t allocator;
// rcl_node_t node;

// unsigned long last_odom_update = 0;

// // Macro สำหรับเช็ค Error
// #define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){return;}}

// void cmd_vel_callback(const void * msgin) {
//     const geometry_msgs__msg__Twist * msg = (const geometry_msgs__msg__Twist *)msgin;
//     float v = msg->linear.x;
//     float w = msg->angular.z;

//     // คำนวณความเร็วล้อซ้าย-ขวา
//     float vLeft = v - (w * track_width / 2.0);
//     float vRight = v + (w * track_width / 2.0);

//     // ตั้งค่าความเร็วให้มอเตอร์ (Steps per second)
//     LeftWheel.setSpeed(vLeft * steps_per_meter);
//     RightWheel.setSpeed(-vRight * steps_per_meter); // มอเตอร์ขวากลับทิศเพื่อให้เดินหน้าไปทางเดียวกัน
// }

// void setup() {
//     // ใช้ Serial สำหรับ WROOM-32 (/dev/ttyUSB0)
//     Serial.begin(115200);
//     set_microros_serial_transports(Serial);

//     // มอเตอร์ Setup
//     LeftWheel.setMaxSpeed(15000);
//     RightWheel.setMaxSpeed(15000);
//     pinMode(EN_PIN, OUTPUT);
//     digitalWrite(EN_PIN, HIGH); // LOW = Enable (ตรวจสอบไดรเวอร์ของคุณอีกที)

//     // รอ Agent เชื่อมต่อ
//     while (rmw_uros_ping_agent(100, 1) != RCL_RET_OK) {
//         delay(100);
//     }

//     // micro-ROS Initial
//     allocator = rcl_get_default_allocator();
//     RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
//     RCCHECK(rclc_node_init_default(&node, "esp32_robot_node", "", &support));

//     // จอง Memory สำหรับ Odometry
//     static char frame_id[] = "odom";
//     static char child_id[] = "base_link";
//     odom_msg.header.frame_id.data = frame_id;
//     odom_msg.header.frame_id.capacity = strlen(frame_id) + 1;
//     odom_msg.header.frame_id.size = strlen(frame_id);
//     odom_msg.child_frame_id.data = child_id;
//     odom_msg.child_frame_id.capacity = strlen(child_id) + 1;
//     odom_msg.child_frame_id.size = strlen(child_id);

//     // สร้าง Publisher และ Subscriber
//     RCCHECK(rclc_publisher_init_default(&odom_publisher, &node,
//         ROSIDL_GET_MSG_TYPE_SUPPORT(nav_msgs, msg, Odometry), "odom"));
//     RCCHECK(rclc_subscription_init_default(&subscriber, &node,
//         ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "cmd_vel"));

//     // สร้าง Executor
//     RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
//     RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg_cmd_vel, &cmd_vel_callback, ON_NEW_DATA));
// }

// void loop() {
//     // รัน micro-ROS (ใช้เวลาประมวลผลน้อยที่สุด)
//     rclc_executor_spin_some(&executor, RCL_MS_TO_NS(0));

//     // เลื่อนตำแหน่งมอเตอร์ตามจังหวะเวลา
//     LeftWheel.runSpeed();
//     RightWheel.runSpeed();

//     // ส่งค่า Odometry ทุก 100ms
//     if (millis() - last_odom_update >= 100) {
//         odom_msg.header.stamp.sec = millis() / 1000;
//         odom_msg.header.stamp.nanosec = (millis() % 1000) * 1000000;

//         rcl_ret_t ret = rcl_publish(&odom_publisher, &odom_msg, NULL);
//         (void)ret;

//         last_odom_update = millis();
//     }
// }
// #include <Arduino.h>
// #include <micro_ros_platformio.h>
// #include <rcl/rcl.h>
// #include <rclc/rclc.h>
// #include <rclc/executor.h>
// #include <geometry_msgs/msg/twist.h>
// #include <nav_msgs/msg/odometry.h>
// #include <AccelStepper.h>

// // --- Pins ---
// #define STEP_L 26
// #define DIR_L  15
// #define STEP_R 25
// #define DIR_R  27
// #define ENC_L_A 32
// #define ENC_L_B 33
// #define ENC_R_A 34
// #define ENC_R_B 35

// // --- Parameters ---
// const float wheelRadius = 0.075;
// const float trackWidth  = 0.59;
// const float wheelCircumference = 2.0 * PI * wheelRadius;
// const float stepsPerMeter = (200.0 * 16.0) / wheelCircumference;

// // --- Objects ---
// AccelStepper LeftWheel(AccelStepper::DRIVER, STEP_L, DIR_L);
// AccelStepper RightWheel(AccelStepper::DRIVER, STEP_R, DIR_R);

// // --- Micro-ROS Variables ---
// rcl_subscription_t subscriber;
// geometry_msgs__msg__Twist msg_cmd_vel;
// rcl_publisher_t odom_pub;
// nav_msgs__msg__Odometry msg_odom;
// rclc_executor_t executor;
// rclc_support_t support;
// rcl_allocator_t allocator;
// rcl_node_t node;

// // --- Global Variables ---
// volatile long encoderL = 0, encoderR = 0;
// float x = 0, y = 0, theta = 0;

// // --- ISR (Encoder) ---
// void IRAM_ATTR ISR_L() { (digitalRead(ENC_L_B)) ? encoderL++ : encoderL--; }
// void IRAM_ATTR ISR_R() { (digitalRead(ENC_R_B)) ? encoderR-- : encoderR++; }

// // --- ROS2 Callback ---
// void subscription_callback(const void * msgin) {
//     const geometry_msgs__msg__Twist * msg = (const geometry_msgs__msg__Twist *)msgin;
//     float vx = msg->linear.x;
//     float wz = msg->angular.z;

//     // Differential Drive Kinematics
//     float vL = vx - (wz * trackWidth / 2.0);
//     float vR = vx + (wz * trackWidth / 2.0);

//     // Set Speed (Steps per second)
//     LeftWheel.setSpeed(-vL * stepsPerMeter);
//     RightWheel.setSpeed(vR * stepsPerMeter);
// }

// void setup() {
//     // ใช้ Serial สำหรับ micro-ROS เท่านั้น ห้าม Serial.print() เพื่อป้องกัน Data Corrupt
//     Serial.begin(115200);
//     set_microros_serial_transports(Serial);

//     // Encoder Pins
//     pinMode(ENC_L_A, INPUT_PULLUP); pinMode(ENC_L_B, INPUT_PULLUP);
//     pinMode(ENC_R_A, INPUT_PULLUP); pinMode(ENC_R_B, INPUT_PULLUP);
//     attachInterrupt(digitalPinToInterrupt(ENC_L_A), ISR_L, CHANGE);
//     attachInterrupt(digitalPinToInterrupt(ENC_R_A), ISR_R, CHANGE);

//     // Stepper Config
//     LeftWheel.setMaxSpeed(10000);
//     RightWheel.setMaxSpeed(10000);

//     // Micro-ROS Initialization
//     allocator = rcl_get_default_allocator();
//     rclc_support_init(&support, 0, NULL, &allocator);
//     rclc_node_init_default(&node, "esp32_robot_node", "", &support);

//     // Init Subscriber (cmd_vel)
//     rclc_subscription_init_default(&subscriber, &node,
//         ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "cmd_vel");

//     // Init Publisher (odom)
//     rclc_publisher_init_default(&odom_pub, &node,
//         ROSIDL_GET_MSG_TYPE_SUPPORT(nav_msgs, msg, Odometry), "odom");

//     // Executor Setup (1 handles for subscription)
//     rclc_executor_init(&executor, &support.context, 1, &allocator);
//     rclc_executor_add_subscription(&executor, &subscriber, &msg_cmd_vel, &subscription_callback, ON_NEW_DATA);
// }

// void loop() {
//     // ปรับ Timeout เป็น 0 เพื่อให้ Stepper หมุนได้ต่อเนื่องไม่สะดุด
//     rclc_executor_spin_some(&executor, RCL_MS_TO_NS(0));

//     // รันมอเตอร์ตามความเร็วที่ตั้งไว้
//     LeftWheel.runSpeed();
//     RightWheel.runSpeed();

//     static unsigned long lastOdom = 0;
//     if (millis() - lastOdom > 50) {

//         msg_odom.header.stamp.sec = millis() / 1000;
//         msg_odom.header.stamp.nanosec = (millis() % 1000) * 1000000;
//         msg_odom.pose.pose.position.x = x;
//         msg_odom.pose.pose.position.y = y;
//         msg_odom.pose.pose.orientation.z = sin(theta / 2.0);
//         msg_odom.pose.pose.orientation.w = cos(theta / 2.0);

//         rcl_publish(&odom_pub, &msg_odom, NULL);
//         lastOdom = millis();
//     }
// }
// #include <Arduino.h>
// #include <micro_ros_platformio.h>
// #include <AccelStepper.h>

// #include <rcl/rcl.h>
// #include <rclc/rclc.h>
// #include <rclc/executor.h>

// #include <geometry_msgs/msg/twist.h>
// #include <nav_msgs/msg/odometry.h>

// // --- Pins Config (ปรับให้ใช้ขาปลอดภัยสำหรับ WROOM-32) ---
// #define STEP_L 26
// #define DIR_L  25  // เปลี่ยนจาก 15 เป็น 25 เพื่อความปลอดภัยในการ Boot
// #define STEP_R 33
// #define DIR_R  32

// // --- Parameters ---
// const float wheelRadius = 0.075;
// const float trackWidth  = 0.059;
// const float wheelCircumference = 2.0 * PI * wheelRadius;
// // คำนวณ stepsPerMeter: (Steps ต่อรอบ * Microstep) / เส้นรอบวงล้อ
// const float stepsPerMeter = (200.0 * 4.0 ) / wheelCircumference;

// // --- Objects ---
// AccelStepper LeftWheel(AccelStepper::DRIVER, STEP_L, DIR_L);
// AccelStepper RightWheel(AccelStepper::DRIVER, STEP_R, DIR_R);

// // --- Micro-ROS Variables ---
// rcl_subscription_t subscriber;
// geometry_msgs__msg__Twist msg_cmd_vel;
// rcl_publisher_t odom_pub;
// nav_msgs__msg__Odometry msg_odom;
// rclc_executor_t executor;
// rclc_support_t support;
// rcl_allocator_t allocator;
// rcl_node_t node;

// unsigned long lastOdom = 0;

// // --- ROS2 Callback ---
// void subscription_callback(const void * msgin) {
//     const geometry_msgs__msg__Twist * msg = (const geometry_msgs__msg__Twist *)msgin;
//     float vx = msg->linear.x;
//     float wz = msg->angular.z;

//     // Differential Drive Kinematics
//     float vL = vx - (wz * trackWidth / 2.0);
//     float vR = vx + (wz * trackWidth / 2.0);

//     // Set Speed (ใช้ตรรกะทิศทางที่คุณยืนยันว่าถูกต้อง)
//     LeftWheel.setSpeed(vL * stepsPerMeter);
//     RightWheel.setSpeed(-vR * stepsPerMeter);
// }

// void setup() {
//     // ใช้ Serial สำหรับ micro-ROS
//     Serial.begin(115200);
//     set_microros_serial_transports(Serial);

//     // Stepper Config
//     LeftWheel.setMaxSpeed(500000);
//     RightWheel.setMaxSpeed(500000);

//     // รอ Agent เชื่อมต่อ
//     while (rmw_uros_ping_agent(100, 1) != RCL_RET_OK) {
//         delay(100);
//     }

//     // Micro-ROS Initialization
//     allocator = rcl_get_default_allocator();
//     rclc_support_init(&support, 0, NULL, &allocator);
//     rclc_node_init_default(&node, "esp32_robot_node", "", &support);

//     // จัดการ Memory สำหรับ Header ของ Odometry
//     static char frame_id[] = "odom";
//     static char child_id[] = "base_link";
//     msg_odom.header.frame_id.data = frame_id;
//     msg_odom.header.frame_id.capacity = strlen(frame_id) + 1;
//     msg_odom.header.frame_id.size = strlen(frame_id);
//     msg_odom.child_frame_id.data = child_id;
//     msg_odom.child_frame_id.capacity = strlen(child_id) + 1;
//     msg_odom.child_frame_id.size = strlen(child_id);

//     // Init Subscriber (cmd_vel)
//     rclc_subscription_init_default(&subscriber, &node,
//         ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "cmd_vel");

//     // Init Publisher (odom)
//     rclc_publisher_init_default(&odom_pub, &node,
//         ROSIDL_GET_MSG_TYPE_SUPPORT(nav_msgs, msg, Odometry), "odom");

//     // Executor Setup
//     rclc_executor_init(&executor, &support.context, 1, &allocator);
//     rclc_executor_add_subscription(&executor, &subscriber, &msg_cmd_vel, &subscription_callback, ON_NEW_DATA);
// }

// void loop() {
//     // ประมวลผล micro-ROS
//     rclc_executor_spin_some(&executor, RCL_MS_TO_NS(0));

//     // รันมอเตอร์
//     LeftWheel.runSpeed();
//     RightWheel.runSpeed();

//     // Publish Odometry ทุก 50ms
//     if (millis() - lastOdom > 50) {
//         msg_odom.header.stamp.sec = millis() / 1000;
//         msg_odom.header.stamp.nanosec = (millis() % 1000) * 1000000;

//         // (ส่งค่า Publish)
//         rcl_publish(&odom_pub, &msg_odom, NULL);
//         lastOdom = millis();
//     }
// }

///good
// #include <Arduino.h>
// #include <micro_ros_platformio.h>
// #include <rcl/rcl.h>
// #include <rclc/rclc.h>
// #include <rclc/executor.h>
// #include <geometry_msgs/msg/twist.h>
// #include <nav_msgs/msg/odometry.h>
// #include "FastAccelStepper.h"
// #include <rosidl_runtime_c/string_functions.h>

// // --- Pins Config ---
// #define STEP_L 26
// #define DIR_L  25
// #define STEP_R 33
// #define DIR_R  32
// #define EN_PIN 27

// // --- Robot Parameters ---
// #define WHEEL_RADIUS 0.075
// #define TRACK_WIDTH 0.59
// #define GEAR_RATIO 10.0          // ตามที่คุณตั้งไว้ล่าสุด
// #define MICROSTEP 4.0            // 800 steps/rev
// #define TICKS_PER_REV (200.0 * MICROSTEP * GEAR_RATIO)
// #define METER_TO_STEPS (TICKS_PER_REV / (2.0 * PI * WHEEL_RADIUS))
// #define RAD_TO_STEPS (METER_TO_STEPS * TRACK_WIDTH / 2.0)

// // ปรับค่าความเร่งให้สูงมากเพื่อให้ตอบสนองทันที (เสมือนไม่มีความเร่ง)
// #define NO_ACCEL 1000000

// // --- Objects ---
// FastAccelStepperEngine engine = FastAccelStepperEngine();
// FastAccelStepper *lstepper = NULL;
// FastAccelStepper *rstepper = NULL;

// rcl_subscription_t subscriber;
// geometry_msgs__msg__Twist msg_cmd_vel;
// rcl_publisher_t odom_pub;
// nav_msgs__msg__Odometry msg_odom;
// rclc_executor_t executor;
// rclc_support_t support;
// rcl_allocator_t allocator;
// rcl_node_t node;

// double pose_x = 0.0, pose_y = 0.0, pose_theta = 0.0;
// long last_l_ticks = 0, last_r_ticks = 0;

// void cmd_vel_callback(const void * msgin) {
//     const geometry_msgs__msg__Twist * msg = (const geometry_msgs__msg__Twist *)msgin;

//     // คูณความเร็ว x2 จากคำสั่งที่ได้รับมา (ถ้าต้องการให้วิ่งเร็วขึ้นจาก input เดิม)
//     float vx = msg->linear.x * 2.0;
//     float wz = msg->angular.z * 2.0;

//     int32_t l_speed = (int32_t)(vx * METER_TO_STEPS - wz * RAD_TO_STEPS);
//     int32_t r_speed = (int32_t)(vx * METER_TO_STEPS + wz * RAD_TO_STEPS);

//     // ล้อซ้าย
//     if (l_speed == 0) {
//         lstepper->stopMove();
//     } else {
//         lstepper->setSpeedInHz(abs(l_speed));
//         if (l_speed > 0) lstepper->runForward(); else lstepper->runBackward();
//     }

//     // ล้อขวา
//     if (r_speed == 0) {
//         rstepper->stopMove();
//     } else {
//         rstepper->setSpeedInHz(abs(r_speed));
//         // ทิศทางล้อขวา (อ้างอิงตามที่คุณเคยใช้แล้วได้ผล)
//         if (r_speed > 0) rstepper->runBackward(); else rstepper->runForward();
//     }
// }

// void setup() {
//     Serial.begin(115200);
//     set_microros_serial_transports(Serial);

//     engine.init();
//     lstepper = engine.stepperConnectToPin(STEP_L);
//     if (lstepper) {
//         lstepper->setDirectionPin(DIR_L);
//         lstepper->setEnablePin(EN_PIN);
//         lstepper->setAutoEnable(true);
//         lstepper->setAcceleration(NO_ACCEL); // ตั้งค่าสูงมากเพื่อให้พุ่งทันที
//     }

//     rstepper = engine.stepperConnectToPin(STEP_R);
//     if (rstepper) {
//         rstepper->setDirectionPin(DIR_R);
//         rstepper->setEnablePin(EN_PIN);
//         rstepper->setAutoEnable(true);
//         rstepper->setAcceleration(NO_ACCEL); // ตั้งค่าสูงมากเพื่อให้พุ่งทันที
//     }

//     while (rmw_uros_ping_agent(100, 1) != RCL_RET_OK) { delay(100); }

//     allocator = rcl_get_default_allocator();
//     rclc_support_init(&support, 0, NULL, &allocator);
//     rclc_node_init_default(&node, "esp32_stepper_node", "", &support);

//     rosidl_runtime_c__String__assign(&msg_odom.header.frame_id, "odom");
//     rosidl_runtime_c__String__assign(&msg_odom.child_frame_id, "base_link");

//     rclc_subscription_init_default(&subscriber, &node,
//         ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "cmd_vel");
//     rclc_publisher_init_default(&odom_pub, &node,
//         ROSIDL_GET_MSG_TYPE_SUPPORT(nav_msgs, msg, Odometry), "odom");

//     rclc_executor_init(&executor, &support.context, 1, &allocator);
//     rclc_executor_add_subscription(&executor, &subscriber, &msg_cmd_vel, &cmd_vel_callback, ON_NEW_DATA);
// }

// void loop() {
//     rclc_executor_spin_some(&executor, RCL_MS_TO_NS(1));

//     static unsigned long lastOdom = 0;
//     if (millis() - lastOdom > 50) {
//         long curr_l = lstepper->getCurrentPosition();
//         long curr_r = -rstepper->getCurrentPosition();

//         double dl = (double)(curr_l - last_l_ticks) / METER_TO_STEPS;
//         double dr = (double)(curr_r - last_r_ticks) / METER_TO_STEPS;

//         last_l_ticks = curr_l;
//         last_r_ticks = curr_r;

//         double dc = (dl + dr) / 2.0;
//         double dt = (dr - dl) / TRACK_WIDTH;

//         pose_x += dc * cos(pose_theta);
//         pose_y += dc * sin(pose_theta);
//         pose_theta += dt;

//         msg_odom.header.stamp.sec = millis() / 1000;
//         msg_odom.header.stamp.nanosec = (millis() % 1000) * 1000000;
//         msg_odom.pose.pose.position.x = pose_x;
//         msg_odom.pose.pose.position.y = pose_y;
//         msg_odom.pose.pose.orientation.z = sin(pose_theta / 2.0);
//         msg_odom.pose.pose.orientation.w = cos(pose_theta / 2.0);

//         rcl_publish(&odom_pub, &msg_odom, NULL);
//         lastOdom = millis();
//     }
// }

// #include <Arduino.h>
// #include <micro_ros_platformio.h>
// #include <rcl/rcl.h>
// #include <rclc/rclc.h>
// #include <rclc/executor.h>
// #include <geometry_msgs/msg/twist.h>
// #include <nav_msgs/msg/odometry.h>
// #include <std_msgs/msg/float32.h> // เพิ่มสำหรับส่งค่า RPM
// #include "FastAccelStepper.h"
// #include <rosidl_runtime_c/string_functions.h>

// // --- Pins & Parameters (เหมือนเดิม) ---
// #define STEP_L 26
// #define DIR_L  25
// #define STEP_R 33
// #define DIR_R  32
// #define EN_PIN 27
// #define ENC_L_A 14
// #define ENC_L_B 12
// #define ENC_R_A 34
// #define ENC_R_B 35

// #define WHEEL_RADIUS 0.075
// #define TRACK_WIDTH  0.59
// #define GEAR_RATIO   5.0
// #define STEP_MICRO   4.0
// #define ENCODER_PPR  1000.0
// #define TICKS_PER_REV_STEP (200.0 * STEP_MICRO * GEAR_RATIO)

// const float METER_TO_STEPS = TICKS_PER_REV_STEP / (2.0 * PI * WHEEL_RADIUS);
// const float RAD_TO_STEPS   = (METER_TO_STEPS * TRACK_WIDTH / 2.0);
// const float METER_PER_TICK = (2.0 * PI * WHEEL_RADIUS) / (ENCODER_PPR * 2.0);

// // --- Variables ---
// FastAccelStepperEngine engine = FastAccelStepperEngine();
// FastAccelStepper *lstepper = NULL;
// FastAccelStepper *rstepper = NULL;

// volatile long encoderL = 0, encoderR = 0;
// long lastEncL = 0, lastEncR = 0;
// double pose_x = 0, pose_y = 0, pose_theta = 0;

// // micro-ROS Objects
// rcl_subscription_t subscriber;
// geometry_msgs__msg__Twist msg_cmd_vel;
// rcl_publisher_t odom_pub;
// nav_msgs__msg__Odometry msg_odom;

// // RPM Publishers
// rcl_publisher_t rpm_l_pub;
// rcl_publisher_t rpm_r_pub;
// std_msgs__msg__Float32 msg_rpm_l;
// std_msgs__msg__Float32 msg_rpm_r;

// rclc_executor_t executor;
// rclc_support_t support;
// rcl_allocator_t allocator;
// rcl_node_t node;

// // --- ISR ---
// void IRAM_ATTR ISR_L() { (digitalRead(ENC_L_B)) ? encoderL++ : encoderL--; }
// void IRAM_ATTR ISR_R() { (digitalRead(ENC_R_B)) ? encoderR-- : encoderR++; }

// void cmd_vel_callback(const void * msgin) {
//     const geometry_msgs__msg__Twist * msg = (const geometry_msgs__msg__Twist *)msgin;
//     float vx = msg->linear.x * 2.0;
//     float wz = msg->angular.z * 2.0;

//     int32_t l_speed = (int32_t)(vx * METER_TO_STEPS - wz * RAD_TO_STEPS);
//     int32_t r_speed = (int32_t)(vx * METER_TO_STEPS + wz * RAD_TO_STEPS);

//     if (l_speed == 0) lstepper->stopMove();
//     else {
//         lstepper->setSpeedInHz(abs(l_speed));
//         (l_speed > 0) ? lstepper->runForward() : lstepper->runBackward();
//     }

//     if (r_speed == 0) rstepper->stopMove();
//     else {
//         rstepper->setSpeedInHz(abs(r_speed));
//         (r_speed > 0) ? rstepper->runBackward() : rstepper->runForward();
//     }
// }

// void setup() {
//     Serial.begin(115200);
//     set_microros_serial_transports(Serial);

//     pinMode(ENC_L_A, INPUT_PULLUP); pinMode(ENC_L_B, INPUT_PULLUP);
//     pinMode(ENC_R_A, INPUT_PULLUP); pinMode(ENC_R_B, INPUT_PULLUP);
//     attachInterrupt(digitalPinToInterrupt(ENC_L_A), ISR_L, CHANGE);
//     attachInterrupt(digitalPinToInterrupt(ENC_R_A), ISR_R, CHANGE);

//     engine.init();
//     lstepper = engine.stepperConnectToPin(STEP_L);
//     lstepper->setDirectionPin(DIR_L);
//     lstepper->setEnablePin(EN_PIN);
//     lstepper->setAutoEnable(true);
//     lstepper->setAcceleration(1000000);

//     rstepper = engine.stepperConnectToPin(STEP_R);
//     rstepper->setDirectionPin(DIR_R);
//     rstepper->setEnablePin(EN_PIN);
//     rstepper->setAutoEnable(true);
//     rstepper->setAcceleration(1000000);

//     while (rmw_uros_ping_agent(100, 1) != RCL_RET_OK) { delay(100); }

//     allocator = rcl_get_default_allocator();
//     rclc_support_init(&support, 0, NULL, &allocator);
//     rclc_node_init_default(&node, "esp32_robot_node", "", &support);

//     rosidl_runtime_c__String__assign(&msg_odom.header.frame_id, "odom");
//     rosidl_runtime_c__String__assign(&msg_odom.child_frame_id, "base_link");

//     // Init Topics
//     rclc_subscription_init_default(&subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "cmd_vel");
//     rclc_publisher_init_default(&odom_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(nav_msgs, msg, Odometry), "odom");
//     rclc_publisher_init_default(&rpm_l_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "real_rpm_l");
//     rclc_publisher_init_default(&rpm_r_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "real_rpm_r");

//     rclc_executor_init(&executor, &support.context, 1, &allocator);
//     rclc_executor_add_subscription(&executor, &subscriber, &msg_cmd_vel, &cmd_vel_callback, ON_NEW_DATA);
// }

// void loop() {
//     rclc_executor_spin_some(&executor, RCL_MS_TO_NS(1));

//     static unsigned long lastOdom = 0;
//     if (millis() - lastOdom > 50) {
//         float dt = (millis() - lastOdom) / 1000.0; // เวลาที่ผ่านไปเป็นวินาที

//         long cL = encoderL;
//         long cR = encoderR;
//         long deltaL = cL - lastEncL;
//         long deltaR = cR - lastEncR;

//         // 1. คำนวณ RPM (ΔTicks / (PPR * 2) * 60 / dt)
//         msg_rpm_l.data = ((float)deltaL / (ENCODER_PPR * 2.0)) * (60.0 / dt);
//         msg_rpm_r.data = ((float)deltaR / (ENCODER_PPR * 2.0)) * (60.0 / dt);

//         // 2. คำนวณ Odom
//         float dL = deltaL * METER_PER_TICK;
//         float dR = deltaR * METER_PER_TICK;
//         lastEncL = cL; lastEncR = cR;

//         float dS = (dL + dR) / 2.0;
//         float dT = (dR - dL) / TRACK_WIDTH;

//         pose_x += dS * cos(pose_theta);
//         pose_y += dS * sin(pose_theta);
//         pose_theta += dT;

//         // Publish Odom
//         msg_odom.header.stamp.sec = millis() / 1000;
//         msg_odom.header.stamp.nanosec = (millis() % 1000) * 1000000;
//         msg_odom.pose.pose.position.x = pose_x;
//         msg_odom.pose.pose.position.y = pose_y;
//         msg_odom.pose.pose.orientation.z = sin(pose_theta / 2.0);
//         msg_odom.pose.pose.orientation.w = cos(pose_theta / 2.0);

//         rcl_publish(&odom_pub, &msg_odom, NULL);

//         // Publish RPM
//         rcl_publish(&rpm_l_pub, &msg_rpm_l, NULL);
//         rcl_publish(&rpm_r_pub, &msg_rpm_r, NULL);

//         lastOdom = millis();
//     }
// }


//////////////////////////////////////////////////////////NICE
#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <geometry_msgs/msg/twist.h>
#include <nav_msgs/msg/odometry.h>
#include <std_msgs/msg/float32.h>
#include <std_msgs/msg/int64.h>
#include "FastAccelStepper.h"
#include <rosidl_runtime_c/string_functions.h>

// --- Pins Config (WROOM-32 Safe Pins) ---
#define STEP_L 26
#define DIR_L  25
#define STEP_R 33
#define DIR_R  32
#define EN_PIN 27

#define ENC_L_A 14
#define ENC_L_B 13
#define ENC_R_A 34
#define ENC_R_B 35

// --- Robot Parameters ---
#define WHEEL_RADIUS 0.075      // 7.5 cm = 0.075 m
#define TRACK_WIDTH  0.59       // 59 cm = 0.59 m
#define GEAR_RATIO   10.0       // 1:10 (ใช้เฉพาะฝั่ง Stepper)
#define STEP_MICRO   4.0        // 800 steps/rev
#define ENCODER_PPR  1000.0     // Pulse Per Revolution
#define TICKS_PER_REV_STEP (200.0 * STEP_MICRO * GEAR_RATIO)

// --- Constants Calculation ---
const float METER_TO_STEPS = TICKS_PER_REV_STEP / (2.0 * PI * WHEEL_RADIUS);
const float RAD_TO_STEPS   = (METER_TO_STEPS * TRACK_WIDTH / 2.0);

// Encoder อยู่ที่แกนล้อ: หมุน 1 รอบได้ระยะทาง 2*PI*R
// นับแบบ CHANGE (2 Ticks/Pulse) = 2000 Ticks/Round
const float METER_PER_TICK = (2.0 * PI * WHEEL_RADIUS) / (ENCODER_PPR * 2.0);

// --- Global Variables ---
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *lstepper = NULL;
FastAccelStepper *rstepper = NULL;

volatile long encoderL = 0, encoderR = 0;
long lastEncL = 0, lastEncR = 0;
double pose_x = 0, pose_y = 0, pose_theta = 0;

// micro-ROS Objects
rcl_subscription_t subscriber;
geometry_msgs__msg__Twist msg_cmd_vel;
rcl_publisher_t odom_pub;
nav_msgs__msg__Odometry msg_odom;

rcl_publisher_t rpm_l_pub, rpm_r_pub;
rcl_publisher_t tick_l_pub, tick_r_pub;
std_msgs__msg__Float32 msg_rpm_l, msg_rpm_r;
std_msgs__msg__Int64 msg_tick_l, msg_tick_r;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

// --- ISR (Encoder) ---
void IRAM_ATTR ISR_L() { (digitalRead(ENC_L_B)) ? encoderL++ : encoderL--; }
void IRAM_ATTR ISR_R() { (digitalRead(ENC_R_B)) ? encoderR-- : encoderR++; }

// --- ROS2 Callback ---
void cmd_vel_callback(const void * msgin) {
    const geometry_msgs__msg__Twist * msg = (const geometry_msgs__msg__Twist *)msgin;

    // ความเร็ว x2 ตามต้องการ
    float vx = msg->linear.x * 2.0;
    float wz = msg->angular.z * 2.0;

    int32_t l_speed = (int32_t)(vx * METER_TO_STEPS - wz * RAD_TO_STEPS);
    int32_t r_speed = (int32_t)(vx * METER_TO_STEPS + wz * RAD_TO_STEPS);

    if (l_speed == 0) lstepper->stopMove();
    else {
        lstepper->setSpeedInHz(abs(l_speed));
        (l_speed > 0) ? lstepper->runForward() : lstepper->runBackward();
    }

    if (r_speed == 0) rstepper->stopMove();
    else {
        rstepper->setSpeedInHz(abs(r_speed));
        (r_speed > 0) ? rstepper->runBackward() : rstepper->runForward();
    }
}

void setup() {
    Serial.begin(115200);
    set_microros_serial_transports(Serial);

    // Encoder Setup
    pinMode(ENC_L_A, INPUT_PULLUP); pinMode(ENC_L_B, INPUT_PULLUP);
    pinMode(ENC_R_A, INPUT_PULLUP); pinMode(ENC_R_B, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENC_L_A), ISR_L, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_R_A), ISR_R, CHANGE);
    // Stepper Setup (No Acceleration)
    engine.init();
    lstepper = engine.stepperConnectToPin(STEP_L);
    if (lstepper) {
        lstepper->setDirectionPin(DIR_L);
        lstepper->setEnablePin(EN_PIN);
        lstepper->setAutoEnable(true);
        lstepper->setAcceleration(1000000); // High Acceleration = Instant
    }
    rstepper = engine.stepperConnectToPin(STEP_R);
    if (rstepper) {
        rstepper->setDirectionPin(DIR_R);
        rstepper->setEnablePin(EN_PIN);
        rstepper->setAutoEnable(true);
        rstepper->setAcceleration(1000000);
    }

    // Initialize micro-ROS
    while (rmw_uros_ping_agent(100, 1) != RCL_RET_OK) { delay(100); }

    allocator = rcl_get_default_allocator();
    rclc_support_init(&support, 0, NULL, &allocator);
    rclc_node_init_default(&node, "esp32_robot_node", "", &support);

    // Setup Odom Frame IDs
    rosidl_runtime_c__String__assign(&msg_odom.header.frame_id, "odom");
    rosidl_runtime_c__String__assign(&msg_odom.child_frame_id, "base_link");

    // Init Publishers & Subscribers
    rclc_subscription_init_default(&subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "cmd_vel");
    rclc_publisher_init_default(&odom_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(nav_msgs, msg, Odometry), "odom");
    rclc_publisher_init_default(&rpm_l_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "real_rpm_l");
    rclc_publisher_init_default(&rpm_r_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "real_rpm_r");
    rclc_publisher_init_default(&tick_l_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64), "enc_l_ticks");
    rclc_publisher_init_default(&tick_r_pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64), "enc_r_ticks");

    rclc_executor_init(&executor, &support.context, 1, &allocator);
    rclc_executor_add_subscription(&executor, &subscriber, &msg_cmd_vel, &cmd_vel_callback, ON_NEW_DATA);
}

void loop() {
    rclc_executor_spin_some(&executor, RCL_MS_TO_NS(1));

    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 50) { // 20Hz Update Rate
        float dt = (millis() - lastUpdate) / 1000.0;
        lastUpdate = millis();

        long cL = encoderL;
        long cR = encoderR;
        long dL_ticks = cL - lastEncL;
        long dR_ticks = cR - lastEncR;
        lastEncL = cL;
        lastEncR = cR;

        // 1. ส่งค่า Ticks ดิบ
        msg_tick_l.data = cL;
        msg_tick_r.data = cR;

        // 2. คำนวณ Real RPM (ล้อ)
        msg_rpm_l.data = ((float)dL_ticks / (ENCODER_PPR * 2.0)) * (60.0 / dt);
        msg_rpm_r.data = ((float)dR_ticks / (ENCODER_PPR * 2.0)) * (60.0 / dt);

        // 3. คำนวณ Odometry (ระยะทางหน่วยเป็น เมตร)
        float distL = (float)dL_ticks * METER_PER_TICK;
        float distR = (float)dR_ticks * METER_PER_TICK;

        float dStep = (distL + distR) / 2.0;
        float dTheta = (distR - distL) / TRACK_WIDTH;

        pose_x += dStep * cos(pose_theta);
        pose_y += dStep * sin(pose_theta);
        pose_theta += dTheta;

        // 4. บรรจุข้อมูลและ Publish
        msg_odom.header.stamp.sec = millis() / 1000;
        msg_odom.header.stamp.nanosec = (millis() % 1000) * 1000000;
        msg_odom.pose.pose.position.x = pose_x;
        msg_odom.pose.pose.position.y = pose_y;
        msg_odom.pose.pose.orientation.z = sin(pose_theta / 2.0);
        msg_odom.pose.pose.orientation.w = cos(pose_theta / 2.0);

        rcl_publish(&odom_pub, &msg_odom, NULL);
        rcl_publish(&rpm_l_pub, &msg_rpm_l, NULL);
        rcl_publish(&rpm_r_pub, &msg_rpm_r, NULL);
        rcl_publish(&tick_l_pub, &msg_tick_l, NULL);
        rcl_publish(&tick_r_pub, &msg_tick_r, NULL);
    }
}