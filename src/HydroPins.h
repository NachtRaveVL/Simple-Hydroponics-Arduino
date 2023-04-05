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
#ifndef HYDRO_DISABLE_MULTITASKING
class HydroPinExpander;
#endif

#include "Hydruino.h"
#include "HydroUtils.h"
#include "HydroModules.h"

// Creates Pin from passed Pin data (return ownership transfer - user code *must* delete returned Pin)
extern HydroPin *newPinObjectFromSubData(const HydroPinData *dataIn);

// Returns virtual pin # (100+) to use for a given pin channel # [0,16*Max), else -1/invalid
inline pintype_t pinNumberForPinChannel(int8_t pinChannel) { return isValidChannel(pinChannel) && pinChannel >= 0 ? (pintype_t)(hpin_virtual + pinChannel) : hpin_none; }
// Returns expanded pin channel # [0,16) to use for a given virtual pin # (100+), else pin # [0,99] if not virtual (<100), else -1/invalid
inline uint8_t pinChannelOrPinNumber(pintype_t pinNumber) { return isValidPin(pinNumber) ? (pinNumber >= hpin_virtual ? (pinNumber - hpin_virtual) % 16 : pinNumber) : (uint8_t)-1; }
// Returns expander position # [0,Max] to use for a given virtual pin # (100+), else -1/invalid if not virtual (<100)
inline hposi_t expanderPosForPinNumber(pintype_t pinNumber) { return isValidPin(pinNumber) && pinNumber >= hpin_virtual ? (pinNumber - hpin_virtual) / 16 : (hposi_t)-1; }
// Returns expander position # [0,Max] to use for a given expanded pin channel # [0,16*Max), else -1/invalid if not expanded
inline hposi_t expanderPosForPinChannel(int8_t pinChannel) { return isValidChannel(pinChannel) && pinChannel >= 0 ? pinChannel / 16 : (hposi_t)-1; }
// Returns muxer channel # [0,16) to use for a given muxed pin channel # [-1,-17), else -1/invalid if not muxed
inline uint8_t muxerChannelForPinChannel(int8_t pinChannel) { return isValidChannel(pinChannel) && pinChannel < 0 ? (abs(pinChannel) - 1) : (uint8_t)-1; }
// Returns muxed pin channel # [-1,-17) to use for a given muxer channel # [0,16), else -127/none
inline int8_t pinChannelForMuxerChannel(uint8_t muxChannel) { return muxChannel != (uint8_t)-1 && muxChannel != (uint8_t)hpinchnl_none ? -(int8_t)(1 + constrain(muxChannel,0,125)) : hpinchnl_none; }
// Returns expanded pin channel # [+0,16*Max) to use for a given expander channel # [0,16*Max), else -127/none
inline int8_t pinChannelForExpanderChannel(uint8_t expChannel) { return expChannel != (uint8_t)-1 && expChannel != (uint8_t)hpinchnl_none ? (int8_t)constrain(expChannel,0,127) : hpinchnl_none; }


// Pin Base
struct HydroPin {
    enum : signed char { Digital, Analog, Unknown = -1 } type; // Pin type (custom RTTI)
    inline bool isDigitalType() const { return type == Digital; }
    inline bool isAnalogType() const { return type == Analog; }
    inline bool isUnknownType() const { return type <= Unknown; }

    pintype_t pin;                                          // Pin number, else -1/undef or 100+/virtual
    Hydro_PinMode mode;                                     // Pin mode setting
    int8_t channel;                                         // Pin muxer(-vals)/expander(+vals) channel #, else -127/none

    HydroPin();
    HydroPin(int classType, pintype_t pinNumber = hpin_none, Hydro_PinMode pinMode = Hydro_PinMode_Undefined, int8_t pinChannel = hpinchnl_none);
    HydroPin(const HydroPinData *dataIn);

    explicit operator HydroDigitalPin() const;
    explicit operator HydroAnalogPin() const;

    // Initializes pin to mode settings. Re-entrant.
    void init();
    // De-initializes pin to floating state. Re-entrant.
    void deinit();

