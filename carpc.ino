#include <Wire.h> // подключаем библиотеку работы с i2c шиной
#include <LiquidCrystal_I2C.h> // подключаем библиотеку для работы с дисплеем по i2c
#include <TimerOne.h>

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif
 
LiquidCrystal_I2C lcd(0x3f, 20, 4); // 0x27-адрес модуля в i2c сети, 16 и 2 - количество столбцов и строк соответственно


uint8_t empty[8]  = {0, 0, 0, 0, 0, 0, 0};
uint8_t larrow[8]  = {0x2,0x6,0xf,0x1f,0xf,0x6,0x2};
uint8_t rarrow[8]  = {0x8,0xc,0x1e,0x1f,0x1e,0xc,0x8};

const int RPMPin = 7;     // the number of the pushbutton pin
const int leftTurnPin = 2;     // the number of the pushbutton pin
const int rightTurnPin = 3;     // the number of the pushbutton pin
const int emergencyPin = 4;     // the number of the pushbutton pin
const int speakerPin = 10;     // the number of the pushbutton pin

int buttonState = 0;         // variable for reading the pushbutton status
int i = 0;
int lastSecond = 0;

float speed = 6;
float speedFrequency = 2; // in Hz
float speedUpdateTime = 0;

float odometer = 0;
float odometerFrequency = 1; // in Hz
float odometerUpdateTime = 0;

int RPM = 0;
int lastRPMInputState = 0;

int turnLightsStart;
int turnLightsMode = 0;
int turnLightsEnabled = 0;
float turnLightsFrequency = 2; // in Hz


const byte COUNT_NOTES = 39; // Колличество нот
int frequences[COUNT_NOTES] = {
  392, 392, 392, 311, 466, 392, 311, 466, 392,
  587, 587, 587, 622, 466, 369, 311, 466, 392,
  784, 392, 392, 784, 739, 698, 659, 622, 659,
  415, 554, 523, 493, 466, 440, 466,
  311, 369, 311, 466, 392
};
//длительность нот
int durations[COUNT_NOTES] = {
  350, 350, 350, 250, 100, 350, 250, 100, 700,
  350, 350, 350, 250, 100, 350, 250, 100, 700,
  350, 250, 100, 350, 250, 100, 100, 100, 450,
  150, 350, 250, 100, 100, 100, 450,
  150, 350, 250, 100, 750
};

void setup()
{
  Serial.begin(9600);
  Timer1.initialize();

  // initialize the pushbutton pin as an input:
  pinMode(leftTurnPin, INPUT_PULLUP);
  pinMode(rightTurnPin, INPUT_PULLUP);
  pinMode(emergencyPin, INPUT_PULLUP);
  pinMode(RPMPin, INPUT_PULLUP);
  
  pinMode(speakerPin, OUTPUT);

  lcd.init();                      // initialize the lcd 
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

  for (int i = 0; i < 1000; i++) {
    // lcd.setCursor(0,0);
    // lcd.printByte(i % 2 ? 1 : 0);
    // lcd.setCursor(19,0);
    // lcd.printByte(i % 2 ? 2 : 0);
    // delay(500);

  }   

  /*
  for (int i = 0; i < COUNT_NOTES; i++  ) { // Цикл от 0 до количества нот
    tone(speakerPin, frequences[i], durations[i] * 2); // Включаем звук, определенной частоты
    delay(durations[i] * 2);  // Дауза для заданой ноты
    noTone(speakerPin); // Останавливаем звук
  }
  */
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

void updateOdometer() {
  int fTime = millis();
  int delta = fTime - odometerUpdateTime;
  if (delta > 1000 / odometerFrequency) {
    char buffer[24];
    char result[24];

    odometer += speed / 3600. / 1000. * delta;

    dtostrf(odometer, 0, 1, buffer);
    sprintf(result, "%skm", buffer);
    lcd.setCursor(7, 0);
    lcd.print(result);

    odometerUpdateTime = fTime; 
  }
}
 
void updateSpeed() {
  int fTime = millis();
  int delta = fTime - speedUpdateTime;
  if (delta > 1000 / speedFrequency) {
   lcd.setCursor(18, 2);
   lcd.print("  ");
   lcd.setCursor(17, 2);
   lcd.print(int(speed));

   speedUpdateTime = fTime;
 }
}

void updateRPM() {
  // read the state of the pushbutton value:
  int RPMInputState = !digitalRead(RPMPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (RPMInputState == HIGH && RPMInputState != lastRPMInputState) {
    RPM++;
    delay(20);
  }
  lastRPMInputState = RPMInputState; 

  int second = millis() / 1000;
  if (lastSecond != second) {
    lcd.setCursor(6, 2);
    lcd.print("    ");
    lcd.setCursor(6, 2);
    lcd.print(RPM * 60);
    RPM = 0;
  }
  lastSecond = second;
}
 
void loop()
{

  updateTurnLights();
  updateOdometer();
  updateSpeed();
  updateRPM();


  /*
  int i = 0;
  for (i = 200; i < 5000; i += 5) {
    Timer1.setPeriod(1000000 / i);
    Timer1.pwm(speakerPin, 255);   
    delay(1);
  }
  */
}