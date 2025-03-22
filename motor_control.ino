#include <Servo.h>


/**********************PID Control***********************/
volatile double angle = 0;    //Measured angle from feedback
volatile float thetaPre = 0;  //previous angle from feedback
volatile int turns = 0;
volatile unsigned long tHigh;
volatile unsigned long tLow;
volatile unsigned long rise;
volatile unsigned long fall;

const int dutyScale = 1;
const int unitsFC = 360;
const float dcMin = 0.029;  //From Parallax spec sheet
const float dcMax = 0.971;  //From Parallax spec sheet
const int q2min = unitsFC / 4;
const int q3max = q2min * 3;

double Kp = 0.65;  //Proportional Gain, higher values for faster response, higher values contribute to overshoot.
double Ki = .3;    //Integral Gain, higher values to converge faster to zero error, higher values produce oscillations. Higher values are more unstable near a target_angle = 0.
double Kd = 1;     //Derivative Gain, higher values dampen oscillations around target_angle. Higher values produce more holding state jitter. May need filter for error noise.


#define DEBOUNCE_DELAY 120
#define pin_feedback A0
#define pin_turn_page_left 12
#define pin_turn_page_right 13
#define pin_left_z_down A1
#define pin_left_z_up 4
#define pin_right_z_down 7
#define pin_right_z_up 8
#define pin_hor_right 2
#define pin_hor_left 3

/*********************************************/
Servo horX, zLeft, zRight, horHook, verHook;

volatile bool rightButtonPressed = false;
volatile bool leftButtonPressed = false;
volatile unsigned long lastDebounceTime = 0;
volatile unsigned long lastDebounceTimeL = 0;

double output_val, target_angle = 90;

void setup() {
  zLeft.attach(5);
  zRight.attach(6);
  horHook.attach(9);
  verHook.attach(10);
  horX.attach(11);

  horX.write(90);
  zLeft.write(90);
  zRight.write(90);
  horHook.write(90);
  verHook.write(90);

  pinMode(pin_feedback, INPUT_PULLUP);
  pinMode(pin_turn_page_left, INPUT);
  pinMode(pin_turn_page_right, INPUT);
  pinMode(pin_left_z_down, INPUT);
  pinMode(pin_left_z_up, INPUT);
  pinMode(pin_right_z_down, INPUT);
  pinMode(pin_right_z_up, INPUT);
  pinMode(pin_hor_right, INPUT);
  pinMode(pin_hor_left, INPUT);

  PCICR |= 0x07;   //Port D,C,B
  PCMSK0 |= 0x31;  //D8, D12, D13
  PCMSK1 |= 0x02;  //A1
  PCMSK2 |= 0x90;  //D4, D7

  attachInterrupt(digitalPinToInterrupt(pin_hor_right), right_button_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pin_hor_left), left_button_isr, CHANGE);
  Serial.begin(115200);
}


//one button to left page (reversed)
//one button to right page (current)
//2 buttons to move horX (done)
//4 button to move Z dir (2 motors) (done, set correct speed)

//next receive and send through MQTT on ESP8266 through UART

void right_button_isr() {
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    lastDebounceTime = millis();
    if (digitalRead(pin_hor_right) == HIGH) {
      horX.write(0);
      rightButtonPressed = true;
    } else {
      horX.write(90);
      rightButtonPressed = false;
    }
  }
}

void left_button_isr() {
  if ((millis() - lastDebounceTimeL) > DEBOUNCE_DELAY) {  // Use separate debounce variable
    lastDebounceTimeL = millis();
    if (digitalRead(pin_hor_left)  == HIGH) {
      leftButtonPressed = true;
      horX.write(180);
    } else {
      leftButtonPressed = false;
      horX.write(90);
    }
  }
}

void turnRightPage() {
  //we have the current angle of the motor in place

/*
  1.
  2.
  3.
  4.

*/

}

void turnLeftPage() {
}

//Port B
ISR(PCINT0_vect) {
  //D8 (move right z motor up)
  if (digitalRead(pin_right_z_up) == HIGH) {
    zRight.write(110);
  } else {
    zRight.write(90);
  }
  //D12 (turn left page)
  if (digitalRead(pin_turn_page_left) == HIGH) {
    turnLeftPage();
  }
  //D13 (turn right Page)
  else if (digitalRead(pin_turn_page_right) == HIGH) {
    turnRightPage();
  }
}

//Port C
ISR(PCINT1_vect) {
  //A0

  //A1 (move left z motor down)
  if (digitalRead(pin_left_z_down) == HIGH) {
    zLeft.write(70);
  } else {
    zLeft.write(90);
  }
}
//Port D
ISR(PCINT2_vect) {
  //D4 (move left z motor up)
  if (digitalRead(pin_left_z_up) == HIGH) {
    zLeft.write(110);
  } else {
    zLeft.write(90);
  }
  //D7 (move right z motor down)
  if (digitalRead(pin_right_z_down) == HIGH) {
    zRight.write(70);
  } else {
    zRight.write(90);
  }
}

double integral = 0;
double deriv = 0;
double prevErr = 0;
void setAngle(float desiredAngle, float threshold) {
  float output, offset, value;
  for (double errorAngle = desiredAngle - angle;
       abs(errorAngle) > threshold;
       errorAngle = desiredAngle - angle) {
    output = errorAngle * Kp;                 //proportional
    integral = (integral + errorAngle) * Ki;  //integral
    // integral = constrain(integral, -5, 5);
    deriv = Kd * (errorAngle - prevErr);  //deriv
    output += integral + deriv;

    if (output > 200.0)
      output = 200.0;
    if (output < -200.0)
      output = -200.0;
    if (errorAngle > 0)
      offset = 30.0;
    else if (errorAngle < 0)
      offset = -30.0;
    else
      offset = 0.0;

    prevErr = errorAngle;
    value = output + offset;
    horX.writeMicroseconds(1490 - value);
    getServoAngle();
  }

  horX.writeMicroseconds(1490);
}

void getServoAngle() {
  float dc = 0;
  float tCycle = 0;
  while (1)  //From Parallax spec sheet
  {
    tHigh = pulseIn(pin_feedback, HIGH);
    tLow = pulseIn(pin_feedback, LOW);
    tCycle = tHigh + tLow;
    if (tCycle > 1000 && tCycle < 1200) {
      break;  //valid tCycle;
    }
  }


  dc = (dutyScale * tHigh) / tCycle;

  float theta = ((dc - dcMin) * unitsFC) / (dcMax - dcMin);

  if (theta < 0.0)
    theta = 0.0;
  else if (theta > (unitsFC - 1.0))
    theta = unitsFC - 1.0;

  if ((theta < q2min) && (thetaPre > q3max))
    turns++;
  else if ((thetaPre < q2min) && (theta > q3max))
    turns--;

  if (turns >= 0)
    angle = (turns * unitsFC) + theta;
  else if (turns < 0)
    angle = ((turns + 1) * unitsFC) - (unitsFC - theta);


  thetaPre = theta;
}

int SERVO_VAL = 93;
void loop() {

  // getServoAngle();

  if (Serial.available() > 0) {
    int receivedNumber = Serial.parseInt();
    // setAngle(receivedNumber, 2);
  }

  // Serial.print("Turns: ");
  // Serial.print(turns);
  // Serial.print(" Angle: ");
  // Serial.println(angle);
  delay(100);
}
