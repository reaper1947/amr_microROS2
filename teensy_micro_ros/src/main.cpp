// #include <Arduino.h>
// #include <micro_ros_platformio.h>
// #include <AccelStepper.h>

// #include <rcl/rcl.h>
// #include <rclc/rclc.h>
// #include <rclc/executor.h>

// #include <geometry_msgs/msg/twist.h>
// #include <nav_msgs/msg/odometry.h>
// #include <rmw_microros/rmw_microros.h>

// // ================= ROBOT CONFIG =================
// const float wheelRadius = 0.075;      // meters
// const float trackWidth  = 0.59;       // meters
// const int   ENCODER_PPR = 1000;       // pulses per revolution
// const int   QUAD_FACTOR = 4;          // x4 decoding

// const float wheelCircumference = 2.0 * PI * wheelRadius;
// const float meterPerTick = wheelCircumference / (ENCODER_PPR * QUAD_FACTOR);

// // ================= ENCODER PINS =================
// #define ENC_L_A 2
// #define ENC_L_B 4
// #define ENC_R_A 3
// #define ENC_R_B 5

// volatile long encoderL = 0;
// volatile long encoderR = 0;

// long lastEncL = 0;
// long lastEncR = 0;

// // ================= STEPPER =================
// AccelStepper LeftWheel(AccelStepper::DRIVER, 8, 9);
// AccelStepper RightWheel(AccelStepper::DRIVER, 10, 11);

// // ================= micro-ROS =================
// rcl_node_t node;
// rclc_support_t support;
// rcl_allocator_t allocator;
// rcl_subscription_t subscriber;
// rcl_publisher_t odom_publisher;
// rclc_executor_t executor;

// geometry_msgs__msg__Twist msg_cmd_vel;
// nav_msgs__msg__Odometry odom_msg;

// #define RCCHECK(fn) { rcl_ret_t rc = fn; if(rc != RCL_RET_OK){ while(1){ digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN)); delay(100);} } }
// #define RCSOFTCHECK(fn) { rcl_ret_t rc = fn; (void)rc; }

// // ================= ODOM =================
// double x = 0.0, y = 0.0, th = 0.0;
// unsigned long last_odom_time = 0;

// // frame buffers
// char frame_id[] = "odom";
// char child_frame_id[] = "base_link";

// // ================= ENCODER ISR =================
// void handleLeftA() {
//   if (digitalRead(ENC_L_A) == digitalRead(ENC_L_B))
//     encoderL++;
//   else
//     encoderL--;
// }

// void handleRightA() {
//   if (digitalRead(ENC_R_A) == digitalRead(ENC_R_B))
//     encoderR++;
//   else
//     encoderR--;
// }

// // ================= cmd_vel =================
// void cmd_vel_callback(const void * msgin)
// {
//   const geometry_msgs__msg__Twist * msg =
//     (const geometry_msgs__msg__Twist *)msgin;

//   float v = msg->linear.x;
//   float w = msg->angular.z;

//   float vLeft  = v - (w * trackWidth / 2.0);
//   float vRight = v + (w * trackWidth / 2.0);

//   float stepPerMeter = 3200.0 / wheelCircumference;

//   LeftWheel.setSpeed(vLeft * stepPerMeter);
//   RightWheel.setSpeed(-vRight * stepPerMeter);
// }

// // ================= ODOM FROM ENCODER =================
// void update_odometry()
// {
//   long currL = encoderL;
//   long currR = encoderR;

//   long deltaL = currL - lastEncL;
//   long deltaR = currR - lastEncR;

//   lastEncL = currL;
//   lastEncR = currR;

//   double dL = deltaL * meterPerTick;
//   double dR = deltaR * meterPerTick;

//   double distance = (dL + dR) / 2.0;
//   double dTh = (dR - dL) / trackWidth;

//   x += distance * cos(th + dTh/2.0);
//   y += distance * sin(th + dTh/2.0);
//   th += dTh;

//   int64_t now = rmw_uros_epoch_millis();

//   odom_msg.header.stamp.sec = now / 1000;
//   odom_msg.header.stamp.nanosec = (now % 1000) * 1000000;

