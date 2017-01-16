//This code drive an Arduino Mega to control a few solinoid valves and a pump
//to irrigate soil based on sensor data.
//
//This was coded by Andrew Simon for the ES1050 Design project.
//This code was last modified in April, 2016. 
//

//Preprocessor directives
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>

//Pin declarations for the realy control board.
const int relayPump = 2;
const int relaySol1 = 3;
const int relaySol2 = 4;
const int relaySol3 = 5;
const int relaySol4 = 6;

//Soil Sensor pins.
//Analoge Values
const int soilSensA_1 = A0;
const int soilSensA_2 = A1;
const int soilSensA_3 = A2;
const int soilSensA_4 = A3;
//Digital Values
const int soilSensD_1 = 52; //52,50,48,46
const int soilSensD_2 = 50;
const int soilSensD_3 = 48;
const int soilSensD_4 = 46;

//Assignes the pin that the each DHT11 is attached to 
const int pinDHT11_1 = 40;
const int pinDHT11_2 = 38;
const int pinDHT11_3 = 36;
const int pinDHT11_4 = 34;

//Defines the DHT according to the library class, type and pin assigned.
DHT dht11_1(pinDHT11_1, DHT11);
DHT dht11_2(pinDHT11_2, DHT11);
DHT dht11_3(pinDHT11_3, DHT11);
DHT dht11_4(pinDHT11_4, DHT11);

//Sets up the LCD 
LiquidCrystal_I2C LCD(0x27,20,4);

//Sets up an array to monitor the relays.
const int relayPin[] = {2,3,4,5,6};
bool relayToggle[6];


//Global Variable to Hold sensor data;
double h[4], t[4];
int a[4], d[4];





void setup() {
  
  //Initialzes the Serial output on the Arduino.
  Serial.begin(9600);
  
  //Initializes the LCD
  LCD.init();
  LCD.backlight();
  
  PrintLCD("    Group 10", "    ES1050", 12);
  //UNCOMMENT
  
  //Initializes the DHT11 Chips.
  dht11_1.begin();
  dht11_2.begin();
  dht11_3.begin();
  dht11_4.begin();
  
  //Initializes the Realy pins.
  pinMode(relayPump, OUTPUT);
  pinMode(relaySol1, OUTPUT);
  pinMode(relaySol2, OUTPUT);
  pinMode(relaySol3, OUTPUT);
  pinMode(relaySol4, OUTPUT);
  
  //Initializes the Water Sensor Digital output.
  pinMode(soilSensD_1, INPUT);
  pinMode(soilSensD_2, INPUT);
  pinMode(soilSensD_3, INPUT);
  pinMode(soilSensD_4, INPUT);
  
  //Sets up the realy toggle array.
  for(int i = 0; i<5; i++){
    relayToggle[i] = false;
  }
  
  //Gets initial sensor data
  getSensorData();
  
  // UNCOMMENT
  //Print out a welcome message. 
  PrintLCD("Welcome to Group", "10s final ES1050", 0);
  delay(1500);
  PrintLCD("Design Project!", "", 0);
  delay(1500);
  
  
  //Call for the set programs.
  if(d[2] == 0){
    PrintLCD("Program #1","******RUN*******" ,0);
    delay(1000);
    intPrg1();
  }else if(d[3] == 0){
    PrintLCD("Program #2","******RUN*******" ,0);
    intPrg2();
  }else{
    PrintLCD("No Prg. Selc.","******RUN*******" ,0);
  }
  

}

void loop() {
  
  //Gets the fresh sensor data.
  getSensorData(); 
 
  //Determine the status of the Relay's based on
  //collected Sensor data.
  detRelayStatus();
  
  //Print the data to screen.
  basicData();
  
  //Cycles through the relays manually. This is mainly used for debugging.
  //cycleRelays();
  
}


