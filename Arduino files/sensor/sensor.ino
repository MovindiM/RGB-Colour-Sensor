#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
int inp_led[] = {11, 12, 13};
int value = A0;//LDR input take analog value


String calibColor[] = {"Black", "White"};
String color[]={"Red","Green","Blue"};
int calibrater[3][2] = {{363, 892}, {356, 839}, {326, 833}};
int outvalue[] = {0, 0, 0};
int out_led[] = {A1, A2, A3};
String mode;
int out;
String key;

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {2, 3, 4, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 7, 8, 9}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );



void setup() {
  lcd.init();
  lcd.backlight();

  for (int i = 0; i < 3; i++) {
    pinMode(inp_led[i], OUTPUT);
  }

  pinMode(value, INPUT);

  for (int i = 0; i < 3; i++) {
    pinMode(out_led[i], OUTPUT);
  }

  Serial.begin(9600);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WELCOME :)");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press");
  lcd.setCursor(0, 1);
  lcd.print("C to calibrate");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press");
  lcd.setCursor(0, 1);
  lcd.print("D to sensor");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press A to");
  lcd.setCursor(0, 1);
  lcd.print("use keypad value");

}


void loop() {
  mode = keypad.waitForKey();
  if (mode == "C") {
    calibrate();
  }
  else if (mode == "D") {
    sensor();
    screenout();
    ledout();
  }
  else if (mode == "A") {
    pad();
    ledout();

  }
  else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalidinput");
    delay(500);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press C to");
  lcd.setCursor(0, 1);
  lcd.print("calibrate again");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Else Press D");
  lcd.setCursor(0, 1);
  lcd.print("to sensor");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press A to");
  lcd.setCursor(0, 1);
  lcd.print("use keypad value");

}

void calibrate() {
  char key = keypad.getKey();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CALIBRATION");
  delay(500);
  for (int j = 0; j < 2; j++) {
    lcd.setCursor(0, 0);
    lcd.print("Show" + String(calibColor[j]) + "color" );
    lcd.setCursor(0, 1);
    lcd.print("press any key to continue");
    key = keypad.waitForKey();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CALIBRATING.....");
    for (int i = 0; i < 3; i++) {
      digitalWrite(inp_led[i], HIGH);
      delay(500);
      calibrater[i][j] = analogRead(value);
      delay(500);
      digitalWrite(inp_led[i],LOW);
      Serial.print(String(color[i]));
      Serial.print("=");
      Serial.print(calibrater[i][0]);
      Serial.print(",");
      Serial.print(calibrater[i][1]);
      Serial.println();
    }
    if (j != 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("CALIBRATION");
      delay(500);
    }
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("CALIBRATION DONE");
      delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(String(calibrater[0][0]) + "," + String(calibrater[0][1]));
      lcd.setCursor(8, 0);
      lcd.print(String(calibrater[1][0]) + "," + String(calibrater[1][1]));
      lcd.setCursor(0, 1);
      lcd.print(String(calibrater[2][0]) + "," + String(calibrater[2][1]));
      delay(1000);
    }

  }
}

void sensor() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SENSORING .....");
  lcd.setCursor(0, 1);
  lcd.print("press any key to sensor");
  key = keypad.waitForKey();
  for (int i = 0; i < 3; i++) {
    digitalWrite(inp_led[i], HIGH);
    delay(1000);
    out = analogRead(value);
    delay(10);
    digitalWrite(inp_led[i], LOW);
    outvalue[i] = map(out, calibrater[i][0], calibrater[i][1], 0, 255);
    outvalue[i] = outvalue[i] < 0 ? 0 : outvalue[i] > 255 ? 255 : outvalue[i];

  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sensor DONE");
  delay(500);
}

void pad() {
  String rgbval = "";
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Type R,G,B value");
  for (int i = 0; i < 11; i++) {
    if (i == 3 || i == 7) {
      lcd.print('/');
      continue;
    }
    char rgbno = keypad.waitForKey();
    lcd.setCursor(i, 1);
    lcd.print(rgbno);
    rgbval += rgbno;
  }
  for (int i = 0; i < 3; i++) {
    outvalue[i] = rgbval.substring(3 * i, (3 * i + 3)).toInt();
  }

}
void screenout() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("R=" + String(outvalue[0]));
  lcd.setCursor(7, 0);
  lcd.print("G=" + String(outvalue[1]));
  lcd.setCursor(0, 1);
  lcd.print("B=" + String(outvalue[2]));

  delay(1000);
}
void ledout() {
  for (int i = 0; i < 3; i++) {
    analogWrite(out_led[i], outvalue[i]);
  }
  delay(1000);
  for (int i = 0; i < 3; i++) {
    digitalWrite(out_led[i], LOW);
  }
}
