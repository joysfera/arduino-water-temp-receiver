/*
 * Water Temperature Receiver
 * 
 * This library receives and decodes data sent by
 * remote water sensor KW9043 and compatible ones.
 *
 * Copyright 2013 by Petr Stehlik  http://pstehlik.cz/
 *
 * Idea of the communication protocol and timing based on
 *   reverse-engineering of Abdullah Tahiri's RemoteTransmitter
 *
 * License: GPLv3. See LICENSE.md
 */

#include <WaterTempReceiver.h>

short int WaterTempReceiver::interrupt;
WaterTempReceiverCallback WaterTempReceiver::callback;
byte WaterTempReceiver::interruptPin;
bool WaterTempReceiver::enabled;
unsigned long WaterTempReceiver::lastChange;
bool WaterTempReceiver::preamble;
bool WaterTempReceiver::space;
byte WaterTempReceiver::bits;
bool WaterTempReceiver::arr[28];
byte WaterTempReceiver::last_id;
byte WaterTempReceiver::last_chan;
int WaterTempReceiver::last_temp;
unsigned long WaterTempReceiver::last_milistamp;

void WaterTempReceiver::init(short int _interrupt, WaterTempReceiverCallback _callback)
{
    interrupt = _interrupt;
    callback = _callback;

    interruptPin = (interrupt == 0) ? 2 : 3;  // TODO: find a conversion function for this
    pinMode(interruptPin, INPUT_PULLUP);

    enable();
    if (interrupt >= 0)
        attachInterrupt(interrupt, interruptHandler, CHANGE);
}

void WaterTempReceiver::deinit()
{
    disable();
    if (interrupt >= 0)
        detachInterrupt(interrupt);
}

void WaterTempReceiver::enable()
{
    lastChange = micros();
    bits = 0;
    preamble = space = false;
    enabled = true;
}

void WaterTempReceiver::disable()
{
    enabled = false;
}

bool WaterTempReceiver::isImpuls(int duration)
{
    return (abs(duration) <= RWR_TOLERANCE);
}

void WaterTempReceiver::interruptHandler()
{
    if (!enabled) return;

    unsigned long currentTime = micros();
    unsigned duration = currentTime - lastChange;
    lastChange = currentTime;
    boolean state = digitalRead(interruptPin);
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
                    decodeTemp();
                    bits = 0;
                    preamble = false;
                }
            }
            else {
                preamble = false;
            }
        }
        space = false;
    }
}

int WaterTempReceiver::readBits(byte start, byte count)
{
    int val = 0;
    for(byte i = start; i < start + count; i++) {
        val <<= 1;
        val |= arr[i];
    }
    return val;
}

bool WaterTempReceiver::checkSum()
{
    byte checksum = readBits(0, 4);
    byte chk = 0;
    for(byte i=1; i<7; i++)
        chk += readBits(i*4, 4);
    chk--;
    return (checksum == (chk & 0x0f));
}

void WaterTempReceiver::decodeTemp()
{
// check control sum
    if (!checkSum()) {
        // Serial.println("Checksum failed.");
        return;
    }

// decode to temporary variables and compare with internal struct
    byte id = readBits(4, 8);
    int temp = readBits(12, 12);
    byte chan = readBits(24, 2);
    boolean batt = arr[26];
    boolean beep = arr[27];
    
// if it's the same data and last timestamp is less than 1 second ago then ignore it
    if ((millis() - last_milistamp) < 1000 && id == last_id && temp == last_temp && chan == last_chan) {
        // Serial.println("Received same packet again");
        return;
    }

// store temporary variables to internal struct and mark it with current timestamp (millis)
    last_id = id;
    last_temp = temp;
    last_chan = chan;
    last_milistamp = millis();

// call user callback with data
    (callback)(id, temp, chan, batt, beep);
}