void PrintLCD(String msgLine1, String msgLine2, int scroll){
  
  LCD.init();
  LCD.setCursor(0,0);
  LCD.print(msgLine1);
  
  LCD.setCursor(0,1);
  LCD.print(msgLine2);
  
  if(scroll == 0){
    //do nothing
  }else{
      for(int i = 0; i < scroll; i++){
      LCD.scrollDisplayLeft();
      delay(250);
      }
  }
    
}


void detRelayStatus(){
  
  //Convert digital sensor data into boolean logic.
  //WHEN:
  //  dW[i] = true -> the sensor is trigger ON.
  //  dW[i] = false -> the sensor is trigger OFF.
  //beacause...
  //
  //  d[i] = 1 -> the sensor if triggerd OFF.
  //  d[i] = 0 -> the sensor if triggerd ON.
  bool dW[4];
  for(int i = 0; i < 4; i++){
    dW[i] =! d[i];
  }
  
  //Soloniod Opens up first. This is important.
  //IF THE OTHER 2 RELAYS NEED TO BE USED
  //CHANGE THE 2 TO A 4 IN THE LOOP. THIS IS TO PROTECT
  //THE PUMP THAT IS NOT CONNECTED TO A SOLINOID ON THAT
  //BRANCH.
  bool pumpState = false;
  for(int i = 0; i < 2; i++){
    if(dW[i] == true){
      relayCtrl(i+1, true);
      pumpState = true;
    }
  }
  
  //If any of the solinods are open, turn the pump on.
  if(pumpState == true){
    delay(500);
    relayCtrl(0, true);
  }
  
  
  //When all the Sensors are off, turn the pump off, then the solinods.
  int sensOff = 0;
  for(int i = 0; i < 4; i++){
    if(dW[i] == false){
      sensOff++;
    }
  }
  //If all 4 of the SENSORS (NOT RELAYS) are off, turn the pump off.
  if(sensOff == 4){
    relayCtrl(0, false);
   delay(500);
  }
  
  //Turn the Soloids off now.
   for(int i = 0; i < 4; i++){
    if(dW[i] == false){
      relayCtrl(i+1, false);
    }
  }
  
}


//For test purposes only.
//Cycles the Relay's on and off in an increasing and
//decreasing order.
void cycleRelays(){
  
  for(int i = 5; i >= 0; i--){
    //digitalWrite(relayArray[i], LOW);
    relayCtrl(i, true);
    delay(1000); // dongmin 1000 > 500
  }
  
  delay(3000); // dongmin 3000 > 500
  
  for(int i = 0; i < 5; i++){
    //digitalWrite(relayArray[i], HIGH);
    relayCtrl(i, false);
    delay(1000); //dongmin 1000 > 500
  }
  
}


//Controls the Relay's from other parts of the program.
void relayCtrl(int arrayPos, bool stat){
  
    //If status is false, turn the relay on arrayPos off.  
    if(stat == false){
        digitalWrite(relayPin[arrayPos], LOW);
        //Marks the virutal relay array for further program reference.
        relayToggle[arrayPos] = false;
    //If status is false, turn the relay on arrayPos off. 
    }else if(stat == true){
        digitalWrite(relayPin[arrayPos], HIGH);
        //Marks the virutal relay array for further program reference.
        relayToggle[arrayPos] = true;
    }
  
}


