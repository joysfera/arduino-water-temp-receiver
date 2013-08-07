/*
 * Decode Remote Water Thermometer - simple Sencor thermo
 *
 * Petr Stehlik in 2013
 * 
 * based on RemoteTransmitter by ...
 *
 * GPLv3
 */

bool enabled = false;
unsigned long lastChange = 0;
boolean preamble = false;
boolean space = false;
byte bits = 0;
boolean arr[28];

#define TOLERANCE 50

void setup() {
    Serial.begin(115200);
    attachInterrupt(0, handler, CHANGE);
    bits = 0;
    enabled = true;
}

void loop() {
    if (bits == 28) {
        decode_temp();
        bits = 0;
        enabled = true;
    }
}

int delta(int dist) { return abs(dist); }

bool isImpuls(int duration, unsigned int length)
{
    return (delta(duration - length) <= TOLERANCE);
}

void handler()
{
    if (!enabled) return;

    unsigned long currentTime = micros();
    unsigned duration = currentTime - lastChange;
    lastChange = currentTime;
    boolean state = digitalRead(2);
    if (!state) {     // goes from HIGH to LOW
        space = isImpuls(duration, 500);
    }
    else if (space) { // goes from LOW to HIGH
        if (!preamble) {
            preamble = isImpuls(duration, 9500);
            bits = 0;
        }
        else {
            boolean low = isImpuls(duration, 2000);
            boolean high = !low && isImpuls(duration, 4500);
            if (low != high) {
                arr[bits++] = high;
                if (bits == 28) {
                    // 0) all bits are received so disable decoding to not run into recursive deadloop
                    enabled = false;

                    // 1) decode to temporary variables and compare with internal struct

                    // 2) if it's the same data and current millis() < timestamp of internal struct + 1 second then go to 5

                    // 3) store temporary variables to internal struct and mark it with current timestamp (millis)

                    // 4) call user callback with data

                    // 5) reset variables and re-enable decoding
                    preamble = false;
                    // bits = 0;
                    // enabled = true;
                }
            }
            else {
                preamble = false;
            }
        }
        space = false;
    }
}

int readbits(byte start, byte count)
{
    int val = 0;
    for(byte i = start; i < start + count; i++) {
        val <<= 1;
        val |= arr[i];
    }
    return val;
}

boolean check_sum()
{
    byte checksum = readbits(0, 4);
    byte chk = 0;
    for(byte i=1; i<7; i++)
        chk += readbits(i*4, 4);
    chk--;
    return (checksum == (chk & 0x0f));
}

void decode_temp()
{
    if (!check_sum()) {
        Serial.println("Checksum failed.");
        return;
    }

    byte id = readbits(4, 8);
    int temp = readbits(12, 12);
    byte chan = readbits(24, 2);
    boolean batt = arr[26];
    boolean beep = arr[27];
    
    Serial.print("id = ");
    Serial.print(id);
    Serial.print(", temp = ");
    Serial.print(temp/10);
    Serial.print(".");
    Serial.print(temp%10);
    Serial.print(", chan = ");
    Serial.println(chan);
}
