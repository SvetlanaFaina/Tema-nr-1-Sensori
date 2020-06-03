//Faina Svetlana 

/*
LCD 16x2 4 bit mode/ for display sensors values
Piezzo element for sound attention if some of sensors get max value
PIR Sensor - movment detection
UDS - Ultrasonic distance sensor - get distance from object, if moves
TMP - temperature sensor - get temperature
Photoresistor - LDR get light intesity(to do conversion in lux)

=====================================

Circuit LCD:
RS pin to DP 8
E pin to DP 9
DB4 pin to DP 10
DB5 pin to DP 11
DB6 pin to DP 12
DB7 pin to DP 13
LED+ pin to res 220 Ohm to +5V
---------------------------------------
*/
//initiem libraria pentru LCD si keypad
#include <LiquidCrystal.h> 
#include <Keypad.h>

//declar variabile globale
char BUTvalue; 
int ButonState = 0;
float var_Misc; 
float var_Dist;
float var_Temp;
float var_Ilum; 


//declaram volorile limita
int max_Dist = 290;
int minDist = 35;
int max_Temp = 40;  
int minTemp = 15;
int min_Ilum = 30;
//initiem libraria cu interfata pinilor
LiquidCrystal lcd(8,9,10,11,12,13);

void setupLCD()
{
 lcd.begin(16,2);//setam numarul de colonite si rinduri
 lcd.clear();
}

//initiem keypad
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

//utiliz doar 4 but (2x2)
byte rowPins[ROWS] = {7, 6}; //pini procesor pentru rinduri
byte colPins[COLS] = {5, 4}; //pini procesor pentru colonite

Keypad keypad2x2 = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//inlocuire text in loc de HIGH 1, in loc de LOW 0
#define HIGH 1
#define LOW 0
 

//init R & G LEDs
#define RL_PIN 2
#define GL_PIN 3
void SetupLED(){
  pinMode (RL_PIN, OUTPUT);
  pinMode (GL_PIN, OUTPUT);
}

void RED_LED_ON(){
  digitalWrite (RL_PIN, 1);
  digitalWrite (GL_PIN, 0);
}

void GREEN_LED_ON(){
  digitalWrite (RL_PIN, 0);
  digitalWrite (GL_PIN, 1);
}

void LEDs_OFF(){
  digitalWrite (RL_PIN, 0);
  digitalWrite (GL_PIN, 0); 
}

//=========Piezo (Buzzer)========//
#define SpeakerPin A5

void SetupSpeaker(){
  pinMode(SpeakerPin, OUTPUT);
}

int period = 250;//pentru perioada de redare
unsigned long time_now = 0;// pentru timp de redare

//==============================//

typedef struct Multi_Sensor 
{
 float (*get) (void);//pointer
 char NumeParametru[20];//numar caractere in denumire parametru
 char NumeUnitMasura[10];//numar caractere in denumire unitate de masura
 char NumBut[2];//numar caractere buton atribuit
}
Sensor_Type;

enum {S_PIR, S_UDS, S_TMP, S_LDR, S_Nothing};
//PIR=Motion, UDS=Distance, TMP=temperatura, LDR=ligth dependent resistor

//=====PIR Driver =======//
#define PIR_PIN A4 

void SetupPIR(){
  pinMode (PIR_PIN, INPUT);
}

float ReadPIR(){
  int MovState = digitalRead(PIR_PIN);// returneaza ce citeste din pin
  return MovState;// a fost TMPstate
}
//=======================//

//======UDS Driver=======//
#define UDS_TRIG_PIN A3
#define UDS_ECHO_PIN A2
//UDS_TRIG_PIN 7// A3 - tot nu merge, vezi mai jos
//UDS_ECHO_PIN 6//A2 - nu merge, se citeste 0 si frineaza tot procesul

long durata_echo;
int distanta;

void SetupUDS(){
 pinMode (UDS_TRIG_PIN, OUTPUT);//setare pin TRIG iesire
 pinMode (UDS_ECHO_PIN, INPUT);//setare pin Echo intrare
}