    void saveToData(HydroPinData *dataOut) const;

    // Attempts to both select the pin muxer (set address/ready pin state) for the pin on
    // its channel number and activate it (toggle chip enable). For pin expanders, attempts
    // pin expander device I/O sync.
    // Typically called pre-read. Returns success boolean. May return early.
    inline bool selectAndActivatePin() { return enablePin(0); }
    // Attempts to select the pin muxer (set address/ready pin state) for the pin on
    // its channel number. For pin expanders, attempts pin expander device I/O sync.
    // Typically called pre-write. Returns success boolean. May return early.
    inline bool selectPin() { return enablePin(1); }
    // Attempts to activate the pin muxer (toggle chip enable). For pin expanders, attempts
    // pin expander device I/O sync.
    // Typically called post-write. Returns success boolean. May return early.
    inline bool activatePin() { return enablePin(2); }

    inline bool isValid() const { return isValidPin(pin) && mode != Hydro_PinMode_Undefined; }
    inline bool isVirtual() const { return isValidPin(pin) && pin >= hpin_virtual; }
    inline bool isMuxed() const { return isValidChannel(channel) && channel <= -1; }
    inline bool isExpanded() const { return isValidChannel(channel) && channel >= 0; }
    inline bool isInput() const { return mode == Hydro_PinMode_Digital_Input ||
                                         mode == Hydro_PinMode_Digital_Input_PullUp ||
                                         mode == Hydro_PinMode_Digital_Input_PullDown ||
                                         mode == Hydro_PinMode_Analog_Input; }
    inline bool canRead() const { return isValid() && isInput(); }
    inline bool isOutput() const { return mode == Hydro_PinMode_Digital_Output ||
                                          mode == Hydro_PinMode_Digital_Output_PushPull ||
                                          mode == Hydro_PinMode_Analog_Output; }
    inline bool canWrite() const { return isValid() && isOutput(); }
    inline bool isDigital() const { return mode == Hydro_PinMode_Digital_Input ||
                                           mode == Hydro_PinMode_Digital_Input_PullUp ||
                                           mode == Hydro_PinMode_Digital_Input_PullDown ||
                                           mode == Hydro_PinMode_Digital_Output ||
                                           mode == Hydro_PinMode_Digital_Output_PushPull; }
    inline bool isAnalog() const { return mode == Hydro_PinMode_Analog_Input ||
                                          mode == Hydro_PinMode_Analog_Output; }

protected:
    bool enablePin(int step);
};

// Digital Pin
struct HydroDigitalPin : public HydroPin, public HydroDigitalInputPinInterface, public HydroDigitalOutputPinInterface {
    bool activeLow;                                         // Active-low trigger state boolean

    HydroDigitalPin();
    HydroDigitalPin(pintype_t pinNumber,                    // Digital pin number (e.g. D0, D1)
                    ard_pinmode_t pinMode,                  // Arduino pin mode (e.g. INPUT, OUTPUT, determines activeLow trigger state)
                    int8_t pinChannel = hpinchnl_none);     // Pin channel select, else -127/none
    HydroDigitalPin(pintype_t pinNumber,                    // Digital pin number (e.g. D0, D1)
                    Hydro_PinMode pinMode,                  // Hydruino pin mode (determines activeLow trigger state)
                    int8_t pinChannel = hpinchnl_none);     // Pin muxer/expander channel #, else -127/none
    HydroDigitalPin(pintype_t pinNumber,                    // Digital pin number (e.g. D0, D1)
                    ard_pinmode_t pinMode,                  // Arduino pin mode (e.g. INPUT, OUTPUT)
                    bool isActiveLow,                       // Explicit pin active-low trigger state boolean
                    int8_t pinChannel = hpinchnl_none);     // Pin muxer/expander channel #, else -127/none
    HydroDigitalPin(pintype_t pinNumber,                    // Digital pin number (e.g. D0, D1)
                    Hydro_PinMode pinMode,                  // Hydruino pin mode
                    bool isActiveLow,                       // Explicit pin active-low trigger state boolean
                    int8_t pinChannel = hpinchnl_none);     // Pin muxer/expander channel #, else -127/none
    HydroDigitalPin(const HydroPinData *dataIn);

