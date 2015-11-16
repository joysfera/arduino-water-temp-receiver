/*
 * Decode Remote Water Thermometer - KW9043
 *
 * Petr Stehlik in 2013
 * 
 * GPLv3
 */

#include <WaterTempReceiver.h>

void setup()
{
    Serial.begin(115200);
    WaterTempReceiver::init(0, printdata);
}

void loop()
{
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
