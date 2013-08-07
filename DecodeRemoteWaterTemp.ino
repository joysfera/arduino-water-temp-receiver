/*
 * Decode Remote Water Thermometer - simple Sencor thermo
 *
 * Petr Stehlik in 2013
 * 
 * based on RemoteTransmitter by ...
 *
 * GPLv3
 */

bool enabled;
unsigned long lastChange;
boolean preamble;
boolean space;
byte bits;
boolean arr[28];

byte last_id;
byte last_chan;
int last_temp;
unsigned long last_milistamp;

typedef void (*SencorReceiverCallback)(byte &id, int &temp, byte &chan, boolean &batt, boolean &beep);
SencorReceiverCallback callback;

#define TOLERANCE 50

void setup()
{
    Serial.begin(115200);
    callback = printdata;
    init1();
}

void loop()
{
}

void init1()
{
    attachInterrupt(0, handler, CHANGE);
    enable1();
}

void deinit1()
{
    disable1();
    detachInterrupt(0);
}

void enable1()
{
    lastChange = micros();
    bits = 0;
    preamble = space = false;
    enabled = true;
}

void disable1()
{
    enabled = false;
}

void printdata(byte &id, int &temp, byte &chan, boolean &batt, boolean &beep)
{
    Serial.print("id = ");
    Serial.print(id);
    Serial.print(", temp = ");
    Serial.print(temp/10);
    Serial.print(".");
    Serial.print(temp%10);
    Serial.print(", chan = ");
    Serial.println(chan);
}

bool isImpuls(int duration)
{
    return (abs(duration) <= TOLERANCE);
}

void handler()
{
    if (!enabled) return;

    unsigned long currentTime = micros();
    unsigned duration = currentTime - lastChange;
    lastChange = currentTime;
    boolean state = digitalRead(2);
    if (!state) {     // goes from HIGH to LOW
        space = isImpuls(duration - 500);
    }
    else if (space) { // goes from LOW to HIGH
        if (!preamble) {
            preamble = isImpuls(duration - 9500);
            bits = 0;
        }
        else {
            boolean low = isImpuls(duration - 2000);
            boolean high = !low && isImpuls(duration - 4500);
            if (low != high) {
                arr[bits++] = high;
                if (bits == 28) {
                    // all bits are received so disable receiving to not run into recursive deadloop
                    disable1();
                    // decode data and if it's fresh new then call user callback
                    decode_temp();
                    // reset receiving state and re-enable receiving
                    enable1();
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
// check control sum
    if (!check_sum())
        return;

// decode to temporary variables and compare with internal struct
    byte id = readbits(4, 8);
    int temp = readbits(12, 12);
    byte chan = readbits(24, 2);
    boolean batt = arr[26];
    boolean beep = arr[27];
    
// if it's the same data and last timestamp is less than 1 second ago then ignore it
    if ((millis() - last_milistamp) < 1000 && id == last_id && temp == last_temp && chan == last_chan)
        return;

// store temporary variables to internal struct and mark it with current timestamp (millis)
    last_id = id;
    last_temp = temp;
    last_chan = chan;
    last_milistamp = millis();

// 4) call user callback with data
    (callback)(id, temp, chan, batt, beep);
}
