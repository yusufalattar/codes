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

// Dead zone for stick centering
const int DEADZONE = 50;

// Maximum PWM value (can be adjusted for speed limiting)
const int MAX_SPEED = 255;

void setup() {
  // Initialize serial for debugging (optional)
  Serial.begin(115200);
  
  // Set all motor pins as outputs
  pinMode(REN_L, OUTPUT); 
  pinMode(LEN_L, OUTPUT);
  pinMode(RPWM_L, OUTPUT); 
  pinMode(LPWM_L, OUTPUT);

  pinMode(REN_R, OUTPUT); 
  pinMode(LEN_R, OUTPUT);
  pinMode(RPWM_R, OUTPUT); 
  pinMode(LPWM_R, OUTPUT);

  // Enable both motor drivers
  digitalWrite(REN_L, HIGH); 
  digitalWrite(LEN_L, HIGH);
  digitalWrite(REN_R, HIGH); 
  digitalWrite(LEN_R, HIGH);

  // Initialize iBus
  ibus.begin(Serial);
  
  // Brief delay to ensure everything is initialized
  delay(100);
}

void loop() {
  // Read channels (adjust channel numbers if needed)
  int chThrottle = ibus.readChannel(1);  // CH2 (throttle - forward/backward)
  int chSteering = ibus.readChannel(0);  // CH1 (steering - left/right)

  // Check if we're receiving valid iBus data
  if (chThrottle < 1000 || chThrottle > 2000 || chSteering < 1000 || chSteering > 2000) {
    // Stop motors if no valid signal
    stopMotors();
    delay(20);
    return;
  }

  // Convert to -500 to +500 range first for better precision
  int throttle = chThrottle - 1500;  // -500 to +500
  int steering = chSteering - 1500;  // -500 to +500

  // Apply deadzone
  if (abs(throttle) < DEADZONE) throttle = 0;
  if (abs(steering) < DEADZONE) steering = 0;

  // Scale to motor range (-255 to +255)
  int speed = map(throttle, -500, 500, -MAX_SPEED, MAX_SPEED);
  int turn = map(steering, -500, 500, -MAX_SPEED, MAX_SPEED);

  // Calculate differential steering
  // When going straight forward/backward, both motors should get the same speed
  // When turning, one motor speeds up, the other slows down
  int leftSpeed = speed + (turn / 2);   // Reduce turn influence
  int rightSpeed = speed - (turn / 2);  // Reduce turn influence

  // Constrain to valid PWM range
  leftSpeed = constrain(leftSpeed, -MAX_SPEED, MAX_SPEED);
  rightSpeed = constrain(rightSpeed, -MAX_SPEED, MAX_SPEED);

  // Control left motor
  controlMotor(leftSpeed, RPWM_L, LPWM_L);
  
  // Control right motor
  controlMotor(rightSpeed, RPWM_R, LPWM_R);

  // Optional: Print debug info
  /*
  Serial.print("Throttle: "); Serial.print(chThrottle);
  Serial.print(" Steering: "); Serial.print(chSteering);
  Serial.print(" Left: "); Serial.print(leftSpeed);
  Serial.print(" Right: "); Serial.println(rightSpeed);
  */

  delay(20);  // 50Hz update rate
}

void controlMotor(int speed, int forwardPin, int backwardPin) {
  if (speed > 0) {
    // Forward direction
    analogWrite(forwardPin, speed);
    analogWrite(backwardPin, 0);
  } else if (speed < 0) {
    // Backward direction
    analogWrite(forwardPin, 0);
    analogWrite(backwardPin, -speed);
  } else {
    // Stop
    analogWrite(forwardPin, 0);
    analogWrite(backwardPin, 0);
  }
}

void stopMotors() {
  analogWrite(RPWM_L, 0);
  analogWrite(LPWM_L, 0);
  analogWrite(RPWM_R, 0);
  analogWrite(LPWM_R, 0);
}