float CalcDistanta(){
  //un ciclu cu 1 puls de 10 mcs TRIG emis
  digitalWrite (UDS_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite (UDS_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite (UDS_TRIG_PIN, LOW);
  //citire reflectie semnal
  durata_echo = pulseIn(UDS_ECHO_PIN, HIGH);//nu este I(i mare) ci l (l mic)
  //calculare distanta
  distanta = durata_echo*0.034/2;
  return distanta;
}
//============================//

//=========TMP Driver==========//
#define TMP_PIN A1
// ADC range
#define ADC_MIN 0
#define ADC_MAX 1023
// Voltage range
#define VOLT_MIN 0
#define VOLT_MAX 5
// Temperature range
#define TEMP_MIN -40
#define TEMP_MAX 125

//float celsius;

void SetupTMP(){
 pinMode (TMP_PIN, INPUT);// setare pin intrare
}

float CalcTemperatura(void) {
  int rawData = analogRead(TMP_PIN);
  float celsius = map(((rawData - 20) * 3.04), ADC_MIN, ADC_MAX, TEMP_MIN, TEMP_MAX);
  return celsius; //return temperature;
}
//==============================//

//===========LDR Driver==========//
#define LDR_PIN A0
#define LIGHT_MIN 10
#define LIGHT_MAX 100
#define ADC_MIN 0
#define ADC_MAX 1023
#define VOLT_MIN 0
#define VOLT_MAX 5

void SetupLDR() {
  pinMode(LDR_PIN, INPUT);//setare pin la intrare
}
// aici de gasit si de pus formula cu transmirmare in lux
float CalcLux() {
  int rawData = analogRead(LDR_PIN);
  float voltage = map(rawData, ADC_MIN, ADC_MAX, VOLT_MIN, VOLT_MAX);
  float lux = map(voltage, VOLT_MIN, VOLT_MAX, LIGHT_MIN, LIGHT_MAX);
  
  return lux;
}
//================================//

//====functia enumerare denumiri parametri, nume unitati de masura=====//
Sensor_Type sensorList[S_Nothing] = {//pentru ca sunt 4 sensore, al 5 este ultimul care nu-l avem
  {ReadPIR,"Miscare", "Da/Nu", '1'},
  {CalcDistanta,"Distanta","cm", '2'},
  {CalcTemperatura,"Temperatura","celsius", '4'},
  {CalcLux,"Iluminanta","lux", '5'}
 };
//=================================//

//===popularea listei cu valori, denumiri: parametri, unitati de masura 
float SensorGetValue(int id) {
    if(id < S_Nothing) {
      return sensorList[id].get();
    }
}

char * SensorGetUnit(int id) {
    if(id < S_Nothing) {
      return sensorList[id].NumeUnitMasura;
    }
}

char * SensorGetParamName(int id) {
    if(id < S_Nothing) {
      return sensorList[id].NumeParametru;
    }
}

char * SensorGetButonAtribuit(int id) {
    if(id < S_Nothing) {
      return sensorList[id].NumBut;
    }
  
}
//================================//


void setup()
{
  //display
  Serial.begin(9600);
  setupLCD();
  SetupLED();
  //sensors
  SetupPIR();
  SetupUDS();
  SetupTMP();
  SetupLDR();
  //audio
  SetupSpeaker();
}


void loop()
{
    lcd.setCursor(0, 0);//primul rind
    lcd.print ("MULTISENSOR TEST");//
  
  // incerc sa schimb S_Nothing cu cifra 4, tot ok
    for(int i=0; i<S_Nothing; i++) {
    char * paramName = SensorGetParamName(i);
    Serial.print(paramName);
    Serial.print(": ");
    
    float value = SensorGetValue(i);  
    Serial.print(value);
    
    char * unit = SensorGetUnit(i);
    Serial.print(" ");
    Serial.print(unit);
    Serial.print(" , ");

    char * But = SensorGetButonAtribuit(i);
    Serial.print(" ");
    Serial.print(But);
    Serial.print(" , ");
      
   char input = keypad2x2.getKey();
   Serial.print(input);
   BUTvalue = input;
    if (BUTvalue == '1') ButonState = 1;
    if (BUTvalue == '2') ButonState = 2;
    if (BUTvalue == '4') ButonState = 3;
    if (BUTvalue == '5') ButonState = 4;
//avem acumulate date pentru fiecare senzor 
//vreau sa le scot intr-o variabila pe toate impreuna
    if (i == 0) var_Misc = value; 
    if (i == 1) var_Dist = value;
    if (i == 2) var_Temp = value;
    if (i == 3) var_Ilum = value;
      
   Serial.println();//trece din rind nou fiecare parametru, fiecare for

    } // sfirsit for

  Serial.println();//un rind nou gol pentru despartire sfirsit ciclu for
 

//======= Sectiune afisare LCD========//
 lcd.setCursor(0, 1);// al doilea rind

//---afisare miscare----
 if (ButonState == 1){
   if (var_Misc) {
	lcd.print("  ESTE Miscare  ");
     RED_LED_ON();
   }else{
    lcd.print("NU ESTE Miscare "); 
     GREEN_LED_ON();
   }        
 }else LEDs_OFF();

//----afisade distanta----
 if (ButonState == 2){
   lcd.print(" Distanta ");
   lcd.print(round(var_Dist));
   lcd.print("cm  ");
   if(var_Dist < max_Dist && var_Dist > minDist){
     GREEN_LED_ON();
   }else RED_LED_ON();
 }else LEDs_OFF();
  
//-----afisare temperatura-----
  if (ButonState == 3){
   lcd.print("Temperatura ");
   lcd.print(round(var_Temp));
   lcd.print("C ");
   if(var_Temp < max_Temp && var_Temp > minTemp){
     GREEN_LED_ON();
   }else RED_LED_ON();
  }else LEDs_OFF();
        
//-----afisare Iluminare----- // nu am gasit formula pentru conversie in lux
  if (ButonState == 4){
   lcd.print("Iluminanta ");
   lcd.print(round(var_Ilum));
   lcd.print("PT");
    if (var_Ilum < min_Ilum){
      RED_LED_ON();
    }else GREEN_LED_ON();
  }else LEDs_OFF();
  
}
