
// Dave Davies @dodavies This code runs on a dedicated pcb board as a charge rate controller for use with a Rolec/Mainline EPC
// Huge credit to openenergymonitor.org & other contributors without them this would not be possible, buy lots of boards from them :-)

//  This code was mostly copied from simple rfb12b demo by the guys at openenergymonitor.org as part of the openenergymonitor.org project
//  Authors: Glyn Hudson & Trystan Lea 
//  Builds upon JCW JeeLabs RF12 library and Arduino 
//  Licence: GNU GPL V3

// This code will work in an emontx2 and is expecting to see data from an emon-tx3 on id9
// My dedicated board has a 'parallelling' output which allows you to half the resistance seen on the EPC terminals
// The documentation suggests anything under 50 ohms will cause shutdown of the charger,the wiper resistance is about 40 ohms
// so using the two jumpers allows 20ohms which will shut the charger down. the AD5206 has 6 pots



#include <JeeLib.h>
#define myNodeID 30 //node ID of rx
#define network 212 //network group 212 for use in my setup you should change this so suit your own setup
#define RF_freq RF12_868MHZ  //The Freq setting of the onboard radio RF12B can be RF12_433MHZ, RF12_868MHZ in the UK legally.
#include <SPI.h> //spi library for analog devices digital pot control
const int slaveSelectPin = 6; // Slave select pin to talk to the AD5206 digital pot
typedef struct { int power1, power2, power3, power4, Vrms, temp; } PayloadTX; //standard stuff in emontx, we only care about solar on 2/4
PayloadTX emontx;  // standard emontx stuff
const int emonTx_NodeID=9; //emonTx3 node ID default is 10 mine is 9 
void setup() {
rf12_initialize(myNodeID,RF_freq,network);   //Initialize RFM12 Radio with settings as defined above
delay(200); //wait for radio to calm down a bit before firing into the main setup
Serial.begin(9600); //serial comms speed
Serial.println("PV EPC Charge Rate Controller"); //serial banner
Serial.println("@dodavies 2014"); //serial banner
Serial.print("Freq: "); //serial banner
Serial.print("868Mhz"); //serial banner
Serial.print(" Network: "); //serial banner
Serial.println(network); //serial banner
 pinMode (slaveSelectPin, OUTPUT);
 SPI.begin();
}
// set up the digial pot via spi
void digitalPotWrite(int address, int value) {
  // take the SlaveSelect pin low to select the chip:
  digitalWrite(slaveSelectPin,LOW);
  //  send in the address and value to the AD5206 via SPI:
  SPI.transfer(address);
  SPI.transfer(value);
  // take the SlaveSelect pin high to de-select the AD5206 chip:
  digitalWrite(slaveSelectPin,HIGH); 
}  
void loop(){
if (rf12_recvDone()){    
  if (rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0)
  {
    int node_id = (rf12_hdr & 0x1F); //extract nodeID from payload     
    if (node_id == emonTx_NodeID)  { //check data is coming from node with the correct ID
        emontx=*(PayloadTX*) rf12_data;  // Extract the data from the payload
      // const int LEDpin = 9;
      // digitalWrite(LEDpin, HIGH); delay(2); digitalWrite(LEDpin, LOW);
       Serial.print("SolarOutput - Watts: "); Serial.println(emontx.power4+emontx.power2);
       Serial.print("GridVoltage: "); Serial.println(emontx.Vrms/100);
       Serial.println(" "); 
       if (emontx.power2+emontx.power4 > 2000){ // If Solar is over 2000w set the pot to 348 ohms
       digitalPotWrite(3,8);
}else
{ 
 // set the pot value to 4 for 6A i.e if it is lower than 2000w, probably need more stuff but this is a start
 //4 = 196  ohms 6A zcw says it should be 191 ohms
 //5 = 235  ohms 9A zcw says it should be 237 ohms
 //6 = 273  ohms 11A zcw says it should be 267 ohms
 //8 = 349  ohms 16A zcw says it should be 348 ohms
 //18 = 729 ohms 32A zcw says it should be 732 ohms
 
digitalPotWrite(3,0);
digitalPotWrite(1,0);
}
}
}       
}
}
