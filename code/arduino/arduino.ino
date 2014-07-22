#include <SoftwareSerial.h>
#include "constants.h"
#include "butterworth.h"

#define SRF_ADDRESS      0x01  // Address of the SFR01
#define GETSOFT          0x5D  // Byte to tell SRF01 we wish to read software version
#define GETRANGE         0x54  // Byte used to get range from SRF01
#define GETSTATUS        0x5F  // Byte used to get the status of the transducer


#define LUX_BQ 188
#define SRF_BQ 300

#define SRF_Serial Serial1


int luxpin = A0;
int srf_pin = 5;

bw_state lux_bw = {0, 0};
bw_state srf_bw = {0, 0};


SoftwareSerial srf01 = SoftwareSerial(srf_pin, srf_pin);


void setup() {
    Serial.begin(115200);
    SRF_Serial.begin(9600);
    srf01.begin(9600);

    // while (!Serial.available()) {}
    // Serial.println("serial online");

    // byte softVer;
    // SRF_Cmd(GETSOFT);


    // while (SRF_Serial.available() < 1) {}
    // softVer = SRF_Serial.read();

    // Serial.print("Version: ");
    // Serial.println(softVer, DEC);

}


/* Assumes frames are always faster than 70000us. (which they are)
 * If they are not it will stop to work x min after power on.
 */

void loop() {
    unsigned long t_start = micros();
    unsigned long expect = t_start;

    for (;;) {
        expect += us_per_frame;
        loop_real();
        unsigned long now = micros();
        if (now < expect) {
            delayMicroseconds(expect - now);
        } else {
            Serial.println(">> WARNING: FRAME UNDERRUN");
        }
    }
}

void loop_real() {
    char outbuf[32];
    if (Serial.available()) {

    }
    SRF_Cmd(GETRANGE);
    unsigned lux = butterworth(&lux_bw, LUX_BQ, analogRead(luxpin));

    while (SRF_Serial.available() < 2) {}
    byte hByte = SRF_Serial.read();
    byte lByte = SRF_Serial.read();
    unsigned range = (hByte<<8) | lByte;
    unsigned rangeb = butterworth(&srf_bw, SRF_BQ, range);

    // sprintf(outbuf, "lux: %4d   range: %5d\r", lux, rangeb);
    // sprintf(outbuf, "%4u %5u %5u\r\n", lux, range, rangeb);
    // Serial.write(outbuf);
    // Serial.write("SYN");
    // Serial.write(lux);
    // Serial.write(rangeb);
    Serial.write("SY");
    Serial.write(lowByte(lux));
    Serial.write(highByte(lux));
    Serial.write(lowByte(range));
    Serial.write(highByte(range));
    Serial.write(lowByte(lux) ^ highByte(lux) ^ lowByte(range) ^ highByte(range));

}


void SRF_Cmd(byte cmd){               // Function to send commands to the SRF01
    pinMode(srf_pin, OUTPUT);
    digitalWrite(srf_pin, LOW);                        // Send a 2ms break to begin communications with the SRF01
    delay(2);
    digitalWrite(srf_pin, HIGH);
    delay(1);
    srf01.write(SRF_ADDRESS);                               // Send the address of the SRF01
    srf01.write(cmd);                                   // Send commnd byte to SRF01
    pinMode(srf_pin, INPUT);

    // while (SRF_Serial.read() < 0) {}
    int availbleJunk = SRF_Serial.available();               // As RX and TX are the same pin it will have recieved the data we just sent out, as we dont want this we read it back and ignore it as junk before waiting for useful data to arrive

    for(int x = 0;  x < availbleJunk; x++) {
        SRF_Serial.read();
    }
}
