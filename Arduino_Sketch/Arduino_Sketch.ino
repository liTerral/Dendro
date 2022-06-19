#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <EEPROM.h>
#include <GyverPower.h>

#define pin_moistureSensor A0
#define pin_moisturePower 4
#define pin_btnMode 2
#define pin_btnAct 3

#define pin_crtLowTemp 5
#define pin_crtHighTemp 6
#define pin_crtLowPress 7
#define pin_crtHighPress 8
#define pin_crtLowHum 9
#define pin_crtHighHum 10
#define pin_crtLowSoilMoist 11
#define pin_crtHighSoilMoist 12

LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_BME280 bme;


int maxMoisture = 100;

int crtLowTemp = 0, crtHighTemp = 60;
int crtLowHum = 10, crtHighHum = 95;
int crtLowSoilMoist = 20, crtHighSoilMoist = 200;
int crtLowPress = 0, crtHighPress = 810;

unsigned long lastUpd_tm = 0, lastAction_tm = 0;
unsigned long timeUpd = 30, timeSleep = 3600, timeWait = 300;

float temperature = 0, pressure = 0, humidity = 0, soilMoisture = 0;

bool activeMode = true, bklight = true, pcMode = true;
bool chkCrtTemp = false, chkCrtHum = false, chkCrtSoilMoist = false, chkCrtPress = false;
bool bme280 = false;

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
    
      if (millis() - press_tm <= 800) status = 1;
      else status = 2;
    }
  }

  byte getLastStatus() {
    return status;
  }

};

Button btnMode(pin_btnMode), btnAct(pin_btnAct);



void setup() {
  EEPROM.get(0, valTemp); // 2
  EEPROM.get(1, valPres);
  
  EEPROM.get(2, timeUpd); // 12
  EEPROM.get(6, timeSleep);
  EEPROM.get(10, timeWait);

  EEPROM.get(14, pcMode); //2
  EEPROM.get(15, bklight);

  EEPROM.get(16, chkCrtTemp); // 5
  EEPROM.get(17, crtLowTemp);
  EEPROM.get(19, crtHighTemp);

  EEPROM.get(21, chkCrtPress); // 5
  EEPROM.get(22, crtLowPress);
  EEPROM.get(24, crtHighPress);

  EEPROM.get(26, chkCrtHum); // 5
  EEPROM.get(27, crtLowHum);
  EEPROM.get(29, crtHighHum);

  EEPROM.get(31, chkCrtSoilMoist); // 5
  EEPROM.get(32, crtLowSoilMoist);
  EEPROM.get(34, crtHighSoilMoist);

  EEPROM.get(36, maxMoisture); // 2
  
  pinMode(pin_btnMode, INPUT_PULLUP);
  pinMode(pin_btnAct, INPUT_PULLUP);
  pinMode(pin_moisturePower, OUTPUT);

  pinMode(pin_crtLowTemp, OUTPUT);
  pinMode(pin_crtHighTemp, OUTPUT);
  pinMode(pin_crtLowPress, OUTPUT);
  pinMode(pin_crtHighPress, OUTPUT);
  pinMode(pin_crtLowHum, OUTPUT);
  pinMode(pin_crtHighHum, OUTPUT);
  pinMode(pin_crtLowSoilMoist, OUTPUT);
  pinMode(pin_crtHighSoilMoist, OUTPUT);

  power.setSleepMode(POWERDOWN_SLEEP);
  power.calibrate(8562);      //(int) = getMaxTimeout()
  power.correctMillis(true);
  
  Serial.begin(9600);

  lcd.init();
  lcd.createChar(0, circle);
  lcd.createChar(1, arrows);

  if (bklight) lcd.backlight();
  

  for (byte i = 0; i < 10; i++) {
    if (!bme.begin(0x76)) {
      lcd.clear();
      lcd.setCursor(2, 1);
      lcd.print("failed start");
      lcd.setCursor(2, 2);
      lcd.print("reason: BME280");
      
      delay(500);
    } else {
      bme280 = true;
      break;
    }
  }

  lcd.clear();
  lastAction_tm = millis();
}

