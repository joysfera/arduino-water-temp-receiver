/*
 * Water Temperature Receiver
 * 
 * This library receives and decodes data sent by
 * remote water sensor KW9043 and compatible ones.
 * 
 * Copyright 2013 by Petr Stehlik http://pstehlik.cz/
 *
 * Idea of the communication protocol and timing based on
 *   reverse-engineering of Abdullah Tahiri's RemoteTransmitter
 *
 * License: GPLv3. See LICENSE.md
 */

#ifndef WaterTempReceiver_h
#define WaterTempReceiver_h

#include <Arduino.h>

#define RWR_TOLERANCE 200

typedef void (*WaterTempReceiverCallback)(byte &id, int &temp, byte &chan, boolean &batt, boolean &beep);

/**
 * Generic class for receiving and decoding 433MHz remote water thermometer sensor KW9043 (and similar ones).
 *
 * Hardware required for this library:
 * A simple and cheap 433MHz receiver from Ebay etc.
 */
class WaterTempReceiver {
    public:
        /**
         * Initializes the receiver. When a valid data package has been received, the callback is called.
         *
         * If interrupt >= 0, init will register pin <interrupt> to this library.
         * If interrupt < 0, no interrupt is registered. In that case, you have to call interruptHandler()
         * yourself whenever the output of the receiver changes, or you can use InterruptChain.
         *
         * @param interrupt     The interrupt as is used by Arduino's attachInterrupt function. See attachInterrupt for details.
                                If < 0, you must call interruptHandler() yourself.
         * @param callback      Pointer to a callback function
         *
         */
        static void init(short int interrupt, WaterTempReceiverCallback callback);

        /**
        * Deinitializes the receiver and removes interrupt handler.
        */
        static void deinit();

        /**
        * Enable decoding. No need to call enable() after init().
        */
        static void enable();

        /**
        * Disable decoding. You can re-enable decoding by calling enable();
        */
        static void disable();

        /**
         * interruptHandler is called on every change in the input signal. If WaterTempReceiver::init is called
         * with interrupt <0, you have to call interruptHandler() yourself. (Or use InterruptChain)
         */
        static void interruptHandler();
     
    private:
        static bool isImpuls(int delta);
        static int readBits(byte start, byte count);
        static bool checkSum();
        static void decodeTemp();

        static short int interrupt;
        static WaterTempReceiverCallback callback;
        static byte interruptPin;
        static bool enabled;
        static unsigned long lastChange;
        static bool preamble;
        static bool space;
        static byte bits;
        static bool arr[28];

        static byte last_id;
        static byte last_chan;
        static int last_temp;
        static unsigned long last_milistamp;
};

#endif
