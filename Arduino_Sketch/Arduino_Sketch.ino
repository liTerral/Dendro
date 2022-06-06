#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <GyverPower.h>

#define pin_moistureSensor A0
#define pin_moisturePower 7
#define pin_btnMode 2
#define pin_btnAct 3

LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_BME280 bme;


int maxMoisture = 100;

int crtLowTemp = 0, crtHighTemp = 50;
int crtLowHum = 0, crtHighHum = 95;
int crtLowSoilMoist = 20, crtHighSoilMoist = 150;
float crtLowPress = 0, crtHighPress = 200000;

unsigned long lastUpd_tm = 0, lastAction_tm = 0;
unsigned long timeActUpd = 30, timePasUpd = 3600, timeWait = 300;

float temperature = 0, pressure = 0, humidity = 0, soilMoisture = 0;

bool activeMode = true, bklight = true, pcMode = false;

const char varTemp[2] = {'C', 'F'};
const char *varPres[] = {"Pa", "mmHg", "inHg"};
byte valTemp = 0, valPres = 1;

byte circle[8] = {0x02, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
byte arrows[8] = {0x00, 0x0A, 0x1A, 0x0A, 0x0A, 0x0B, 0x0A, 0x00};


class Button {
  byte pin, status = 0;
  bool flag = false;
  unsigned long press_tm = 0;

public:
  Button(byte pin){
    this->pin = pin;
  }

  void checkBtn() {
    //INPUT_PULLUP - натиснення: 0

    if (!flag && digitalRead(pin)) {
      status = 0;
    }
    else if (!flag && !digitalRead(pin)) {
      flag = true;
      status = 0;

      press_tm = millis();
    }
    else if (flag && digitalRead(pin) && press_tm >= 100) {
      flag = false;
    
      if (millis() - press_tm <= 1000) status = 1;
      else status = 2;
    }
  }

  byte getLastStatus() {
    return status;
  }

};

Button btnMode(pin_btnMode), btnAct(pin_btnAct);



void setup() {
  pinMode(pin_btnMode, INPUT_PULLUP);
  pinMode(pin_btnAct, INPUT_PULLUP);
  pinMode(pin_moisturePower, OUTPUT);

  power.setSleepMode(POWERDOWN_SLEEP);
  power.calibrate(8562);      //(int) = getMaxTimeout()
  power.correctMillis(true);

  //attachInterrupt(0, btnWakeUp, FALLING);
  //detachInterrupt(0);
  //power.sleepDelay(timeSleep);
  
  Serial.begin(9600);

  lcd.init();
  lcd.createChar(0, circle);
  lcd.createChar(1, arrows);

  if (bklight) lcd.backlight();
  

  while (true) {
    if (!bme.begin(0x76)) {
      lcd.setCursor(0, 0);
      lcd.print("!BME280");
      delay(500);
    } else break;
  }

  lcd.clear();

  getValues();
  lcdPrintData();

  lastUpd_tm = millis();
  lastAction_tm = millis();
}



void loop() {
  updData();

  btnModeFunc();
  btnActFunc();

  if (activeMode && millis() - lastAction_tm >= timeWait * 1000) {

  }
}



void updData() { 
  if ((activeMode && lastUpd_tm + timeActUpd*1000 <= millis()) 
        || (!activeMode && lastUpd_tm + timePasUpd <= millis())) {
    lastUpd_tm = millis();
    getValues();

    if (activeMode) {
      lcdPrintData();
    }
    
    if (pcMode) {
      serialPrintData();
    }

    if (pcMode && activeMode) {
      lcd.setCursor(19, 3);
        lcd.write(byte(1));
    }
  }
}


void getValues() {
  digitalWrite(pin_moisturePower, HIGH);
  
  temperature = bme.readTemperature();
  pressure = bme.readPressure();
  humidity = bme.readHumidity();

  soilMoisture = analogRead(pin_moistureSensor);
  digitalWrite(pin_moisturePower, LOW);
}

void serialPrintData() {
  Serial.println(String(resTemperature(temperature)) + " " + String(varTemp[valTemp]));
  Serial.println(String(resPressure(pressure)) + " " + String(varPres[valPres]));
  Serial.println(String(humidity) + " %");
  Serial.println(String(map(soilMoisture, maxMoisture, 1023, 100, 0)) + " %\n");
}

void lcdPrintData() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(String(resPressure(pressure)) + " " + String(varPres[valPres]));

  lcd.setCursor(11, 0);
  lcd.print(String(resTemperature(temperature)) + " ");
  lcd.write(byte(0));
  lcd.print(varTemp[valTemp]);

  lcd.setCursor(0, 2);
  lcd.print("Humidity:   " + String(int(humidity)) + " %");
  
  lcd.setCursor(0, 3);
  lcd.print("Soil.Moist: " + String(map(soilMoisture, maxMoisture, 1023, 100, 0)) + " %");
}


float resTemperature(float temperature) {
  if (valTemp == 0)
    return temperature;
  else if (valTemp == 1)
    return (temperature*1.8)+32;
}

long resPressure(float pressure) {
  if (valPres == 0)
    return pressure;
  else if (valPres == 1)
    return pressure/133.32239;
  else if (valPres == 2)
    return pressure/3386.375258;
}


void btnModeFunc() {
  btnMode.checkBtn();

  if (btnMode.getLastStatus() == 0) {
    

  }
  else if (btnMode.getLastStatus() == 1) {

    if (bklight) lcd.noBacklight();
    else lcd.backlight();

    bklight = !bklight;

    lastAction_tm = millis();
  }
  else if (btnMode.getLastStatus() == 2) {
    
    lastAction_tm = millis();
  }
}

void btnActFunc() {
  btnAct.checkBtn();

  if (btnAct.getLastStatus() == 0) {
    
  }
  else if (btnAct.getLastStatus() == 1) {
    lcd.setCursor(19, 3);
    
    if (!pcMode) lcd.write(byte(1));
    else lcd.print(" ");

    pcMode = !pcMode;

    lastAction_tm = millis();
  }
  else if (btnAct.getLastStatus() == 2) {
    maxMoisture = soilMoisture;
    
    lcdPrintData();

    lastAction_tm = millis();
  }
}


void btnWakeUp() {
  power.wakeUp();

  lastAction_tm = millis();
  activeMode = true;
}
