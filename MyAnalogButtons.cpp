#include "MyAnalogButtons.h"

MyAnalogButtons::MyAnalogButtons(const byte &pin, int * thresholds, const int &buttonsCount,  const int &zeroThreshold, int longPressTime, int longPressTimeTick, int hysteresis, int readHysteresis, unsigned long sampleTime) {
    _pin = pin;
    _thresholds = thresholds;
    _buttonsCount = buttonsCount;
    _zeroThreshold = zeroThreshold;
    _hysteresis = hysteresis;
    _readHysteresis = readHysteresis;
    _sampleTime = sampleTime;

    _longPressTime = longPressTime;
    _longPressTimeTick = longPressTimeTick;
    
}

void MyAnalogButtons::setCallback(void (buttonCallback)(byte, ButtonState, unsigned long)){
    _buttonCallback = buttonCallback;
}


void MyAnalogButtons::handle(){

    if (millis() - _sampleMillis < _sampleTime) return;
    
    int currentRead = analogRead(_pin);

    _goodCount = CONTAINED(currentRead, _lastRead, _readHysteresis) ? _goodCount + 1 : 0;

    if (_goodCount == GOOD_SAMPLE_THRESHOLD){
        if (CONTAINED(currentRead, _zeroThreshold, _hysteresis)){
            _curPressed = 0;
        }
        else {
            for (int x=0; x<_buttonsCount; x++){
                if (CONTAINED(currentRead, _thresholds[x], _hysteresis)) {
                    _curPressed = x+1;
                    break;
                }
            }
        }
        _goodCount = 0;
    }

    if (_curPressed != _lastPressed){
        
        if (_lastPressed != 0 ){
            DEBUG_ANALOG_BUTTON("Button %d relased\n", _lastPressed);
            if (_buttonCallback != nullptr) _buttonCallback(_lastPressed, ButtonState::RELASED , millis() - _longPressMillis);
            _longPressMillis = millis();
        }

        if (_curPressed != 0){
            DEBUG_ANALOG_BUTTON("Button %d pressed\n", _curPressed);
            if (_buttonCallback != nullptr) _buttonCallback(_curPressed, ButtonState::PRESSED , millis() - _longPressMillis);
            _longPressMillis = millis();
            _longPressTickMillis = millis();
            _longHandle = false;
        }

        //Serial.println(_curPressed);
        _lastPressed = _curPressed;
    }

    if (_curPressed != 0){
        if (!_longHandle && _longPressTime > 0 && millis() - _longPressTickMillis > _longPressTime) {
            DEBUG_ANALOG_BUTTON("Button %d long\n", _curPressed);
            if (_buttonCallback != nullptr) _buttonCallback(_curPressed, ButtonState::LONG_PRESS , millis() - _longPressMillis);
            _longPressTickMillis = millis();
            _longHandle = true;
        }
        else if (_longHandle && _longPressTimeTick > 0 && millis() - _longPressTickMillis > _longPressTimeTick) {
            DEBUG_ANALOG_BUTTON("Button %d long tick\n", _curPressed);
            if (_buttonCallback != nullptr) _buttonCallback(_curPressed, ButtonState::LONG_PRESS_TICK , millis() - _longPressMillis);
            _longPressTickMillis = millis();
        }    
    }

    _lastRead = currentRead;
    _sampleMillis = millis();
}

