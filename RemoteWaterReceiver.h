/*
 * RemoteWaterThermometer
 * 
 * This library receives, decodes and transmits data of
 * remote water sensors KW9043.
 * 
 * Copyright 2013 by Petr Stehlik http://pstehlik.cz/
 *
 * Idea of the communication protocol and timing based on ...
 *
 * License: GPLv3. See license.txt
 */

#ifndef RemoteWaterReceiver_h
#define RemoteWaterReceiver_h

#include <Arduino.h>

#define RWR_TOLERANCE 80

typedef void (*RemoteWaterReceiverCallback)(byte &id, int &temp, byte &chan, boolean &batt, boolean &beep);

/**
 * Generic class for receiving and decoding 433MHz remote water thermometer sensor KW9043 (and similar ones).
 *
 * Hardware required for this library:
 * A 433MHz/434MHz SAW receiver, e.g. http://www.sparkfun.com/products/10532
 */
class RemoteWaterReceiver {
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
        static void init(short int interrupt, RemoteWaterReceiverCallback callback);

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
         * interruptHandler is called on every change in the input signal. If RemoteWaterReceiver::init is called
         * with interrupt <0, you have to call interruptHandler() yourself. (Or use InterruptChain)
         */
        static void interruptHandler();
     
    private:
        static bool isImpuls(int delta);
        static int readBits(byte start, byte count);
        static bool checkSum();
        static void decodeTemp();

        static short int interrupt;
        static RemoteWaterReceiverCallback callback;
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