    void saveToData(HydroPinData *dataOut) const;

    virtual ard_pinstatus_t digitalRead() override;
    inline bool isActive() { return digitalRead() == (activeLow ? LOW : HIGH); }

    virtual void digitalWrite(ard_pinstatus_t status) override;
    inline void activate() { digitalWrite((activeLow ? LOW : HIGH)); }
    inline void deactivate() { digitalWrite((activeLow ? HIGH : LOW)); }
};

// Analog Pin
struct HydroAnalogPin : public HydroPin, public HydroAnalogInputPinInterface, public HydroAnalogOutputPinInterface {
    BitResolution bitRes;                                   // Bit resolution
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
                   int8_t pinChannel = hpinchnl_none);      // Pin muxer/expander channel #, else -127/none
    HydroAnalogPin(pintype_t pinNumber,                     // Analog pin number (e.g. A0, A1)
                   Hydro_PinMode pinMode,                   // Hydruino pin mode
                   uint8_t analogBitRes = 0,                // Bit resolution (0 for std DAC/ADC res by mode i/o)
#ifdef ESP32
                   uint8_t pinPWMChannel = 1,               // PWM channel (0 reserved for buzzer)
#endif
#ifdef ESP_PLATFORM
                   float pinPWMFrequency = 1000,            // PWM frequency
#endif
                   int8_t pinChannel = hpinchnl_none);      // Pin muxer/expander channel #, else -127/none
    HydroAnalogPin(const HydroPinData *dataIn);

    void init();

    void saveToData(HydroPinData *dataOut) const;

    virtual float analogRead() override;
    virtual int analogRead_raw() override;

    virtual void analogWrite(float amount) override;
    virtual void analogWrite_raw(int amount) override;
};

// Combined Pin Serialization Sub Data
struct HydroPinData : public HydroSubData
{
    pintype_t pin;                                          // Pin number
    Hydro_PinMode mode;                                     // Pin mode
    int8_t channel;                                         // Pin muxer(-)/expander(+) channel #
    union {
        struct {
            bool activeLow;                                 // Active low trigger state
        } digitalPin;                                       // Digital pin
        struct {
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


// Pin Multiplexer (Muxer)
// A pin muxer reduces the number of individual MCU output pins needed by instead going
// through a multiplexer chip, such as a CD74HC, which utilizes a small 4-bit/16-channel
// address select bus. This allows a single shared signal pin access to much more than one
// circuit and/or device. Multiplexed pins will enable their associated pin muxer prior to
// utilizing. This class will correctly set signal pin to floating input state upon channel
// switches, with optional usage of a chip enable pin that reactivates after channel select.
// Separate optional interruptable pin allows ISR pass-through to any controller objects
// on shared signal pin upon activation signal.
// Note: Muxers are currently for use with controller objects only, and are not compatible
//       with control inputs/ioAbstractions as used by tcMenu.
// Note: Muxers are referenced in controller by signal pin #, with a pin's channel # defined
//       as a negative integer starting from -1 to indicate muxer channel # (e.g. -1 =
//       muxer[signalPin] channel 0, -16 = muxer[signalPin] channel 15, etc.).
class HydroPinMuxer {
public:
    HydroPinMuxer();
    HydroPinMuxer(HydroPin signalPin,
                  pintype_t *muxChannelPins, int8_t muxChannelBits,
                  HydroDigitalPin chipEnablePin = HydroDigitalPin(),
                  HydroDigitalPin interruptPin = HydroDigitalPin());

    // Initializes muxer channel bus, chip select, & interrupt pin, de-inits signal pin, and deactivates chip select.
    void init();

    // ISR registration requires an interruptable pin, i.e. a valid digitalPinToInterrupt(interruptPin).
    // Unless anyChange true, active-low interrupt pins interrupt on falling-edge, while active-high interrupt pins interrupt on rising-edge.
    // Interrupt routine does little but create an async Task via TaskManager, eliminating majority of race conditions.
    // Once registered, the ISR cannot be unregistered/changed. It is advised to use a lowered numbered pin # if able ([1-15,18]).
    bool tryRegisterISR(bool anyChange = false);

