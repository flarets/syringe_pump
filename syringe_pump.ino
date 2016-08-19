#include <LiquidCrystal.h>
#include <LCDKeypad.h>
#include <TimerOne.h>

// pin definitions 
const byte buttonPin = A0; // analogue pin 0
const byte motorDirPin = 2;
const byte motorStepPin = 3;
const byte MS1 = 4;
const byte MS2 = 5;
const byte MS3 = 6;

// stepper motor constants
long steps_per_revolution = 400;
int usteps_per_step[] = {1, 2, 4, 8, 16}; // microsteps per step
int step_table[] = {LOW,  LOW,  LOW,   // full step
                    HIGH, LOW,  LOW,   // half step 
                    LOW,  HIGH, LOW,   // quarter step
                    HIGH, HIGH, LOW,   // eighth step
                    HIGH, HIGH, HIGH}; // sixteenth step
int USTEPS_INDEX = 4; // 1/16 microsteps results in quietest and smoothest operation

float MIN_FLOW_RATE = 0.99/60.0; // min flow rate, mL/min, accounts for round-off error
float MAX_FLOW_RATE = 60.0/60.0; // max flow rate, mL/min

// syringe pump constants
float syringe_volume = 25.0;
float syringe_barrel_length = 78.0;
float threaded_rod_pitch = 1.5;

// LCD values
LiquidCrystal lcd(8, 13, 9, 4, 5, 6, 7);
float refresh_rate = 1; // refresh rate, Hz
int wholePart;
int decPart;
String s1 = "                "; // upper line
String s2 = "                "; // lower line

// Button values
enum{KEY_SELECT, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_UP, KEY_NONE};
int NUM_KEYS = 6;

long adc_key_val[6] = {614, 815, 856, 904, 931, 1023}; // button values, mV
int adc_error = 5; // button error, mV
volatile long adc_value = adc_key_val[KEY_NONE];

unsigned int last_key;
unsigned int repeat_key_delay = 200; // ms delay between key repeats
unsigned int debounce = 200; // ms for debounce
long last_time;

// stepper motor direction and speed
int dir = HIGH; // direction
float revs_per_s; // revs per second
long delay_per_step; // delay per step, us

// syringe pump values
float revs_per_mL = 1.0;//syringe_barrel_length/(threaded_rod_pitch*syringe_volume); // revs/mL

// user input values
float volume = 0; // cumulative volume, mL
float volume_increment = 1.0/revs_per_mL; //10/(steps_per_revolution*revs_per_mL); // volume increment, mL
float flow_rate; // flow rate, mL/s
float flow_rate_increment = 1/60.0; // flow rate increment, mL/s

// ---------------------------------------------------------------------------

