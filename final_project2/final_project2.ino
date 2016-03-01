#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
//network
#include <SPI.h>
#include <RF24.h>
RF24 radio(7, 8);
//network
LiquidCrystal_I2C lcd(0x27, 16, 2);
#include "EmonLib.h"             // Include Emon Library
EnergyMonitor emon1;             // Create an instance
int Relay4 = 6;
float sum = 0;
float unit;//show
float watt = 0.0;//show
float x = 1;
int Limit = 0;
//network
const uint64_t pipes[2] = { 0xe7e7e7e7e7LL, 0xc2c2c2c2c2LL};

char id[] = "  12";
String checkid = "";
String checklimit = "";
//network

#define DS1307_ADDRESS 0x68 // define DS1307 Address
byte zero = 0x00; //workaround for issue #527

void setup()
{  
  //network
  radio.begin();
  radio.setPayloadSize(32);
  radio.setChannel(0x60);
  radio.setDataRate(RF24_2MBPS);
  radio.setPALevel(RF24_PA_MIN);
  radio.setAutoAck(true);
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]); //num pipe , rang min
  radio.startListening();
  //network
  Serial.begin(9600);
  lcd.begin();
  Wire.begin();
  pinMode (Relay4, OUTPUT);
  emon1.voltage(1, 255, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current(2, 5.3);       // Current: input pin, calibration.
  setDateTime(); //MUST CONFIGURE IN FUNCTION

}

void loop()
{
  printDate(); // print Date and Time Function 
  emon1.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out
  //emon1.serialprint();           // Print out all variables (realpower, apparent power, Vrms, Irms, power factor)
  
  float realPower       = emon1.realPower;        //extract Real Power into variable
  float apparentPower   = emon1.apparentPower;    //extract Apparent Power into variable
  float powerFActor     = emon1.powerFactor;      //extract Power Factor into Variable
  float supplyVoltage   = emon1.Vrms;             //extract Vrms into Variable
  float Irms            = emon1.Irms;             //extract Irms into Variable
  
  watt = supplyVoltage*Irms; // พลังงานที่ใช้ ณ ขณะนั้นๆ
  sum = sum + watt; // ผลรวมพลังงานที่ใช้
  unit = (sum*(x/60))/1000; // ต่อ 1 นาที
  x++;
  //if (unit > Limit)
 // {
   //   digitalWrite(Relay4, LOW);
  
 // }else if(unit < Limit)
  //{
    //digitalWrite(Relay4, HIGH);
  //}
  lcd.setCursor(0, 0);
  //lcd.print("I = ");
  //lcd.print(Irms);         // Apparent power
  //lcd.print(" ");
  //lcd.print("V = ");
  //lcd.print(supplyVoltage);        // Apparent power
  //lcd.print(" ");
  lcd.print("Unit = ");
  lcd.print(unit);
  lcd.setCursor(0, 1);
  lcd.print("W = ");
  lcd.print (supplyVoltage*Irms);
  Serial.println(supplyVoltage);
  Serial.println(Irms);
  Serial.println(supplyVoltage*Irms);
  Serial.println(sum);
  Serial.println(unit);
  delay(1000);
}

void setDateTime()
{ // function for SetDate and Time in RTC
  byte second = 00; //0-59 // config initial value of variable second
  byte minute = 43; //0-59 // config initial value of variable minute
  byte hour = 13; //0-23 // config initial value of variable hour
  byte weekDay = 2; //1-7 // config initial value of variable weekday
  byte monthDay = 16; //1-31 // config initial value of variable day
  byte month = 11; //1-12 // config initial value of variable month
  byte year = 15; //0-99 // config initial value of variable year
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero); //stop Oscillator
  /* convert dec type to BCD type and Write to RTC */
  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(weekDay));
  Wire.write(decToBcd(monthDay));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.write(zero); //start
  Wire.endTransmission();
}
  byte decToBcd(byte val){
  // Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );
}
  byte bcdToDec(byte val) {
  // Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );
}
/* -------------- Function for Print Date and time ---------------- */
void printDate(){
  // Reset the register pointer
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS, 7);
  int second = bcdToDec(Wire.read());
  int minute = bcdToDec(Wire.read());
  int hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
  int weekDay = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
  int monthDay = bcdToDec(Wire.read());
  int month = bcdToDec(Wire.read());
  int year = bcdToDec(Wire.read());
  //print the date EG 3/1/11 23:59:59
  /*Serial.print(month);
  Serial.print("/");
  Serial.print(monthDay);
  Serial.print("/");
  Serial.print(year);
  Serial.print(" ");
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.println(second);
  */
  //network
  String string_id = "";
  String string_limit = "";
  char text[32] = "";
  int stoplimt = 0;
  if (radio.available())
    {
      char textread[32];
      radio.read(&textread, radio.getDynamicPayloadSize());
   
        for(int n = 0; n <= 32; n++){
          if(n <= 3){
          string_id += textread[n]; 
        }else if(n >=4 && n<=11){
          string_limit += textread[n];
        }
      }
      Serial.println(string_id);
      Serial.println(string_limit);
      checkid = string_id;
      checklimit = string_limit;
    }
  int check_limit = string_limit.toInt();
  int n = 2;
  if(checkid == "  12"){
    if(n > check_limit && check_limit != 0){
      digitalWrite(Relay4, LOW);
      Serial.println("LOW");
      }else{
      digitalWrite(Relay4, HIGH);
     }
     Serial.println("Yes");
  } 
  radio.stopListening();     
 // float watt = 13333.53;
  int wattInt = watt;
  float wattCal = watt - wattInt;
  int wattFloat = (int)(wattCal * 100);
  
 // float unit = 13333.53;
  int unitInt = unit;
  float unitCal = unit - unitInt;
  int unitFloat = (int)(unitCal * 100);
  
  
  sprintf(text,"%4s%6d.%2d%6d.%2d",id,unitInt,unitFloat,wattInt,wattFloat);
  Serial.println(text);
  if (!radio.write( &text, sizeof(text) )){
  }
 // radio.write(&text, sizeof(text));
  radio.startListening();  
  //network
  
  delay(1000);
}