//Fetchs the data from the sensors and saves it to a global variable.
void getSensorData(){
  
  //int d1, d2, d3, d4, a1, a2, a3, a4;
  //double h1, h2, h3, h4;
  //double t1, t2, t3, t4;
  
  d[0] = digitalRead(soilSensD_1);
  d[1] = digitalRead(soilSensD_2);
  d[2] = digitalRead(soilSensD_3);
  d[3] = digitalRead(soilSensD_4);
  
  a[0] = analogRead(soilSensA_1);
  a[1] = analogRead(soilSensA_2);
  a[2] = analogRead(soilSensA_3);
  a[3] = analogRead(soilSensA_4);
  
  h[0] = dht11_1.readHumidity();
  t[0] = dht11_1.readTemperature();
  
  h[1] = dht11_2.readHumidity();
  t[1] = dht11_2.readTemperature();
  
  h[2] = dht11_3.readHumidity();
  t[2] = dht11_3.readTemperature();
  
  h[3] = dht11_4.readHumidity();
  t[3] = dht11_4.readTemperature();
  //delay(10000)  //dongmin void > delay(10000) 
  
  
  
  for(int i = 0; i<4; i++){
    Serial.print("Sensor #:");
    Serial.println(i+1);
    Serial.print("Temprature: ");
    Serial.println(t[i]);
    Serial.print("Humidity: ");
    Serial.println(h[i]);
    Serial.print("Moisture Analoge: ");
    Serial.println(a[i]);
    Serial.print("Moisture Digital: ");
    Serial.println(d[i]);
    Serial.println("\n");
  }
  
  
  
  delay(3000);
  
}

void intPrg1(){
  
  //Displays the welcome and instructional message.
  PrintLCD("Getting sensor", "data please", 0);
  delay(1000);
  PrintLCD("wait for this to", "complete...", 0);
  delay(1000);
  //Print out the data.
  
  //Sensor 1 Data Printout.
  PrintLCD("Hydrometer ","Sensor 1:" ,0);
  String s1d = "Dig_1 = ";
  s1d += d[0];
  String s1a = "Anlg_1 = ";
  s1a += a[0];
  delay(1000);
  PrintLCD("Digital Value:", s1d , 0);
  delay(1000);
  PrintLCD("Analoge Value:", s1a , 0);
  delay(1000);
  PrintLCD("Humidity and ","Temp. Sensor 1:" ,0);
  String s1h = "Humid_1: ";
  s1h += h[0];
  s1h += '%';
  String s1t = "Temp_1:  ";
  s1t += t[0];
  s1t += (char)223;
  PrintLCD(s1h, s1t, 0);
  delay(1000);
  
  //Printout for Sensor 2.
  PrintLCD("Hydrometer ","Sensor 2:" ,0);
  String s2d = "Dig_2 = ";
  s2d += d[1];
  String s2a = "Anlg_2 = ";
  s2a += a[1];
  delay(1000);
  PrintLCD("Digital Value:", s2d , 0);
  delay(1000);
  PrintLCD("Analoge Value:", s2a , 0);
  delay(1000);
  PrintLCD("Humidity and ","Temp. Sensor 2:" ,0);
  String s2h = "Humid_2: ";
  s2h += h[1];
  s2h += '%';
  String s2t = "Temp_2:  ";
  s2t += t[1];
  s2t += (char)223;
  PrintLCD(s2h, s2t, 0);
  delay(1000);
  
  PrintLCD(" *Please Note* ","",0);
  delay(500);
  PrintLCD("This data","will NOT be",0);
  delay(1000);
  PrintLCD("displayed every", "time.",0);
  delay(1000);
  PrintLCD("The Arduino will","be updating" ,0);
  delay(1000);
  PrintLCD("sensor info. in","the background." ,0);
  delay(2000);
  
  PrintLCD("     NEXT","     STEP" ,0);
  delay(500);
  PrintLCD("We will use","the collected" ,0);
  delay(1000);
  PrintLCD("data to activate","diffrent branches" ,0);
  delay(1000);
  PrintLCD("of the irrigatio. ","system." ,0);
  delay(2000);
  
  PrintLCD("For our demo,"   ,"each Relay reps.", 0);
  delay(1000);
  PrintLCD("a branch of the" ,"irrigation system", 0);
  delay(1000);
  PrintLCD("controlled by"   ,"solinoid valves", 0);
  delay(1000);
  PrintLCD("Please trigger"   ,"Sensor #3. to", 0);
  delay(1000);
  PrintLCD("see the reaction!","" , 0); 
  delay(2000);
  
  
  PrintLCD("   Hydrometer   ","   Sensor #3    ", 0);
  delay(1000);
  bool runPrg = true;
  int d3 = 0;
  int a3 = 0;
  while(runPrg == true){
    d3 = digitalRead(soilSensD_3);
    String s3d = "Dig_3 = ";
    s3d += d3;
    a3 = analogRead(soilSensA_3);
    String s3a = "Anlg_3 = ";
    s3a += a3;
    PrintLCD(s3d, s3a, 0);
    if(a3 < 750){
       runPrg = false;
    }
  }
  delay(500);
  relayCtrl(3, true);
  
  //PrintLCD("****************","****************" ,0);
  PrintLCD("You should see  ","that Relay #3 is" ,0);
  delay(1000);
  PrintLCD("now active. This","is because the" ,0);
  delay(1000);
  PrintLCD("sensor reading  ","had fallen below" ,0);
  delay(1000);
  PrintLCD("a set value of  ","750." ,0);
  delay(1000);
  PrintLCD("Sensor data can ","be used to turn." ,0);
  delay(1000);
  PrintLCD("water flow on or","off to different" ,0);
  delay(1000);
  PrintLCD("branches of a   ","field." ,0);
  delay(3000);
  PrintLCD("","" ,0);
  
}

