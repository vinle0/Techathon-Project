#include <Servo.h>
#include <Wire.h>
/*
Buttons configuration

EFGH (top breadboard, esp closest to board)
---- 
ABCD (bottom breadboard, closest to the board)



A = D12 (turn page left)
B = D13 (turn page right)
C = D3 (hor left)
D = D2 (hor right)

E = D7 (right z down)
F = D8 (right z up)
G = A1 (left z down)
H = D4 (left z up)

PWM Configuration

AB
----
CDE

A = D5 (zLeft)
B = D6 (zRight)
C = D9 (horHook)
D = D10 (verHook)
E = D11 (horX)




*/

//next receive and send through MQTT on ESP8266 through UART
//18 is RX
//19 is TX

//SoftwareSerial library has the following known limitations:
//It cannot transmit and receive data at the same time.
//On Arduino or Genuino 101 boards the current maximum RX speed is 57600bps.
//On Arduino or Genuino 101 boards RX doesn't work on digital pin 13.

//one button to left page (reversed)
//one button to right page (current)
//2 buttons to move horX (done)
//4 button to move Z dir (2 motors) (done, set correct speed)

/*IOT
Arduino subscribes to eid/w2b, taking in commands for 
  --turning page (reqPrev and reqNext)

Web subscribes to eid/b2w/cmd for
  --turning page (prev and next) to indicate that a page has turned

*/

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
#define pin_rx_esp 18
#define pin_tx_esp 19
#define UP 0
#define DOWN 180
#define ver_hook_rest 50  //angle
#define I2c_slave_addr 0x24
#define buffer 32
/*********************************************/
Servo horX, zLeft, zRight, horHook, verHook;

volatile bool rightButtonPressed = false;
volatile bool leftButtonPressed = false;
volatile unsigned long lastDebounceTime = 0;
volatile unsigned long lastDebounceTimeL = 0;


double getServoAngle();
void setAngle(float desiredAngle, float threshold);

double output_val, target_angle = 90;

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
double Kd = 1.01;  //Derivative Gain, higher values dampen oscillations around target_angle. Higher values produce more holding state jitter. May need filter for error noise.


void setup() {
  zLeft.attach(5);     // will move
  zRight.attach(6);    //will move
  horHook.attach(9);   //static
  verHook.attach(10);  //will move
  horX.attach(11);     //static

  horX.write(90);
  zLeft.write(90);
  zRight.write(90);
  horHook.write(90);
  verHook.write(ver_hook_rest);

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

  Wire.begin(I2c_slave_addr);

  Serial.begin(115200);
}


//right
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
//left
void left_button_isr() {
  if ((millis() - lastDebounceTimeL) > DEBOUNCE_DELAY) {  // Use separate debounce variable
    lastDebounceTimeL = millis();
    if (digitalRead(pin_hor_left) == HIGH) {
      leftButtonPressed = true;
      horX.write(180);
    } else {
      leftButtonPressed = false;
      horX.write(90);
    }
  }
}

volatile bool turnRight = false;
void turnRightPage() {
  turnRight = true;
}
volatile bool turnLeft = false;
void turnLeftPage() {
  turnLeft = true;
}
volatile bool pin_change_d8 = false;
//Port B
ISR(PCINT0_vect) {

  //D12 (turn left page)
  if (digitalRead(pin_turn_page_left) == HIGH) {
    turnLeftPage();
  }
  //D13 (turn right Page)
  else if (digitalRead(pin_turn_page_right) == HIGH) {
    turnRightPage();
  }
    //D8 (move right z motor up)
  else if (digitalRead(pin_right_z_up) == HIGH) {
    pin_change_d8 = true;
    zRight.write(UP);
  } else if (digitalRead(pin_right_z_up) == LOW && pin_change_d8 == true){
    zRight.write(90);
    pin_change_d8=false;
  }
}
//Port C
ISR(PCINT1_vect) {
  //A0

  //A1 (move left z motor down)
  if (digitalRead(pin_left_z_down) == HIGH) {
    zLeft.write(DOWN);
  } else {
    zLeft.write(90);
  }
}
//Port D
ISR(PCINT2_vect) {
  //D4 (move left z motor up)
  if (digitalRead(pin_left_z_up) == HIGH) {
    zLeft.write(UP);
  } else if(digitalRead(pin_left_z_up) == LOW){
    zLeft.write(90);
  }
  //D7 (move right z motor down)
  if (digitalRead(pin_right_z_down) == HIGH) {
    zRight.write(DOWN);
  } else if(digitalRead(pin_right_z_down) == LOW){
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
    horHook.writeMicroseconds(1490 - value);
    getServoAngle();

  }

  horHook.writeMicroseconds(1490);
}