void loop() {
  if (lastUpd_tm + timeUpd*1000 <= millis() || lastUpd_tm == 0) {
    updData();
    crtReaction();
  }

  if (pcMode && Serial.available()) {
    readParameters();
  }

  btnModeFunc();
  btnActFunc();

  if (timeSleep > 0 && activeMode && (lastAction_tm + timeWait*1000 <= millis())) {
    activeMode = false;
  }

  if (!activeMode) {
    turnOnDisp(false);

    Serial.end();
    
    attachInterrupt(0, btnWakeUp, RISING);
    power.sleepDelay(timeSleep*1000 - (millis() - lastUpd_tm));
    detachInterrupt(0);

    Serial.begin(9600);

    if (activeMode) {
      turnOnDisp(bklight);
    }
  }
}



void readParameters() {
  String package = "";
  package = Serial.readString();
  package.trim();

  String tmp = "";
  byte n = 0;
  
  for (int i = 0; i < package.length(); i++) {
    if (package[i] != ',' && i + 1 != package.length()) {
      tmp += package[i];
    }
    else if (package[i] == ',') {
      switch (n)
      {
        case 0:
          timeUpd = tmp.toFloat();
          EEPROM.put(2, timeUpd);

          break;
        case 1:
          timeSleep = tmp.toFloat();
          EEPROM.put(6, timeSleep);

          break;
        case 2:
          timeWait = tmp.toFloat();
          EEPROM.put(10, timeWait);

          break;
        case 3:
          chkCrtTemp = tmp.toInt();
          EEPROM.put(16, chkCrtTemp);
        
          break;
        case 4:
          crtLowTemp = tmp.toInt();
          EEPROM.put(17, crtLowTemp);

          break;
        case 5:
          crtHighTemp = tmp.toInt();
          EEPROM.put(19, crtHighTemp);

          break;
        case 6:
          chkCrtPress = tmp.toInt();
          EEPROM.put(21, chkCrtPress);
        
          break;
        case 7:
          crtLowPress = tmp.toInt();
          EEPROM.put(22, crtLowPress);

          break;
        case 8:
          crtHighPress = tmp.toInt();
          EEPROM.put(24, crtHighPress);

          break;
        case 9:
          chkCrtHum = tmp.toInt();
          EEPROM.put(26, chkCrtHum);
        
          break;
        case 10:
          crtLowHum = tmp.toInt();
          EEPROM.put(27, crtLowHum);

          break;
        case 11:
          crtHighHum = tmp.toInt();
          EEPROM.put(29, crtHighHum);

          break;
        case 12:
          chkCrtSoilMoist = tmp.toInt();
          EEPROM.put(31, chkCrtSoilMoist);

          break;
        case 13:
          crtLowSoilMoist = tmp.toInt();
          EEPROM.put(32, crtLowSoilMoist);

          break;
        case 14:
          crtHighSoilMoist = tmp.toInt();
          EEPROM.put(34, crtHighSoilMoist);

          break;
        case 15:
          valTemp = tmp.toInt();
          EEPROM.put(0, valTemp);

          break;
      }
      tmp = "";
      n++;
    }
    else {
      tmp += package[i];
      valPres = tmp.toInt();
      EEPROM.put(1, valPres);
    }
  }

  printParameters();
  lcdPrintData();
}

void printParameters() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Act.Upd.: " + String(timeUpd));

  lcd.setCursor(0, 1);
  lcd.print("Pas.Upd.: " + String(timeSleep));

  lcd.setCursor(0, 2);
  lcd.print("To wait:  " + String(timeWait));

  while (true) {
    btnAct.checkBtn();
    if (btnAct.getLastStatus() == 1 || btnAct.getLastStatus() == 2)
      break;
  }

  lcd.clear();
  lcd.setCursor(0, 0);

  if (chkCrtTemp) lcd.print('+');
  else lcd.print('-');

  lcd.print("Temperat.  " + String(crtLowTemp));
  lcd.setCursor(17, 0);
  lcd.print(String(crtHighTemp));


  lcd.setCursor(0, 1);

  if (chkCrtPress) lcd.print('+');
  else lcd.print('-');

  lcd.print("Pressure   " + String(crtLowPress));
  lcd.setCursor(15, 1);
  lcd.print("  " + String(crtHighPress));


  lcd.setCursor(0, 2);

  if (chkCrtHum) lcd.print('+');
  else lcd.print('-');

  lcd.print("Humidity   " + String(crtLowHum));
  lcd.setCursor(17, 2);
  lcd.print(String(crtHighHum));


  lcd.setCursor(0, 3);

  if (chkCrtSoilMoist) lcd.print('+');
  else lcd.print('-');

  lcd.print("Soil Mois. " + String(crtLowSoilMoist));
  lcd.setCursor(17, 3);
  lcd.print(String(crtHighSoilMoist));

  while (true) {
    btnAct.checkBtn();
    if (btnAct.getLastStatus() == 1 || btnAct.getLastStatus() == 2)
      break;
  }
}


