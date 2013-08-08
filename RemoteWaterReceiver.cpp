/*
 * RemoteWaterThermometer
 * 
 * This library receives, decodes and transmits data of
 * remote water sensors KW9043.
 *
 * Copyright 2013 by Petr Stehlik  http://pstehlik.cz/
 *
 * Idea of the communication protocol and timing based on ...
 *
 * License: GPLv3. See license.txt
 */

#include <RemoteWaterReceiver.h>

short int RemoteWaterReceiver::interrupt;
RemoteWaterReceiverCallback RemoteWaterReceiver::callback;
byte RemoteWaterReceiver::interruptPin;
bool RemoteWaterReceiver::enabled;
unsigned long RemoteWaterReceiver::lastChange;
bool RemoteWaterReceiver::preamble;
bool RemoteWaterReceiver::space;
byte RemoteWaterReceiver::bits;
bool RemoteWaterReceiver::arr[28];
byte RemoteWaterReceiver::last_id;
byte RemoteWaterReceiver::last_chan;
int RemoteWaterReceiver::last_temp;
unsigned long RemoteWaterReceiver::last_milistamp;

void RemoteWaterReceiver::init(short int _interrupt, RemoteWaterReceiverCallback _callback)
{
    interrupt = _interrupt;
    callback = _callback;

    interruptPin = (interrupt == 0) ? 2 : 3;  // find a conversion function for this
    pinMode(interruptPin, INPUT_PULLUP);

    enable();
    if (interrupt >= 0)
        attachInterrupt(interrupt, interruptHandler, CHANGE);
}

void RemoteWaterReceiver::deinit()
{
    disable();
    if (interrupt >= 0)
        detachInterrupt(interrupt);
}

void RemoteWaterReceiver::enable()
{
    lastChange = micros();
    bits = 0;
    preamble = space = false;
    enabled = true;
}

void RemoteWaterReceiver::disable()
{
    enabled = false;
}

bool RemoteWaterReceiver::isImpuls(int duration)
{
    return (abs(duration) <= RWR_TOLERANCE);
}

void RemoteWaterReceiver::interruptHandler()
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

int RemoteWaterReceiver::readBits(byte start, byte count)
{
    int val = 0;
    for(byte i = start; i < start + count; i++) {
        val <<= 1;
        val |= arr[i];
    }
    return val;
}

bool RemoteWaterReceiver::checkSum()
{
    byte checksum = readBits(0, 4);
    byte chk = 0;
    for(byte i=1; i<7; i++)
        chk += readBits(i*4, 4);
    chk--;
    return (checksum == (chk & 0x0f));
}

void RemoteWaterReceiver::decodeTemp()
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