void intPrg2(){

  while(1){
  
  getSensorData();
    
  //Convert digital sensor data into boolean logic.
  //WHEN:
  //  dW[i] = true -> the sensor is trigger ON.
  //  dW[i] = false -> the sensor is trigger OFF.
  //beacause...
  //
  //  d[i] = 1 -> the sensor if triggerd OFF.
  //  d[i] = 0 -> the sensor if triggerd ON.
  bool dW[4];
  for(int i = 0; i < 4; i++){
    dW[i] =! d[i];
  }
  
  //PrintLCD("****************","****************" ,0);
  
  //Soloniod Opens up first. This is important.
  //IF THE OTHER 2 RELAYS NEED TO BE USED
  //CHANGE THE 2 TO A 4 IN THE LOOP. THIS IS TO PROTECT
  //THE PUMP THAT IS NOT CONNECTED TO A SOLINOID ON THAT
  //BRANCH.
  bool pumpState = false;
  for(int i = 0; i < 2; i++){
    if(dW[i] == true){
      relayCtrl(i+1, true);
      pumpState = true;
      
      //PrintLCD("****************","****************" ,0);
      String rly = "Relay #";
      rly += i+1;
      rly += " is ON;";
      PrintLCD(rly,"Call for Pump",0);
      delay(500);
    }
  }
  
  //If any of the solinods are open, turn the pump on.
  if(pumpState == true){
    PrintLCD("Turning Pump:", "    ON     ",0);
    delay(500);
    relayCtrl(0, true);
  }
  
  
  //When all the Sensors are off, turn the pump off, then the solinods.
  int sensOff = 0;
  for(int i = 0; i < 4; i++){
    if(dW[i] == false){
      sensOff++;
    }
  }
  //If all 4 of the SENSORS (NOT RELAYS) are off, turn the pump off.
  if(sensOff == 4){
    PrintLCD("Pump has been","called OFF",0);
    relayCtrl(0, false);
    delay(500);
  }
  
  //Turn the Soloids off now.
   for(int i = 0; i < 4; i++){
    if(dW[i] == false){
      relayCtrl(i+1, false);
      String rly = "Relay #";
      rly += i+1;
      PrintLCD(rly,"is now OFF",0);
      delay(1000);
    }
  }


  }
}

void basicData(){
  
  //Prints out some basic data for a normal run. 
  String r1 = "A1:";
  r1 += a[0];
  r1 += "  ";
  r1 += "H1: ";
  r1 += int(h[0]);
  r1 += '%';
  String r2 = "A2:";
  r2 += a[1];
  r2 += "  ";
  r2 += "H2: ";
  r2 += int(h[1]);
  r2 += '%';
  PrintLCD(r1,r2,0);

}
