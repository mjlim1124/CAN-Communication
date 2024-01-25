//////////////////////////
//Transceiver
//////////////////////////

#include <SPI.h>          //Library for using SPI Communication
#include <mcp2515.h>      //Library for using CAN Communication
#include <DHT.h>          //Library for using DHT sensor 

#define DHTPIN A0         //DHT signal
#define POWER_PIN  3      //Water Sensor power
#define SIGNAL_PIN A1     //Water Sensor signal
#define pot A2

int value = 0; // variable to store the sensor value
int level = 0; // variable to store the water level

struct can_frame canMsg1;
MCP2515 mcp2515(10);

DHT dht(DHTPIN, DHT22);     //initilize object dht for class DHT with DHT pin with STM32 and DHT type as DHT11

void setup(){
  canMsg1.can_id  = 0x036;
  canMsg1.can_dlc = 8;
  canMsg1.data[0] = 0x00;
  canMsg1.data[1] = 0x00;
  canMsg1.data[2] = 0x00;
  canMsg1.data[3] = 0x00;
  canMsg1.data[4] = 0x00;
  canMsg1.data[5] = 0x00;
  canMsg1.data[6] = 0x00;
  canMsg1.data[7] = 0x00;


  while (!Serial);
  Serial.begin(115200);
  SPI.begin();               //Begins SPI communication
  dht.begin();               //Begins to read temperature & humidity sesnor value
  mcp2515.reset();
  //mcp2515.setBitrate(CAN_125KBPS,MCP_8MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz
  mcp2515.setBitrate(CAN_125KBPS); //Sets CAN at speed 500KBPS and Clock 8MHz

  mcp2515.setNormalMode();

  pinMode(SIGNAL_PIN, INPUT);
  pinMode(POWER_PIN, OUTPUT);   // configure D7 pin as an OUTPUT
  digitalWrite(POWER_PIN, LOW); // turn the sensor OFF

  pinMode(pot, INPUT);
}


void loop(){
  int trig = read_trigger();
  send_request(trig);
  delay(2000);
}

int read_trigger(){
  int n = 0;
  while(!n){
    if (mcp2515.readMessage(&canMsg1) == MCP2515::ERROR_OK){
      int trigger_signal = canMsg1.data[0];
      if(trigger_signal == 7){
        Serial.println("No Trigger");}
      else{
        Serial.println(trigger_signal);}
    n = 1;
    return trigger_signal;
    }
    else{Serial.println("Waiting..."); n = 0; delay(2000);}
  }
}

void send_request(int trigger){
  //clear_data();
  
  switch (trigger){
    case 0:
      canMsg1.data[0] = dht.readHumidity();        
      canMsg1.data[1] = dht.readTemperature();            
      canMsg1.data[2] = fuel(); 
      break;
    case 6:
      canMsg1.data[1] = dht.readTemperature();  
      break;
    case 5:
      canMsg1.data[0] = dht.readHumidity(); 
      break;
    case 4:
      canMsg1.data[0] = dht.readHumidity();                  
      canMsg1.data[1] = dht.readTemperature();  
      break;
    case 3:
      canMsg1.data[2] = fuel(); 
      break;
    case 2:             
      canMsg1.data[1] = dht.readTemperature();               
      canMsg1.data[2] = fuel(); 
      break;
    case 1:
      canMsg1.data[0] = dht.readHumidity();  
      canMsg1.data[2] = fuel();
      break;
    default:
      break;
    }
  for(int i=0; i <8; i++){
  Serial.print(canMsg1.data[i]); Serial.print(" ");}
  Serial.println();

  mcp2515.sendMessage(&canMsg1);     //Sends the CAN message
  Serial.println();

  delay(1000);
}

int fuel(){
  digitalWrite(POWER_PIN, HIGH);  // turn the sensor ON
  delay(10);                      // wait 10 milliseconds
  value = analogRead(SIGNAL_PIN); // read the analog value from sensor
  digitalWrite(POWER_PIN, LOW);   // turn the sensor OFF

  return(value);
}

void get_ID(){
  Serial.print("ID: ");
  Serial.print(canMsg1.can_id); // print Trigger
}

void clear_data(){
  canMsg1.can_id  = 0x036;
  canMsg1.can_dlc = 8;
  canMsg1.data[0] = 0x00;
  canMsg1.data[1] = 0x00;
  canMsg1.data[2] = 0x00;
  canMsg1.data[3] = 0x00;
  canMsg1.data[4] = 0x00;
  canMsg1.data[5] = 0x00;
  canMsg1.data[6] = 0x00;
  canMsg1.data[7] = 0x00;
}