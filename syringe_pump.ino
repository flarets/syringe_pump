#include <LiquidCrystal.h>
#include <LCDKeypad.h>
#include <TimerOne.h>

// pin definitions 
const byte buttonPin = A0; // analogue pin 0
const byte motorDirPin = 2;
const byte motorStepPin = 3;
const byte MS1 = 11;
const byte MS2 = 12;
const byte MS3 = 13;

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
float syringe_volume = 60.0; // mL
float syringe_barrel_length = 109.0; // mm
float threaded_rod_pitch = 8.0; // mm/rev

// LCD values
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int wholePart;
int decPart;
String s1 = " syringe_pump   "; // upper line
String s2 = " 22 aug 2016    "; // lower line

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
float revs_per_mL = syringe_barrel_length/(threaded_rod_pitch*syringe_volume); // revs/mL
unsigned int NSTEPS = 10;

// user input values
float volume = 0; // cumulative volume, mL
float volume_increment = NSTEPS/(revs_per_mL*steps_per_revolution); // volume increment, mL
float flow_rate; // flow rate, mL/s
float flow_rate_increment = 1/60.0; // flow rate increment, mL/s

// ---------------------------------------------------------------------------

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

void increment_values(int key_press)
{
  if (key_press == KEY_UP && flow_rate <= (MAX_FLOW_RATE - flow_rate_increment)) {
    flow_rate += flow_rate_increment;
  } else if (key_press == KEY_DOWN && flow_rate >= (MIN_FLOW_RATE + flow_rate_increment)) {
    flow_rate -= flow_rate_increment;
  }
  revs_per_s = revs_per_mL*flow_rate; // revs/s
  delay_per_step = 0.8e6/(revs_per_s * steps_per_revolution * usteps_per_step[USTEPS_INDEX]); // us/step
}

void move_syringe(int dir)
{
  digitalWrite(motorDirPin, dir);
  
  for (long j=0; j<NSTEPS; j++) {
    //if (dir == HIGH) volume += volume_increment;
    //else volume -= volume_increment;
    for (int k=0; k<usteps_per_step[USTEPS_INDEX]; k++) {
      digitalWrite(motorStepPin, HIGH);
      delayMicroseconds(delay_per_step/2);
      digitalWrite(motorStepPin, LOW);
      delayMicroseconds(delay_per_step/2);
    }
  }
}

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
    move_syringe(HIGH);
    volume += volume_increment;
  } else if (key == KEY_LEFT) {
    move_syringe(LOW);
    volume -= volume_increment;
  } else if (key == KEY_DOWN) {
    increment_values(key);
    delay(debounce);
  } else if (key == KEY_UP) {
    increment_values(key);
    delay(debounce);
  }
  s1 = "Q = " + dec_to_str(flow_rate*60) + "mL/min"; // upper line
  s2 = "V = " + dec_to_str(volume)    + "mL   "; // lower line
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
}

void refresh(void)
{
  lcd.setCursor(0,0);
  lcd.print(s1);
  lcd.setCursor(0,1);
  lcd.print(s2);
}

void setup(void)
{
  // LCD setup
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print(s1);
  lcd.setCursor(0,1);
  lcd.print(s2);
  delay(1000);
  
  // button pin    
  pinMode(buttonPin, INPUT);
  
  // interrupt to refresh the LCD
  long timer_delay = 100e3; // delay in us
  Timer1.initialize(timer_delay);
  Timer1.attachInterrupt(refresh,timer_delay);
  
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
  
  // set initial variables
  flow_rate = 0.5; // mL/s
  revs_per_s = revs_per_mL*flow_rate; // revs/s
  delay_per_step = 0.8e6/(revs_per_s * steps_per_revolution * usteps_per_step[USTEPS_INDEX]); // us/step
}

void loop(void)
{
  handle_keypress();
}