/*
String dec_to_str(float num){
  // displays numbers in %4.2f format

  wholePart = floor(abs(num));
  decPart = round(abs(num)*100 - wholePart*100); // 2 decimal places

  if (decPart > 99) {
    wholePart += 1;
    decPart = 0;
  }
  
  if (floor(num*10) < 0) {
    if (wholePart < 10) {
      if (decPart < 10) {
        return " -" + String(wholePart) + ".0" + String(decPart);
      } else {
        return " -" + String(wholePart) + "." + String(decPart);
      }
    } else if (wholePart < 100) {
      if (decPart < 10) {
        return "-" + String(wholePart) + ".0" + String(decPart);
      } else {
        return "-" + String(wholePart) + "." + String(decPart);
      }
    }
  } else {
    if (wholePart < 10) {
      if (decPart < 10) {
        return "  " + String(wholePart) + ".0" + String(decPart);
      } else {
        return "  " + String(wholePart) + "." + String(decPart);
      }
    } else if (wholePart < 100) {
      if (decPart < 10) {
        return " " + String(wholePart) + ".0" + String(decPart);
      } else {
        return " " + String(wholePart) + "." + String(decPart);
      }
    }
  }

  return "    ";
}

void increment_values()
{
  if (dir == KEY_UP && flow_rate <= (MAX_FLOW_RATE - flow_rate_increment)) {
    flow_rate += flow_rate_increment;
  } else if (dir == KEY_DOWN && flow_rate >= (MIN_FLOW_RATE + flow_rate_increment)) {
    flow_rate -= flow_rate_increment;
  }
  revs_per_s = revs_per_mL*flow_rate; // revs/s
  //delay_per_step = 1e6/(revs_per_s * steps_per_revolution * usteps_per_step[USTEPS_INDEX]);
}
*/
void move_syringe()
{
  /*
  if (dir == HIGH){
    digitalWrite(motorDirPin, HIGH);
    //volume += volume_increment;
  } else if (dir == LOW) {
    digitalWrite(motorDirPin, LOW);
    //volume -= volume_increment;
  }
  */
  delay_per_step = (1e6/(revs_per_s * steps_per_revolution * usteps_per_step[USTEPS_INDEX]));
    
  for (long j=0; j<steps_per_revolution; j++) {
    for (int k=0; k<usteps_per_step[USTEPS_INDEX]; k++) {
      digitalWrite(motorStepPin, HIGH);
      delayMicroseconds(delay_per_step/2);
      digitalWrite(motorStepPin, LOW);
      delayMicroseconds(delay_per_step/2);
    }
  }
}
/*
void do_key_action(unsigned int key)
{
  // if the up or down button is held down, delay between actions
  if ((key == last_key) && ((key == KEY_DOWN) || (key == KEY_UP))) {
    while ((millis() - last_time) < repeat_key_delay) {}
  }
  last_time = millis();
  last_key = key;
 
  // perform action based on key
  if (key == KEY_SELECT) {
    volume = 0; // clear
    delay(debounce);
  } else if (key == KEY_RIGHT) {
    //move_syringe(KEY_RIGHT);
  } else if (key == KEY_LEFT) {
    //move_syringe(KEY_LEFT);
  } else if (key == KEY_DOWN) {
    //increment_values(KEY_DOWN);
    delay(debounce);
  } else if (key == KEY_UP) {
    //increment_values(KEY_UP);
    delay(debounce);
  }
}

void handle_keypress()
{
  // identify which key was pressed based on the latest ADC value
  adc_value = analogRead(0);
  for (long i = 0; i < NUM_KEYS; i++) {
    if (adc_value < adc_key_val[i] + adc_error) {
      do_key_action(i);
      break;
    }
  }
  s1 = "Q = " + dec_to_str(flow_rate*60) + "mL/min"; // upper line
  s2 = "V = " + dec_to_str(volume)    + "mL   "; // lower line
}
*/

void setup(void)
{
  // LCD setup
  lcd.begin(16, 2);
  lcd.setCursor(0,0);

  // button pin    
  pinMode(buttonPin, INPUT);
  
  // interrupt to refresh the LCD
  Timer1.initialize(100000); // delay in us
  Timer1.attachInterrupt(refresh);

  // stepper motor setup
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);
  digitalWrite(MS1, step_table[USTEPS_INDEX*3 + 0]);
  digitalWrite(MS2, step_table[USTEPS_INDEX*3 + 1]);
  digitalWrite(MS3, step_table[USTEPS_INDEX*3 + 2]);
  
  pinMode(motorDirPin, OUTPUT);
  pinMode(motorStepPin, OUTPUT);
  digitalWrite(motorDirPin, dir);

  flow_rate = 1.0; // mL/s
  revs_per_s = revs_per_mL*flow_rate; // revs/s
  //delay_per_step = 1e6/(revs_per_s * steps_per_revolution * usteps_per_step[USTEPS_INDEX]);
  
  Serial.begin(9600); // Note that your serial connection must be set to 57600 to work!
}

void loop(void)
{
  //handle_keypress();
  move_syringe();
  
  // change direction
  if (dir == HIGH) {
    dir = LOW;
  } else { 
    dir = HIGH;
  }
  
  lcd.setCursor(0,0);
  lcd.print(s1);
  lcd.setCursor(0,1);
  lcd.print(s2);
  delay(2000);
}