//NOTE needs to be connected to an active feedback pin else it fails
double getServoAngle() {
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
  return angle;
}
unsigned long previousMillis = 0UL;
//in ms
void delay_func(unsigned long interval) {

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;
  while (currentMillis - previousMillis < interval) { currentMillis = millis(); }
  previousMillis = currentMillis;

}

const double middle_angle = getServoAngle();

void receive_ESP_command(){
  String str = "";
  // Serial.println("Receiving");
  Wire.requestFrom(I2c_slave_addr, buffer);
  while (Wire.available()) { // peripheral may send less than requested
    char c = Wire.read(); // receive a byte as character
    str += c;
  }
  //process received ESP command
  if(str == "reqNext"){
    turnRightPage();
  }
  else if (str == "reqPrev"){
    turnLeftPage();
  }
  //else it was a dummy variable
  else
    Serial.println(str); //debug

}

//send to ESP
void send_ESP_command(const char* str){
  //send the command
  Serial.print("Sending ");
  Serial.println(str);
  Wire.beginTransmission(I2c_slave_addr);
  Wire.write(str);
  Wire.endTransmission();
}

#define z_interval 3000
#define horX_interval 7000
#define horX_offset 500  //offset for the middle of page (angle)
#define end_angle 12000

//recall that middle_angle is between 0 and 360, thus add 360 
double end_angle_l = -1*end_angle + middle_angle - 360;
double end_angle_r = end_angle + middle_angle + 360;
void loop() {

  getServoAngle();

  if (Serial.available() > 0) {
    Serial.println(Serial.readString());
    // int receivedNumber = Serial.parseInt();
    // setAngle(receivedNumber, 7);
  }

  if (turnRight) {

    // Serial.println("start turning right");
    // zRight.write(DOWN);
    // zLeft.write(UP);
    // delay_func(z_interval);
    // zRight.write(90);
    // zLeft.write(90);
    // Serial.println("finish moving z right down and z left up");

    
    //move horX to middle of page, now page is lifted up, it is moving left.
    // Serial.println("horx moving to middle of page");
    // horX.write(DOWN);
    // delay_func(horX_interval);
    // horX.write(90);
    // Serial.println("horx finish moving to middle of page");

    // Serial.println("move ver hook");
    // // Serial.println(middle_angle);
    // //move horHook a little
    // setAngle(middle_angle + horX_offset, 5);  
    // //insert verHook
    // verHook.write(130);
    // Serial.println("finish ver hook");

    // delay_func(100);

    // //release right z hook, now both handles are released
    // Serial.println("release z right");
    // zRight.write(UP);
    // delay_func(2 * z_interval);  //double interval for moving upward to displace the pressed down
    // zRight.write(90);
    // Serial.println("finish release z right");

    // //move horHook all the way to the left page
    // //move CCW (?)
    // Serial.println("Move hor hook to the left");
    // setAngle(end_angle_l, 20);  // set angle correctly (experimental)
    // verHook.write(ver_hook_rest);
    // Serial.println("Finish hor hook to the left");
    // //reset horHook back to middle
    // setAngle(middle_angle, 20);



    // //reset horX
    // Serial.println("horx moving back");
    // horX.write(UP);
    // delay_func(1000);
    // horX.write(90);
    // Serial.println("horx finish moving back");

    // //set down the right and left hook
    Serial.println("move down z");
    zRight.write(DOWN);
    zLeft.write(DOWN);
    delay_func(z_interval);
    zRight.write(90);
    zLeft.write(90);
    Serial.println("finish moving down z");

    send_ESP_command("next");
    turnRight = false;

  }
  if (turnLeft) {
    send_ESP_command("prev");
    turnLeft = false;
  }

  // Serial.print("Angle: ");
  // Serial.println(angle);
  // delay_func(10000);

  receive_ESP_command();
}
