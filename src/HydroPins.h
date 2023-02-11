/*  Hydruino: Simple automation controller for solar tracking systems.
    Copyright (C) 2023 NachtRaveVL          <nachtravevl@gmail.com>
    Hydruino Pins
*/

#ifndef HydroPin_H
#define HydroPin_H

struct HydroPin;
struct HydroDigitalPin;
struct HydroAnalogPin;
struct HydroPinData;

class HydroPinMuxer;
struct HydroPinMuxerData;

#include "Hydruino.h"
#include "HydroUtils.h"

// Creates Pin from passed Pin data (return ownership transfer - user code *must* delete returned Pin)
extern HydroPin *newPinObjectFromSubData(const HydroPinData *dataIn);


// Pin Base
struct HydroPin {
    enum : signed char { Digital, Analog, Unknown = -1 } type; // Pin type (custom RTTI)
    inline bool isDigitalType() const { return type == Digital; }
    inline bool isAnalogType() const { return type == Analog; }
    inline bool isUnknownType() const { return type <= Unknown; }

    pintype_t pin;                                          // Pin number (else -1 if undefined)
    Hydro_PinMode mode;                                     // Pin mode setting
    uint8_t channel;                                        // Muxing channel select (else -1 if unused)

    HydroPin();
    HydroPin(int classType, pintype_t pinNumber = -1, Hydro_PinMode pinMode = Hydro_PinMode_Undefined, uint8_t muxChannel = -1);
    HydroPin(const HydroPinData *dataIn);

    explicit operator HydroDigitalPin() const;
    explicit operator HydroAnalogPin() const;

    // Initializes pin to mode settings. Re-entrant.
    void init();
    // De-initializes pin to floating state. Re-entrant.
    inline void deinit() { pinMode(pin, INPUT); }

    void saveToData(HydroPinData *dataOut) const;

    // Attempts to enable the muxer for the pin on its channel number. Returns success boolean. May return early.
    bool tryEnableMuxer();

    inline bool isValid() const { return isValidPin(pin) && mode != Hydro_PinMode_Undefined; }
    inline bool isMuxed() const { return isValidChannel(channel); }
    inline bool isInput() const { return mode == Hydro_PinMode_Digital_Input_Floating ||
                                         mode == Hydro_PinMode_Digital_Input_PullUp ||
                                         mode == Hydro_PinMode_Digital_Input_PullDown ||
                                         mode == Hydro_PinMode_Analog_Input; }
    inline bool canRead() const { return isValid() && isInput(); }
    inline bool isOutput() const { return mode == Hydro_PinMode_Digital_Output_OpenDrain ||
                                          mode == Hydro_PinMode_Digital_Output_PushPull ||
                                          mode == Hydro_PinMode_Analog_Output; }
    inline bool canWrite() const { return isValid() && isOutput(); }
    inline bool isDigital() const { return mode == Hydro_PinMode_Digital_Input_Floating ||
                                           mode == Hydro_PinMode_Digital_Input_PullUp ||
                                           mode == Hydro_PinMode_Digital_Input_PullDown ||
                                           mode == Hydro_PinMode_Digital_Output_OpenDrain ||
                                           mode == Hydro_PinMode_Digital_Output_PushPull; }
    inline bool isAnalog() const { return mode == Hydro_PinMode_Analog_Input ||
                                          mode == Hydro_PinMode_Analog_Output; }
};

// Digital Pin
struct HydroDigitalPin : public HydroPin, public HydroDigitalInputPinInterface, public HydroDigitalOutputPinInterface
{
    bool activeLow;                                         // Active-low trigger state boolean

    HydroDigitalPin();
    HydroDigitalPin(pintype_t pinNumber,                    // Digital pin number (e.g. D0, D1)
                    ard_pinmode_t pinMode,                  // Arduino pin mode (e.g. INPUT, OUTPUT, determines activeLow trigger state)
                    uint8_t muxChannel = -1);               // Muxing channel select (else -1 if unused)
    HydroDigitalPin(pintype_t pinNumber,                    // Digital pin number (e.g. D0, D1)
                    Hydro_PinMode pinMode,                  // Hydruino pin mode (determines activeLow trigger state)
                    uint8_t muxChannel = -1);               // Muxing channel select (else -1 if unused)
    HydroDigitalPin(pintype_t pinNumber,                    // Digital pin number (e.g. D0, D1)
                    ard_pinmode_t pinMode,                  // Arduino pin mode (e.g. INPUT, OUTPUT)
                    bool isActiveLow,                       // Explicit pin active-low trigger state boolean
                    uint8_t muxChannel = -1);               // Muxing channel select (else -1 if unused)
    HydroDigitalPin(pintype_t pinNumber,                    // Digital pin number (e.g. D0, D1)
                    Hydro_PinMode pinMode,                  // Hydruino pin mode
                    bool isActiveLow,                       // Explicit pin active-low trigger state boolean
                    uint8_t muxChannel = -1);               // Muxing channel select (else -1 if unused)
    HydroDigitalPin(const HydroPinData *dataIn);

    inline operator HydroPin() const { return HydroPin((int)type, pin, mode, channel); }

    void saveToData(HydroPinData *dataOut) const;

