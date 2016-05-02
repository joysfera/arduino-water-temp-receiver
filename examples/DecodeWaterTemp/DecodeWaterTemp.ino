/*
 * Decode Remote Water Thermometer - KW9043
 *
 * Petr Stehlik in 2013-2016
 *
 * GPLv3
 */

#include <WaterTempReceiver.h>

void setup()
{
    Serial.begin(115200);
    WaterTempReceiver::init(0, printdata);  // interrupt 0 = pin D2
}

void loop()
{
}

void printdata(byte &id, int &temp, byte &chan, boolean &batt, boolean &beep)
{
    Serial.print("id = ");
    Serial.print(id);
    Serial.print(", temp = ");
    Serial.print(temp / 10.0f, 1);
    Serial.print(", chan = ");
    Serial.print(chan);
    Serial.print(", batt = ");
    Serial.print(batt ? "OK" : "LOW");
    Serial.print(", beep = ");
    Serial.println(beep ? "yes" : "no");
}