    inline const HydroPin &getSignalPin() const { return _signal; }
    inline const HydroDigitalPin &getChipEnablePin() const { return _chipEnable; }
    inline const HydroDigitalPin &getInterruptPin() const { return _interrupt; }
    inline uint8_t getChannelBits() const { return _channelBits; }
    inline uint8_t getSelectedChannel() const { return _channelSelect; }

protected:
    HydroPin _signal;                                       // Muxed signal pin (unused channel #)
    HydroDigitalPin _chipEnable;                            // Muxing chip enable pin (optional)
    HydroDigitalPin _interrupt;                             // Muxing interrupt pin (optional)
    pintype_t _channelPins[4];                              // Channel select bus pins
    uint8_t _channelBits;                                   // Channel select bits (# of bus pins available)
    uint8_t _channelSelect;                                 // Channel select (active channel)
    bool _usingISR;                                         // Using ISR flag

public: // consider protected
    void selectChannel(uint8_t channelNumber);              // Selects channel
    void setIsActive(bool isActive);                        // Sets channel activation
    inline void activate() { setIsActive(true); }           // Activates channel
    inline void deactivate() { setIsActive(false); }        // Deactivates channel
};

#ifndef HYDRO_DISABLE_MULTITASKING

// Pin Expander
// Ditching the address bus, a pin expander instead expands the pin #'s available by
// providing a set of virtualized pin #'s that can be accessed over serial i2c.
// This class uses IoAbstractionRef objects for tcMenu-compatible pinset expansion.
// In case of analog pins instead assume ioRef object is an AnalogDevice instance.
// Separate optional interruptable pin allows ISR pass-through to all virtual pins
// on the same expander upon activation signal.
// Note: Expanders are referenced in controller by position #, with pin channel # defined as
//       a positive integer to indicate pin expander # & pin # (e.g. 0-15 = expander[0] pin 0-15,
//       16-31 = expander[1] pin 0-15, etc.), and also by assigned virtual pin # (100 + channel #).
// Note: Virtual pins should always have a pin # >= hpin_virtual (100), and are separated
//       into groups of 16 per indexed expander (e.g. pins # 100-115 = expander[0], pins #
//       116-131 = expander[1], etc.).
class HydroPinExpander {
public:
    HydroPinExpander();
    HydroPinExpander(hposi_t expanderPos, uint8_t channelBits, IoAbstractionRef ioRef,
                     HydroDigitalPin interruptPin = HydroDigitalPin());

    // Initializes interrupt pin.
    inline void init() { _interrupt.init(); }

    // ISR registration requires an interruptable pin, i.e. a valid digitalPinToInterrupt(interruptPin).
    // Unless anyChange true, active-low interrupt pins interrupt on falling-edge, while active-high interrupt pins interrupt on rising-edge.
    // Interrupt routine does little but create an async Task via TaskManager, eliminating majority of race conditions.
    // Once registered, the ISR cannot be unregistered/changed. It is advised to use a lowered numbered pin # if able ([1-15,18]).
    bool tryRegisterISR(bool anyChange = false);

    // Synchronizes I/O with expander, returning success flag.
    bool trySyncChannel();

    inline hposi_t getExpanderPos() const { return _expander; }
    inline uint8_t getChannelBits() const { return _channelBits; }
    inline IoAbstractionRef getIoAbstraction() { return _ioRef; }
    inline const HydroDigitalPin &getInterruptPin() const { return _interrupt; }

protected:
    const hposi_t _expander;                                // Expander #/index (for virtual pin #)
    uint8_t _channelBits;                                   // Channel select bits (# of bus pins available)
    IoAbstractionRef _ioRef;                                // IoAbstraction instance
    HydroDigitalPin _interrupt;                             // Expander interrupt pin (optional)
    bool _usingISR;                                         // Using ISR flag
};

#endif // /ifndef HYDRO_DISABLE_MULTITASKING

#endif // /ifndef HydroPin_H
