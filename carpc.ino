#include <Wire.h> // подключаем библиотеку работы с i2c шиной
#include <LiquidCrystal_I2C.h> // подключаем библиотеку для работы с дисплеем по i2c

#include "odspeedmeter.h"

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif
 
LiquidCrystal_I2C lcd(0x3f, 20, 4); // 0x27-адрес модуля в i2c сети, 16 и 2 - количество столбцов и строк соответственно

OdSpeedMeter odSpeedMeter;

uint8_t empty[8]  = {0, 0, 0, 0, 0, 0, 0};
uint8_t larrow[8]  = {0x2,0x6,0xf,0x1f,0xf,0x6,0x2};
uint8_t rarrow[8]  = {0x8,0xc,0x1e,0x1f,0x1e,0xc,0x8};

const int RPMPin = 7;     // the number of the pushbutton pin
const int leftTurnPin = 2;     // the number of the pushbutton pin
const int rightTurnPin = 3;     // the number of the pushbutton pin
const int emergencyPin = 4;     // the number of the pushbutton pin

int buttonState = 0;         // variable for reading the pushbutton status
int i = 0;

int RPM = 0;
int lastRPMInputState = 0;
float RPMFrequency = 1; // in Hz
float RPMUpdateTime = 0;

int turnLightsStart;
int turnLightsEnabled = 0;
float turnLightsFrequency = 2; // in Hz

void setup()
{

  lcd.init();              
  // Print a message to the LCD.
  lcd.backlight();

  lcd.createChar(0, empty);
  lcd.createChar(1, larrow);
  lcd.createChar(2, rarrow);

  lcd.setCursor(0,0);
  lcd.print("                    ");
  lcd.setCursor(0,1);
  lcd.print("Fuel: 76%   Tmp: 102");
  lcd.setCursor(0,2);
  lcd.print(" RPM:       Spd:    ");
  lcd.setCursor(0,3);
  lcd.print(" - Any other info - ");

  Serial.begin(9600);
  
  // initialize the pushbutton pin as an input:
  pinMode(leftTurnPin, INPUT_PULLUP);
  pinMode(rightTurnPin, INPUT_PULLUP);
  pinMode(emergencyPin, INPUT_PULLUP);
  pinMode(RPMPin, INPUT_PULLUP);
  
  analogReference(INTERNAL);
}

void updateTurnLights() {
  int leftTurn = !digitalRead(leftTurnPin);
  int rightTurn = !digitalRead(rightTurnPin);
  int emergency = !digitalRead(emergencyPin);
  int enabled = leftTurn || rightTurn || emergency;
  int delta = (millis() - turnLightsStart) / (1000. / turnLightsFrequency);
  int blinkHigh = !(delta % 2);

  if (!turnLightsEnabled && enabled) {
    turnLightsStart = millis();
    blinkHigh = 1;
  }

  lcd.setCursor(0, 0);
  lcd.printByte(blinkHigh && (leftTurn || emergency) ? 1 : 0);

  lcd.setCursor(19, 0);
  lcd.printByte(blinkHigh && (rightTurn || emergency) ? 2 : 0); 

  turnLightsEnabled = enabled;
}

void updateRPM() {
  int fTime = millis();
  int delta = fTime - RPMUpdateTime;
  // read the state of the pushbutton value:
  int RPMInputState = !digitalRead(RPMPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (RPMInputState == HIGH && RPMInputState != lastRPMInputState) {
    RPM++;
    delay(20);
  }
  lastRPMInputState = RPMInputState; 

  if (delta > 1000 / RPMFrequency) {
    lcd.setCursor(6, 2);
    lcd.print("    ");
    lcd.setCursor(6, 2);
    lcd.print(RPM * 60);
    RPM = 0;

   RPMUpdateTime = fTime;
   }
}
 
void loop()
{
  updateTurnLights();
  updateRPM();
  odSpeedMeter.update();

  char buffer[] = "          \n";
  lcd.setCursor(10, 2);
  lcd.print(odSpeedMeter.getSpeed(buffer));

  strcpy(buffer, "          ");
  lcd.setCursor(7, 0);
  lcd.print(odSpeedMeter.getOdometer(buffer));
  
  delay(1000);
}