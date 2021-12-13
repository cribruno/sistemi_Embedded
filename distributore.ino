#include <LiquidCrystal.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "HX711.h"

//ultrasuoni
const int TRIGGER_PIN = 4;
const int ECHO_PIN = 5;

long duration = 5;
float distance = 5;

//ricevitore infrarossi
const int IR_PIN = 3;
//int irRead;
int new_irRead;
int old_irRead = 1;

//pompa d'acqua
const int PUMP_PIN = 7;

//LCD
LiquidCrystal lcd(8,9,10,11,12,13);

//sensore di temperature
const int ONE_WIRE_BUS = 2;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensore(&oneWire);

//modulo di carico
HX711 scale(A0, A1);

float calibration_factor = -590;
float units;
float ounces;

//"riscaldabevanda"
const int BUTTON_PIN = A2;
const int LED_PIN = 6;

const int WAITTIME = 75;
const int STEP = 5;

int buttonState = LOW;
float temperature = 0; //temperatura simulata
float heat = 0;   //fattore di riscaldamento
int time = 0;

float riscaldaBevanda(float temperature){
  int i;

  i = 0;
  while ( i <= 255 ){
    analogWrite( LED_PIN, i );
    delay( WAITTIME );
    i = i + STEP;
  }

  while(temperature < 70.0){
    LCDriga0(temperature);
    temperature += 2;
    delay(500);
  }

  i = 255;
  while ( i >= 0 ){
    analogWrite( LED_PIN, i );
    delay( WAITTIME );
    i = i - STEP;
  }

  return temperature;

}

void LCDriga0(float temperature){
  String s = "";
  lcd.setCursor(0,0);
  s += "Temp. ";
  s += temperature;
  s += " C  ";
  lcd.print(s);
}

void LCDriga1(){
  String s = "";
  lcd.setCursor(0,1);
  s += "Peso: ";
  s += pesoBicchiere();
  s += "g   ";
  lcd.print(s);
}

int pesoBicchiere(){

  units = scale.get_units(),10;
  if (units < 0)
  {
    units = 0.00;
  }

  return units;
}


void versaLiquido(){

  lcd.setCursor(0,1);
  lcd.print("Sto versando... ");

  digitalWrite(PUMP_PIN, HIGH);

  float l;

  int t = millis();
  int infr = 0;

  do{

    l = livello();

    if (l == 0){
      l = livello();
    }

    infr = digitalRead(IR_PIN);

  }while(!infr && l > 8.75);
    
  digitalWrite(PUMP_PIN, LOW);

  lcd.clear();

}


float livello(){
    //restituisce la distanza che rileva l'ultrasuoni
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
  
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
  
    duration = pulseIn(ECHO_PIN, HIGH);
  
    distance = duration * 0.034 / 2;

    return distance;
}

void setup(){

    Serial.begin(9600);

    //set up the LCD's number of columns and rows:
    lcd.begin(16,2);
    sensore.begin();

    //setup sensore ultrasuoni
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    //setup ricevitore ir
    pinMode(IR_PIN, INPUT);

    //setup pompa
    pinMode(PUMP_PIN, OUTPUT);

    //setup hx711 (peso)
    scale.set_scale(calibration_factor);
    scale.tare();
   
    //setup "riscaldatore"
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
}

void loop(){

  buttonState = !digitalRead(BUTTON_PIN);

  sensore.requestTemperatures();
  float trueTemp = sensore.getTempCByIndex(0);

  temperature = trueTemp + heat;

  if(buttonState){
    temperature = riscaldaBevanda(temperature);
    heat = temperature - trueTemp;
    delay(200);
    time = millis();
  }

  //se la bevanda è riscaldata e sono passati tot ms
  // => raffredda la bevanda
  if((trueTemp <= temperature) && (millis() - time > 10000)){
    heat /= 1.5;

    if (heat <= 0){
      time = 0;
    }
  }

  LCDriga0(temperature);

  pesoBicchiere();
  LCDriga1();

  new_irRead = digitalRead(IR_PIN);

  if (new_irRead == 0 && livello() > 8.75 && new_irRead != old_irRead){
    versaLiquido();
    versaLiquido();
    lcd.clear();
  }

  old_irRead = new_irRead;

  delayMicroseconds(1);
}
