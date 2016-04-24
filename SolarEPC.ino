// Dave Davies @dodavies This code runs in my own board as a charge rate controller for use with a Rolec/Mainline EPC
// Huge credit to @openenergymonitor & other contributors without them this would not be possible, buy lots of boards from them :-)
// You can find full details on the project at openenergymonitor.org
// This code will work in an emontx2 and is expecting to see data from an emontx3 on id9
// My dedicated board has a 'parallelling' output which allows you to half the resistance seen on the EPC terminals
// The documentation suggests anything under 50 ohms will cause shutdown of the charger,the wiper resistance is about 40 ohms
// so using the two jumpers allows 20ohms which will shut the charger down but I found no jumpers works fine.
#include <JeeLib.h>
#define myNodeID 30 //node ID of rx, not really important for rx
#define network 212 //network group 212 for use in my setup you should change this so suit your own
#define RF_freq RF12_868MHZ  //The Freq setting of the onboard radio RF12B can be RF12_433MHZ, RF12_868MHZ in the UK legally.
#include <SPI.h> //spi library for analog devices digital pot control
const int slaveSelectPin = 6; // Slave Select Pin To talk to the AD5206 digital pot
typedef struct {
  int power1, power2, power3, power4, Vrms, temp;
} PayloadTX; //standard stuff in emontx, we only care about solar on 2/4
PayloadTX emontx;  // stand emon stuff
const int emonTx_NodeID = 9; //emonTx3 node ID default is 10 mine is 9
void setup() {
  rf12_initialize(myNodeID, RF_freq, network); //Initialize RFM12 Radio with settings as defined above
  delay(200); //wait for Radio to calm down a bit before firing into the main setup
  Serial.begin(9600); //serial comms speed
  Serial.println("Solar EPC Controller"); //serial banner
  Serial.println("@dodavies 2014"); //serial banner
  pinMode (slaveSelectPin, OUTPUT);
  SPI.begin();
}
// set up the digial pot via spi
void digitalPotWrite(int address, int value) {
  // take the SlaveSelect pin low to select the chip:
  digitalWrite(slaveSelectPin, LOW);
  //  send in the address and value to the AD5206 via SPI:
  SPI.transfer(address);
  SPI.transfer(value);
  // take the SlaveSelect pin high to de-select the AD5206 chip:
  digitalWrite(slaveSelectPin, HIGH);
}
void loop() {

  if (rf12_recvDone()) {
    if (rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0)
    {
      int node_id = (rf12_hdr & 0x1F); //extract nodeID from payload
      if (node_id == emonTx_NodeID)  { //check data is coming from node with the correct ID
        emontx = *(PayloadTX*) rf12_data; // Extract the data from the payload
        int power = emontx.power4 + emontx.power2; //create a place to store pv power
        int realvolts = emontx.Vrms / 100;
        Serial.print("SolarOutput - Watts: "); Serial.println(emontx.power4 + emontx.power2);
        Serial.println(" ");
        if (power < 1600) { // If Solar is less than 1600w set the pot to 0 ohms switching off charging
          digitalPotWrite(3, 0); // off
          digitalPotWrite(1, 0); // the parallelling jumper must be set in the board but this halves the value so needs adjustment to suit
          Serial.print("Insufficient Solar Charging Switched Off");
        }
        else if (power > 1600  < 1900 ) { // If Solar is 1600 - 1900W set the pot to 196 ohms 6A
          digitalPotWrite(3, 4); // 6A
          digitalPotWrite(1, 0); // the parallelling jumper must be set in the board but this halves the value so needs adjustment to suit
          Serial.print("Charging EV at 6A");
          Serial.println(" ");
          Serial.print("Actual Power From Solar in Amps: ");Serial.println(power / realvolts);
        }
        else if (power > 1901  < 2500 ) { // If Solar is  more than 2500W set the pot to 235 ohms 9A
          digitalPotWrite(3, 5); // Set the Pot to 235 ohms to charge at 9A
          digitalPotWrite(1, 0); // the parallelling jumper must be set in the board but this halves the value so needs adjustment to suit
          Serial.print("Charging at 9A");
          Serial.println(" ");
          Serial.print("Actual Solar Output in Amps: ");Serial.println(power / realvolts);

        }
         else if (power > 2500  < 3300) { // If Solar is  more than 2500W set the pot to 273 ohms 11A
          digitalPotWrite(3, 6); // Set the Pot to 273 ohms to charge at 11Amps
          digitalPotWrite(1, 0); // the parallelling jumper must be set in the board but this halves the value so needs adjustment to suit
          Serial.print("Charging at 11A");
          Serial.println(" ");
          Serial.print("Actual Solar Output in Amps: ");Serial.println(power / realvolts);

        }
        
        else
        {
          // set the pot value to 4 for 6A i.e if it is lower than 1600w, probably need more stuff but this is a start
          //4 = 196  ohms 6A zcw says it should be 191 ohms
          //5 = 235  ohms 9A zcw says it should be 237 ohms
          //6 = 273  ohms 11A zcw says it should be 267 ohms
          //8 = 349  ohms 16A zcw says it should be 348 ohms
          //18 = 729 ohms 32A zcw says it should be 732 ohms

          digitalPotWrite(3, 18); // 32A
          digitalPotWrite(1, 0); // the jumper must be set in the board but this halves the value so needs adjustment to suit
          Serial.print("Charging at 32A");
          Serial.println(" ");
          Serial.print("Actual Solar Output in Amps: ");Serial.println(power / realvolts);
        }
      }
    }
  }
}
