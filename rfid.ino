// Based on code by Ben Eckel and Riley Porter
// Updated to include logging to SD card

#include <SdFat.h>
#include <SdFatUtil.h> 
#include <ctype.h>

#define RFID_ENABLE 2    // digital I/0 2 <--> /ENABLE
#define TAG_LEN     10   // max length of RFID tag
#define START_BYTE  0x0A // starting byte of an RFID tag
#define STOP_BYTE   0x0D // ending byte of an RFID tag
#define CS_PIN      10   // the chip select pin for SD communication
#define POW_PIN     8    // power for logging shield


// Variables for SD library
Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;

char tag[TAG_LEN];
char name[] = "tags.log";

void setup() {
    Serial.begin(2400);          // Baud rate of Parallax reader is 2400
    pinMode(RFID_ENABLE,OUTPUT);
    pinMode(CS_PIN,OUTPUT);
    card.init();
    volume.init(card);
    root.openRoot(volume);
}

void loop() {
    enableRFID();
    getRFIDTag();
    if(validCode()) {
        disableRFID();
        writeCode();
        delay(2000);   // delay in milliseconds to move away from reader
    } else {
        disableRFID();
        Serial.println("Noisy input! Gah!");
    }
    Serial.flush();
    clearCode();
}

void clearCode() {
    for(int i=0; i<TAG_LEN; i++) {
        tag[i] = 0;
    }
}

void writeCode() {
    Serial.print("TAG:");  
    for(int i=0; i<TAG_LEN; i++) {
        if (i == 9) Serial.println(tag[i]);
        else Serial.print(tag[i]);  
    }
    file.open(root, name, O_CREAT | O_APPEND | O_WRITE);
    file.print(tag);
    file.close();
}

void enableRFID() {
   digitalWrite(RFID_ENABLE, LOW);    
}
 
void disableRFID() {
   digitalWrite(RFID_ENABLE, HIGH);  
}

void getRFIDTag() {
    byte next_byte; 
    while(Serial.available() <= 0) {}
    if((next_byte = Serial.read()) == START_BYTE) {      
        byte bytesread = 0; 
        while(bytesread < CODE_LEN) {
            if(Serial.available() > 0) { //wait for the next byte
                if((next_byte = Serial.read()) == STOP_BYTE) break;       
                tag[bytesread++] = next_byte;                   
            }
        }                
    }    
}

boolean validCode() {
    byte next_byte; 
    int count = 0;
    while (Serial.available() < 2) {  //there is already a STOP_BYTE in buffer
        delay(1); //probably not a very pure millisecond
        if(count++ > 200) return false;
    }
    Serial.read(); //throw away extra STOP_BYTE
    if ((next_byte = Serial.read()) == START_BYTE) {  
        byte bytes_read = 0; 
        while (bytes_read < CODE_LEN) {
            if (Serial.available() > 0) { //wait for the next byte      
                if ((next_byte = Serial.read()) == STOP_BYTE) break;
                if (tag[bytes_read++] != next_byte) return false;                     
            }
        }                
    }
    return true;   
}
