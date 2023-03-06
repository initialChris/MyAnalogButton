#ifndef MY_ANALOG_BUTTONS
#define MY_ANALOG_BUTTONS

#include <Arduino.h>

#define GOOD_SAMPLE_THRESHOLD 2

#define CONTAINED(x, y, z) ((x) < (y) + (z) && (x) > (y) - (z))

// Debug Macro
//#define ENABLE_MY_ANALOG_BUTTONS_DEBUG
#ifdef ENABLE_MY_ANALOG_BUTTONS_DEBUG
    #define DEBUG_ANALOG_BUTTON_HEADER "MyAnalogButton -> "
    #define DEBUG_ANALOG_BUTTON(...) printHeader(); Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_ANALOG_BUTTON(...)
#endif

enum class ButtonState : uint8_t {
        RELASED=0,
        PRESSED=1,
        LONG_PRESS=2,
        LONG_PRESS_TICK=3
};

class MyAnalogButtons {
    

    public:
        MyAnalogButtons(const byte &pin, int * thresholds, const int &buttonsCount, const int &zeroThreshold,int longPressTime=0, int longPressTimeTick=0, int hysteresis = 60, int readHysteresis = 30, unsigned long sampleTime = 30);
        void setCallback(void (buttonCallback)(byte, ButtonState, unsigned long));

        void handle();

    private:
        
        byte _curPressed = 0;
        byte _lastPressed = 0;
        

        int * _thresholds;
        int _zeroThreshold;
        int _buttonsCount;
        byte _pin;

        int _hysteresis;
        int _readHysteresis;

        unsigned long _sampleMillis;
        unsigned long _sampleTime;

        unsigned long _longPressMillis;
        unsigned long _longPressTickMillis;
        unsigned long _longPressTime;
        unsigned long _longPressTimeTick;

        bool _longHandle = false;
        int _lastRead = 0;
        byte _goodCount = 0;

        void (*_buttonCallback)(byte button_index, ButtonState pressTime, unsigned long ms) = nullptr;

        #ifdef ENABLE_MY_ANALOG_BUTTONS_DEBUG
            static void printHeader() {Serial.print(DEBUG_ANALOG_BUTTON_HEADER);}
        #endif     
};


#endif