    virtual ard_pinstatus_t digitalRead() override;
    inline bool isActive() { return this->digitalRead() == (activeLow ? LOW : HIGH); }

    virtual void digitalWrite(ard_pinstatus_t status) override;
    inline void activate() { this->digitalWrite((activeLow ? LOW : HIGH)); }
    inline void deactivate() { this->digitalWrite((activeLow ? HIGH : LOW)); }
};

// Analog Pin
struct HydroAnalogPin : public HydroPin, public HydroAnalogInputPinInterface, public HydroAnalogOutputPinInterface
{
    HydroBitResolution bitRes;                              // Bit resolution
#ifdef ESP32
    uint8_t pwmChannel;                                     // PWM channel
#endif
#ifdef ESP_PLATFORM
    float pwmFrequency;                                     // PWM frequency
#endif

    HydroAnalogPin();
    HydroAnalogPin(pintype_t pinNumber,                     // Analog pin number (e.g. A0, A1)
                   ard_pinmode_t pinMode,                   // Arduino pin mode (e.g. INPUT, OUTPUT)
                   uint8_t analogBitRes = 0,                // Bit resolution (0 for std DAC/ADC res by mode i/o)
#ifdef ESP32
                   uint8_t pinPWMChannel = 1,               // PWM channel (0 reserved for buzzer)
#endif
#ifdef ESP_PLATFORM
                   float pinPWMFrequency = 1000,            // PWM frequency
#endif
                   uint8_t muxChannel = -1);                // Muxing channel select (else -1 if unused)
    HydroAnalogPin(pintype_t pinNumber,                     // Analog pin number (e.g. A0, A1)
                   Hydro_PinMode pinMode,                   // Hydruino pin mode
                   uint8_t analogBitRes = 0,                // Bit resolution (0 for std DAC/ADC res by mode i/o)
#ifdef ESP32
                   uint8_t pinPWMChannel = 1,               // PWM channel (0 reserved for buzzer)
#endif
#ifdef ESP_PLATFORM
                   float pinPWMFrequency = 1000,            // PWM frequency
#endif
                   uint8_t muxChannel = -1);                // Muxing channel select (else -1 if unused)
    HydroAnalogPin(const HydroPinData *dataIn);

    inline operator HydroPin() const { return HydroPin((int)type, pin, mode, channel); }

    void init();

    void saveToData(HydroPinData *dataOut) const;

    float analogRead();
    int analogRead_raw();

    void analogWrite(float val);
    void analogWrite_raw(int val);
};

// Combined Pin Serialization Sub Data
struct HydroPinData : public HydroSubData
{
    pintype_t pin;                                          // Pin number
    Hydro_PinMode mode;                                     // Pin mode
    uint8_t channel;                                        // Muxing channel
    union
    {
        struct
        {
            bool activeLow;                                 // Active low trigger state
        } digitalPin;                                       // Digital pin
        struct
        {
            uint8_t bitRes;                                 // Bit resolution
#ifdef ESP32
            uint8_t pwmChannel;                             // PWM channel
#endif
#ifdef ESP_PLATFORM
            float pwmFrequency;                             // PWM frequency
#endif
        } analogPin;
    } dataAs;                                               // Enumeration type union

    HydroPinData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};


// Pin Muxer
// A muxer reduces the number of individual i/o wires needed by instead going through
// a multiplexer that takes in a channel address through a small channel address bus.
// This allows a single pin access to much more than one single circuit/device. Pins that
// are muxed will enable their associated pin muxer (which deactivates all other muxers)
// prior to utilizing them. This muxer will set i/o pins to floating state as appropriate
// during channel switches, with optional usage of chip enable.
class HydroPinMuxer
{
public:
    HydroPinMuxer();
    HydroPinMuxer(HydroPin signalPin,
                  pintype_t *muxChannelPins, uint8_t muxChannelBits,
                  HydroDigitalPin chipEnablePin = HydroDigitalPin());
    HydroPinMuxer(const HydroPinMuxerData *dataIn);

    void saveToData(HydroPinMuxerData *dataOut) const;

    void init();

    inline const HydroPin &getSignalPin() { return _signal; }
    inline const HydroDigitalPin &getChipEnablePin() { return _chipEnable; }
    inline uint8_t getChannelSelectBits() { return _channelBits; }
    inline uint8_t getSelectedChannel() { return _channelSelect; }

protected:
    HydroPin _signal;                                       // Muxed signal pin (only needs/saves pin/mode)
    HydroDigitalPin _chipEnable;                            // Muxing chip enable pin (optional)
    pintype_t _channelPins[5];                              // Channel select bus pins
    uint8_t _channelBits;                                   // Channel select bits (# of bus pins available)
    uint8_t _channelSelect;                                 // Channel select (active channel)

    void selectChannel(uint8_t channelNumber);
    void deselect();

    friend class Hydruino;
    friend class HydroPin;
};

// Pin Muxer Serialization Sub Data
struct HydroPinMuxerData : HydroSubData
{
    HydroPinData signal;                                    // Signal pin data
    HydroPinData chipEnable;                                // Chip enable pin data
    pintype_t channelPins[5];                               // Channel select bus pins
    uint8_t channelBits;                                    // Channel select bits

    HydroPinMuxerData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef HydroPin_H
