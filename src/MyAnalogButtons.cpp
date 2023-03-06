#include "MyAnalogButtons.h"

MyAnalogButtons::MyAnalogButtons(const byte &pin, // Pin to read using the analogRead(pin) function
                                 int * thresholds, //Array containing the thresholds of all buttons
                                 const int &buttonsCount, //Number of buttons defined in the array 
                                 const int &zeroThreshold, //Analog value when no button is pressed 
                                 int longPressTime, //Time in milliseconds after what to consider a long press. If set to zero it does not generate the LONG_PRESS event
                                 int longPressTimeTick, //How often to raise the LONG_PRESS_TICK event if the button continues to be pressed. Set to zero to disable
                                 int hysteresis, //Hysteresis window to apply to each threshold 
                                 int readHysteresis, //Hysteresis window to filter noise on analog readings
                                 unsigned long sampleTime //How often should a reading be performed. 
                                                          //Values that are too small can slow down program execution, while values that are too large cause latency
                                 ) {
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

/* The function to call in case of an event generation */
void MyAnalogButtons::setCallback(void (buttonCallback)(byte, ButtonState, unsigned long)){
    _buttonCallback = buttonCallback;
}

/* This is where the reading and processing of data takes place. If this function is not performed repeatedly then the state of the buttons will not be updated */
void MyAnalogButtons::handle(){

    // If the minimum time between one reading and the next has not yet passed, do nothing
    if (millis() - _sampleMillis < _sampleTime) return;
    
    /* READING */
    //Current reading value.
    //A possible improvement is to anchor this value to the address of an external function so that the value coming from external DACs can also be read
    int currentRead = analogRead(_pin);

    /* FILTERING */
    // Check that the current reading is consistent with the previous reading
    _goodCount = CONTAINED(currentRead, _lastRead, _readHysteresis) ? _goodCount + 1 : 0;

    //If the number of coherent readings is equal to the defined value, the button values can be updated
    if (_goodCount == GOOD_SAMPLE_THRESHOLD){

        //If the value equals the value where no button is pressed then set the index of the current button to 0
        if (CONTAINED(currentRead, _zeroThreshold, _hysteresis)){
            _curPressed = 0; //Treat no button presses as the 0 button
        }
        else {
            // Otherwise check if a button matches the current value
            for (int x=0; x<_buttonsCount; x++){
                if (CONTAINED(currentRead, _thresholds[x], _hysteresis)) {
                    _curPressed = x+1; //Apply an offset of 1 as zero is dedicated to no button pressed
                    break;
                }
            }
        }
        _goodCount = 0; // Reset the consistent reading counter
    }

    /* BUTTON STATE HANDLING*/
    //If the button pressed is the same as in the previous cycle then it is not necessary to update the state of the buttons
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

        _lastPressed = _curPressed;
    }

    /* LONG PRESS MANAGEMENT*/
    // If the current button is 0 then no button is pressed and there is no need to indicate a long press

    if (_curPressed != 0){
        
        // If a button has been pressed long enough but the LONG_PRESS event has not yet been raised
        if (!_longHandle && _longPressTime > 0 && millis() - _longPressTickMillis > _longPressTime) {
            //If _longPressTime=0 the generation of the LONG_PRESS event is disabled
            DEBUG_ANALOG_BUTTON("Button %d long\n", _curPressed);
            if (_buttonCallback != nullptr) _buttonCallback(_curPressed, ButtonState::LONG_PRESS , millis() - _longPressMillis);
            _longPressTickMillis = millis();
            _longHandle = true; // Signals that the LONG_PRESS event has already been raised
        }

        //Otherwise, if the LONG_PRESS event has already been signaled, repeatedly fire the tick event
        else if (_longHandle && _longPressTimeTick > 0 && millis() - _longPressTickMillis > _longPressTimeTick) {
            //If _longPressTimeTick=0 the generation of the LONG_PRESS_TICK event is disabled
            DEBUG_ANALOG_BUTTON("Button %d long tick\n", _curPressed);
            if (_buttonCallback != nullptr) _buttonCallback(_curPressed, ButtonState::LONG_PRESS_TICK , millis() - _longPressMillis);
            _longPressTickMillis = millis();
        }    
    }

    _lastRead = currentRead;
    _sampleMillis = millis();
}

