#ifndef Thread_h
#define Thread_h

#include <Arduino.h>

#define THREAD_USER_STAYON 0
#define THREAD_USER_STAYOFF 1
#define THREAD_USER_ONFOR 2

class Thread : public Thread
{
    public:

        // pinNumber is where on the arduino it's hooked up.
        // use a PWM out for pinNumber
        // leadTime is the number of milliseconds it takes the thread to heat up
        // power is the 0-255 power level this thread needs
        Thread(int pinNumber, int leadTime, int power);
        void update();

        // turn on just long enough for color change, then turn off
        void flash();

        // turn on for a certain amount of time, then turn off
        void onFor(long t);

        // turn on and keep it on (tries to be efficient)
        void on();

        // turn off
        void off();
    private:
        // actually send power to the thread
        void _on();

        // actually turn off power to the thread
        void _off();

        // thread specific settings
        int _pinNumber, _power, _leadTime;

        // timestamps of the last time the power was turned on or off
        long _powerLastOnTime, _powerLastOffTime;
        long 

        // number of milliseconds we want the thread to stay on for
        // and then we want to turn it off
        long _onForTime;

        // the last thing the user asked us to do, one of the
        // THREAD_USER_* values
        int _userRequest;
        // when the user made the request
        long _userRequestTime;
}

#endif
