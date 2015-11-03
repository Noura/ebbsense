#include <Arduino.h>
#include <Thread.h>

Thread::Thread(int pinNumber, int leadTime, int power)
    : _pinNumber(pinNumber)
    , _leadTime(leadTime)
    , _power(power)
    , _powerLastOnTime(-1)
    , _powerLastOffTime(-1)
    , _onForTime(-1)
    , userRequest(THREAD_USER_STAYOFF)
{
    pinMode(_pinNumber, OUTPUT);
}

void Thread::_on() {
    analogWrite(_pinNumber, _power);
    _powerLastOnTime = millis();
}

void Thread::_off() {
    analogWrite(_pinNumber, 0);
    _powerLastOffTime = millis();
}

void Thread::onFor(int t) {
    _userRequest = THREAD_USER_ONFOR;
    _userRequestTime = millis();
    _onForTime = t;
}

void Thread::on() {
    _userRequest = THREAD_USER_STAYON;
    _userRequestTime = millis();
}

void Thread::off() {
    _userRequest = THREAD_USER_STAYOFF;
    _userRequestTime = millis();
    _onForTime = -1;
}

void Thread::flash() {
    onFor(_leadTime);
}

void Thread::update() {
    // if the user wanted it on for a specific amount of time which has now
    // passed, or if the user wanted it off
    if ( ( _userRequest == THREAD_USER_ONFOR &&
           millis() - _userRequesTime > _onForTime ) 
         ||
           // or if the user wanted it off
         ( _userRequest == THREAD_USER_STAYOFF ) )
    {
        _off();
    }
    // if the thread should be on right now
    else if ( _userRequest == THREAD_USER_ONFOR ||
              _userRequest == THREAD_USER_STAYON ) 
    {
        // if we haven't turned it on yet
        if ( _userRequestTime < _powerLastOnTime )
        {
            _on();
        }
        // if we have already turned it on and the thread is in its warmup phase
        else if ( _powerLastOnTime - _userRequestTime < _leadTime )
        {
            // do nothing, we already turned it on and should leave it alone
            // to let it warm up
        }
        // if the thread is done warming up
        else if ( _powerLastOnTime - _userRequestTime > _leadTime ) {
            // EFFICIENCY: cycle the thread on / off
            //
        }
    }
}
