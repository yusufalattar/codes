#include <IBusBM.h>
IBusBM ibus;

// Left Motor Driver Pins
const int REN_L = 2;
const int LEN_L = 3;
const int RPWM_L = 4;
const int LPWM_L = 5;

// Right Motor Driver Pins
const int REN_R = 6;
const int LEN_R = 7;
const int RPWM_R = 8;
const int LPWM_R = 9;

// Deadzone and scaling constants
const int DEADZONE = 50;        // Deadzone around center (1500)
const int MAX_PWM = 255;        // Maximum PWM value
const float TURN_SCALING = 0.7; // Reduce turn sensitivity for better control

void setup() {
  // Initialize motor pins
  pinMode(REN_L, OUTPUT); pinMode(LEN_L, OUTPUT);
  pinMode(RPWM_L, OUTPUT); pinMode(LPWM_L, OUTPUT);
  pinMode(REN_R, OUTPUT); pinMode(LEN_R, OUTPUT);
  pinMode(RPWM_R, OUTPUT); pinMode(LPWM_R, OUTPUT);

  // Enable motor drivers
  digitalWrite(REN_L, HIGH); digitalWrite(LEN_L, HIGH);
  digitalWrite(REN_R, HIGH); digitalWrite(LEN_R, HIGH);

  // Initialize serial for debugging (optional)
  Serial.begin(115200);
  
  // Initialize iBUS
  ibus.begin(Serial1); // Use Serial1 if available, or Serial if only one UART
  
  // Stop all motors initially
  stopMotors();
}

void loop() {
  // Read channels (assuming standard iBUS channel mapping)
  int chThrottle = ibus.readChannel(1);  // CH2 (throttle - forward/backward)
  int chSteering = ibus.readChannel(0);  // CH1 (steering - left/right)
  
  // Check for valid iBUS signal
  if (chThrottle == 0 || chSteering == 0) {
    stopMotors();
    return;
  }
  
  // Apply deadzone and normalize to -500 to 500 range first
  int throttleRaw = chThrottle - 1500;
  int steeringRaw = chSteering - 1500;
  
  // Apply deadzone
  if (abs(throttleRaw) < DEADZONE) throttleRaw = 0;
  if (abs(steeringRaw) < DEADZONE) steeringRaw = 0;
  
  // Scale to -255 to 255 range
  int speed = map(constrain(throttleRaw, -500, 500), -500, 500, -MAX_PWM, MAX_PWM);
  int turn = map(constrain(steeringRaw, -500, 500), -500, 500, -MAX_PWM, MAX_PWM);
  
  // Apply turn scaling to reduce sensitivity
  turn = turn * TURN_SCALING;
  
  // Calculate differential drive
  int leftMotor, rightMotor;
  
  if (abs(speed) > abs(turn)) {
    // Forward/backward dominant - preserve max speed
    leftMotor = speed + turn;
    rightMotor = speed - turn;
    
    // Scale down if either motor exceeds limits
    int maxMotor = max(abs(leftMotor), abs(rightMotor));
    if (maxMotor > MAX_PWM) {
      float scale = (float)MAX_PWM / maxMotor;
      leftMotor *= scale;
      rightMotor *= scale;
    }
  } else {
    // Turn dominant - allow pivot turning
    leftMotor = speed + turn;
    rightMotor = speed - turn;
  }
  
  // Final constraint
  leftMotor = constrain(leftMotor, -MAX_PWM, MAX_PWM);
  rightMotor = constrain(rightMotor, -MAX_PWM, MAX_PWM);
  
  // Control motors
  controlMotor(leftMotor, RPWM_L, LPWM_L);
  controlMotor(rightMotor, RPWM_R, LPWM_R);
  
  // Optional: Debug output
  /*
  Serial.print("Throttle: "); Serial.print(chThrottle);
  Serial.print(" Steering: "); Serial.print(chSteering);
  Serial.print(" Speed: "); Serial.print(speed);
  Serial.print(" Turn: "); Serial.print(turn);
  Serial.print(" Left: "); Serial.print(leftMotor);
  Serial.print(" Right: "); Serial.println(rightMotor);
  */
  
  delay(20); // Increased delay for stability
}

void controlMotor(int motorSpeed, int forwardPin, int backwardPin) {
  if (motorSpeed > 0) {
    analogWrite(forwardPin, motorSpeed);
    analogWrite(backwardPin, 0);
  } else if (motorSpeed < 0) {
    analogWrite(forwardPin, 0);
    analogWrite(backwardPin, -motorSpeed);
  } else {
    analogWrite(forwardPin, 0);
    analogWrite(backwardPin, 0);
  }
}

void stopMotors() {
  analogWrite(RPWM_L, 0); analogWrite(LPWM_L, 0);
  analogWrite(RPWM_R, 0); analogWrite(LPWM_R, 0);
}