//   odom_msg.pose.pose.position.x = x;
//   odom_msg.pose.pose.position.y = y;
//   odom_msg.pose.pose.position.z = 0.0;

//   odom_msg.pose.pose.orientation.x = 0.0;
//   odom_msg.pose.pose.orientation.y = 0.0;
//   odom_msg.pose.pose.orientation.z = sin(th/2.0);
//   odom_msg.pose.pose.orientation.w = cos(th/2.0);

//   // optional: velocity estimate
//   double dt = 0.05;  // 50ms
//   odom_msg.twist.twist.linear.x  = distance / dt;
//   odom_msg.twist.twist.angular.z = dTh / dt;
// }

// // ================= SETUP =================
// void setup()
// {
//   pinMode(LED_BUILTIN, OUTPUT);

//   Serial.begin(115200);
//   delay(2000);
//   set_microros_serial_transports(Serial);

//   // encoder
//   pinMode(ENC_L_A, INPUT_PULLUP);
//   pinMode(ENC_L_B, INPUT_PULLUP);
//   pinMode(ENC_R_A, INPUT_PULLUP);
//   pinMode(ENC_R_B, INPUT_PULLUP);

//   attachInterrupt(digitalPinToInterrupt(ENC_L_A), handleLeftA, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(ENC_R_A), handleRightA, CHANGE);

//   LeftWheel.setMaxSpeed(50000);
//   RightWheel.setMaxSpeed(50000);

//   allocator = rcl_get_default_allocator();
//   RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
//   RCCHECK(rclc_node_init_default(&node, "encoder_odom_node", "", &support));

//   // frame id
//   odom_msg.header.frame_id.data = frame_id;
//   odom_msg.header.frame_id.size = strlen(frame_id);
//   odom_msg.header.frame_id.capacity = sizeof(frame_id);

//   odom_msg.child_frame_id.data = child_frame_id;
//   odom_msg.child_frame_id.size = strlen(child_frame_id);
//   odom_msg.child_frame_id.capacity = sizeof(child_frame_id);

//   RCCHECK(rclc_subscription_init_default(
//     &subscriber,
//     &node,
//     ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
//     "cmd_vel"));

//   RCCHECK(rclc_publisher_init_default(
//     &odom_publisher,
//     &node,
//     ROSIDL_GET_MSG_TYPE_SUPPORT(nav_msgs, msg, Odometry),
//     "odom"));

//   RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));

//   RCCHECK(rclc_executor_add_subscription(
//     &executor,
//     &subscriber,
//     &msg_cmd_vel,
//     &cmd_vel_callback,
//     ON_NEW_DATA));

//   rmw_uros_sync_session(1000);
// }

// // ================= LOOP =================
// void loop()
// {
//   rclc_executor_spin_some(&executor, RCL_MS_TO_NS(1));

//   LeftWheel.runSpeed();
//   RightWheel.runSpeed();

//   if (millis() - last_odom_time >= 50) {
//     update_odometry();
//     RCSOFTCHECK(rcl_publish(&odom_publisher, &odom_msg, NULL));
//     last_odom_time = millis();
//   }
// }
/////////////////////////////
#include <AccelStepper.h>

// ===== Pin config =====
#define L_STEP 8
#define L_DIR  9
#define R_STEP 10
#define R_DIR  11
#define EN_PIN 7   // Enable pin

// ===== Create stepper objects =====
AccelStepper stepperL(AccelStepper::DRIVER, L_STEP, L_DIR);
AccelStepper stepperR(AccelStepper::DRIVER, R_STEP, R_DIR);

void setup() {
  Serial.begin(115200);

  // Enable driver
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);   // Active LOW (ถ้าไม่หมุนลอง HIGH)

  stepperL.setMaxSpeed(-2000);
  stepperR.setMaxSpeed(-2000);
  stepperL.setSpeed(1000);
  stepperR.setSpeed(1000);

  Serial.println("Manual motor test starting...");
}

void loop() {

  Serial.println("Forward");
  stepperL.run();
  stepperR.run();



}