void updData() { 
  lastUpd_tm = millis();
  getValues();

  if (activeMode) {
    lcdPrintData();
  }
    
  if (pcMode) {
    serialPrintData();
  }
}


void getValues() {
  digitalWrite(pin_moisturePower, HIGH);
  
  if(bme280) {
    temperature = bme.readTemperature();
    pressure = bme.readPressure();
    humidity = bme.readHumidity();
  }
  
  soilMoisture = analogRead(pin_moistureSensor);
  digitalWrite(pin_moisturePower, LOW);
}

void serialPrintData() {
  Serial.print(String(resTemperature(temperature)) + " *" + String(varTemp[valTemp]) + ",");
  Serial.print(String(resPressure(pressure)) + " " + String(varPres[valPres]) + ",");
  Serial.print(String(humidity) + ",");
  Serial.println(String(map(soilMoisture, maxMoisture, 1023, 100, 0)));
}

void lcdPrintData() {
  lcd.clear();
  lcd.setCursor(0, 0);

  if (bme280) {
    lcd.print(String(resPressure(pressure)) + " " + String(varPres[valPres]));

    lcd.setCursor(11, 0);
    lcd.print(String(resTemperature(temperature)) + " ");
    lcd.write(byte(0));
    lcd.print(varTemp[valTemp]);

    lcd.setCursor(0, 2);
    lcd.print("Humidity:   " + String(int(humidity)) + " %");
  
    lcd.setCursor(0, 3);
  }
  lcd.print("Soil.Moist: " + String(map(soilMoisture, maxMoisture, 1023, 100, 0)) + " %");

  lcdPrintIndication();
}

void lcdPrintIndication() {
  lcd.setCursor(19, 3);

  if (pcMode) lcd.write(byte(1));
  else lcd.print(" ");
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
    return pressure/133.32237;
  else if (valPres == 2)
    return pressure/3386.375258;
}


void crtReaction() {
  if (chkCrtTemp) {
    digitalWrite(pin_crtLowTemp, temperature < crtLowTemp);
    digitalWrite(pin_crtHighTemp, temperature > crtHighTemp);
  }

  if (chkCrtPress) {
    digitalWrite(pin_crtLowPress, pressure < crtLowPress * 133.32237);
    digitalWrite(pin_crtHighPress, pressure > crtHighPress * 133.32237);
  }

  if (chkCrtHum) {
    digitalWrite(pin_crtLowHum, humidity < crtLowHum);
    digitalWrite(pin_crtHighHum, humidity > crtHighHum);
  }

  if (chkCrtSoilMoist) {
    digitalWrite(pin_crtLowSoilMoist, map(soilMoisture, maxMoisture, 1023, 100, 0) < crtLowSoilMoist);
    digitalWrite(pin_crtHighSoilMoist, map(soilMoisture, maxMoisture, 1023, 100, 0) > crtHighSoilMoist);
  }
}


void turnOnDisp(bool on) {
  if (on) {
      lcd.backlight();
      lcd.display();
    }
    else {
      lcd.noBacklight();
      lcd.noDisplay();
    }
}


void btnModeFunc() {
  btnMode.checkBtn();

  if (btnMode.getLastStatus() == 1) {
    bklight = !bklight;
    EEPROM.put(15, bklight);
    
    turnOnDisp(bklight);

    lastAction_tm = millis();
  }
  else if (btnMode.getLastStatus() == 2) {
    activeMode = false;
  }
}

void btnActFunc() {
  btnAct.checkBtn();

  if (btnAct.getLastStatus() == 1) {
    pcMode = !pcMode;
    EEPROM.put(14, pcMode);
    lcdPrintIndication();

    lastAction_tm = millis();
  }
  else if (btnAct.getLastStatus() == 2) {
    maxMoisture = soilMoisture;
    EEPROM.put(36, maxMoisture);
    lcdPrintData();

    lastAction_tm = millis();
  }
}


void btnWakeUp() {
  power.wakeUp();

  lastAction_tm = millis();
  activeMode = true;
}
