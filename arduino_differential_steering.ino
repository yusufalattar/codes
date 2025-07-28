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

// Dead zone for better control
const int DEAD_ZONE = 50;

void setup() {
  Serial.begin(115200);
  
  // Configure motor driver pins
  pinMode(REN_L, OUTPUT); 
  pinMode(LEN_L, OUTPUT);
  pinMode(RPWM_L, OUTPUT); 
  pinMode(LPWM_L, OUTPUT);

  pinMode(REN_R, OUTPUT); 
  pinMode(LEN_R, OUTPUT);
  pinMode(RPWM_R, OUTPUT); 
  pinMode(LPWM_R, OUTPUT);

  // Enable motor drivers
  digitalWrite(REN_L, HIGH); 
  digitalWrite(LEN_L, HIGH);
  digitalWrite(REN_R, HIGH); 
  digitalWrite(LEN_R, HIGH);

  // Initialize IBus
  ibus.begin(Serial);
  
  Serial.println("Differential Steering Robot Ready!");
}

void loop() {
  // Read IBus channels (adjust channel numbers based on your transmitter)
  int chThrottle = ibus.readChannel(1);  // Throttle channel
  int chSteering = ibus.readChannel(0);  // Steering channel

  // Apply dead zone to prevent drift
  if (abs(chThrottle - 1500) < DEAD_ZONE) chThrottle = 1500;
  if (abs(chSteering - 1500) < DEAD_ZONE) chSteering = 1500;

  // Map throttle to -255 to 255 range (full range)
  int throttle = map(chThrottle, 1000, 2000, -255, 255);
  
  // Map steering to -255 to 255 range (full range)
  int steering = map(chSteering, 1000, 2000, -255, 255);

  // Apply exponential curve for better fine control
  throttle = applyExponentialCurve(throttle);
  steering = applyExponentialCurve(steering);

  // Calculate differential drive
  int leftSpeed = throttle + steering;
  int rightSpeed = throttle - steering;

  // Constrain speeds to valid range
  leftSpeed = constrain(leftSpeed, -255, 255);
  rightSpeed = constrain(rightSpeed, -255, 255);

  // Control left motor
  controlMotor(leftSpeed, RPWM_L, LPWM_L);
  
  // Control right motor (reversed for proper differential steering)
  controlMotor(rightSpeed, RPWM_R, LPWM_R);

  // Debug output (optional - remove for production)
  if (millis() % 500 < 10) {  // Print every 500ms
    Serial.print("Throttle: "); Serial.print(throttle);
    Serial.print(" Steering: "); Serial.print(steering);
    Serial.print(" Left: "); Serial.print(leftSpeed);
    Serial.print(" Right: "); Serial.println(rightSpeed);
  }

  delay(20);  // 50Hz update rate
}

// Apply exponential curve for better control feel
int applyExponentialCurve(int value) {
  if (value == 0) return 0;
  
  float normalized = abs(value) / 255.0;
  float curved = normalized * normalized;
  
  if (value > 0) {
    return (int)(curved * 255);
  } else {
    return -(int)(curved * 255);
  }
}

// Control a single motor with proper PWM
void controlMotor(int speed, int pwmPin1, int pwmPin2) {
  if (speed > 0) {
    // Forward
    analogWrite(pwmPin1, speed);
    analogWrite(pwmPin2, 0);
  } else if (speed < 0) {
    // Backward
    analogWrite(pwmPin1, 0);
    analogWrite(pwmPin2, -speed);
  } else {
    // Stop
    analogWrite(pwmPin1, 0);
    analogWrite(pwmPin2, 0);
  }
}