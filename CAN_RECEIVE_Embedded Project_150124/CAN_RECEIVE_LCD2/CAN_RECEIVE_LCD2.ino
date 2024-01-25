//////////////////////////
//Receiver 15 Jan 2024
//////////////////////////

#include <SPI.h>                //Library for using SPI Communication
#include <mcp2515.h>            //Library for using CAN Communication
#include <LiquidCrystal_I2C.h>  //Library for using LCD display

LiquidCrystal_I2C lcd(0x3F,16,2);  // Define LCD's I2C address (0x3F) for a 16 chars and 2 line display
MCP2515 mcp2515(53);               // SPI CS Pin 53

#define temp_but 5
#define humi_but 6
#define fuel_but 7

struct can_frame canMsg; 
int DELAY_VAL = 1000;
double old_vol = 100;              //Initialize fuel volume value
int old_spd = 0;                   //Initialize speed value

void setup(){
  pinMode(temp_but, INPUT_PULLUP);
  pinMode(humi_but, INPUT_PULLUP);
  pinMode(fuel_but, INPUT_PULLUP);

  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  init_msg();

  SPI.begin();                          //Begins SPI communication
  Serial.begin(115200);                 //Begins Serial Communication at 115200 baudrate 
  mcp2515.reset();                          
  mcp2515.setBitrate(CAN_125KBPS);      //Sets CAN at speed 500KBPS and Clock 8MHz 
  mcp2515.setNormalMode();              //Sets CAN at normal mode
}

void loop(){
  int getTemp = digitalRead(temp_but);
  int getHumi = digitalRead(humi_but);
  int getFuel = digitalRead(fuel_but);
  int comb_but = getTemp<<2 | getHumi <<1 | getFuel;  //Combine bits of button {temp:humi:fuel}
      // Serial.print(getTemp);
      // Serial.print(getHumi);
      // Serial.println(getFuel);
      Serial.println(comb_but, BIN);

  can_trigger(comb_but);

  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK){ //Check for error
    get_ID();
    switch (comb_but){
    case 0: //0x111
      lcd.clear();
      lcd.setCursor(0,0);
        lcd.print("Tem:");
          get_temp();
        lcd.print(" Humi:"); 
          get_humi();   
      lcd.setCursor(0,1);
        lcd.print("Fuel:");
          get_fuel();
      break;
    case 6: //0x110
      lcd.clear();
      lcd.setCursor(0,0);
        lcd.print("Tem:");
          get_temp(); 
      break;
    case 5: //0x101
      lcd.clear();
      lcd.setCursor(0,0);
        lcd.print("Humi:"); 
          get_humi(); 
      break;
    case 4: //0x100
      lcd.clear();
      lcd.setCursor(0,0);
        lcd.print("Tem:");
          get_temp();
      lcd.setCursor(0,1);
        lcd.print("Humi:"); 
          get_humi();   
      break;
    case 3: //0x011
      lcd.clear();  
      lcd.setCursor(0,0);
        lcd.print("Fuel:");
          get_fuel();
      break;
    case 2: //0x010
      lcd.clear();
      lcd.setCursor(0,0);
        lcd.print("Tem:");
          get_temp();
      lcd.setCursor(0,1);
        lcd.print("Fuel:");
          get_fuel();
      break;
    case 1: //0x001
      lcd.clear();
      lcd.setCursor(0,0);
        lcd.print("Humi:"); 
          get_humi();   
      lcd.setCursor(0,1);
        lcd.print("Fuel:");
          get_fuel();
      break;
    default:
        lcd.setCursor(0,1);
        lcd.print("Embedded System");
      break;
    }
  delay(DELAY_VAL);
  }

  else{lcd.clear(); lcd.print("Waiting..."); delay(DELAY_VAL);}
}

void init_msg(){
  lcd.clear();
  lcd.setCursor(0,0);                //Display Welcome Message
  lcd.print("SMJE4423 ES");
  lcd.setCursor(4,1);
  lcd.print("CAN Project");
  delay(3000);
}

void get_ID(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("ID:");
  lcd.print(canMsg.can_id); // print ID
}

void get_temp(){    
    int t = canMsg.data[1];        
    lcd.print(t);
}

void get_humi(){
    int h = canMsg.data[0];           
    lcd.print(h);
}

void get_fuel(){         
  int w = canMsg.data[2];
    double y = (w+259.8)/115.44;
    double x = 2.718;
    double z = pow(x, y);
  
  lcd.print(z);
}

void can_trigger(int trigger){
  canMsg.can_id  = 0x00F;           //CAN id as 0x001
  canMsg.can_dlc = 8;               //CAN data length as 8
  canMsg.data[0] = trigger;         //send trigger signal
  canMsg.data[1] = 0x00;            
  canMsg.data[2] = 0x00;            
  canMsg.data[3] = 0x00;
  canMsg.data[4] = 0x00;
  canMsg.data[5] = 0x00;
  canMsg.data[6] = 0x00;
  canMsg.data[7] = 0x00;
  
  for(int i=0; i <8; i++){
    Serial.print(canMsg.data[i]); Serial.print(" ");}
  Serial.println();

  mcp2515.sendMessage(&canMsg);     //Sends the CAN message
  Serial.println();

  delay(1000);
}