/*  Hydruino: Simple automation controller for solar tracking systems.
    Copyright (C) 2023 NachtRaveVL          <nachtravevl@gmail.com>
    Hydruino Pins
*/

#include "Hydruino.h"
#include "AnalogDeviceAbstraction.h"

HydroPin *newPinObjectFromSubData(const HydroPinData *dataIn)
{
    if (!dataIn || !isValidType(dataIn->type)) return nullptr;
    HYDRO_SOFT_ASSERT(dataIn && isValidType(dataIn->type), SFP(HStr_Err_InvalidParameter));

    if (dataIn) {
        switch (dataIn->type) {
            case HydroPin::Digital:
                return new HydroDigitalPin(dataIn);
            case HydroPin::Analog:
                return new HydroAnalogPin(dataIn);
            default: break;
        }
    }

    return nullptr;
}

HydroPin::HydroPin()
    : type(Unknown), pin(hpin_none), mode(Hydro_PinMode_Undefined), channel(hpinchnl_none)
{ ; }

HydroPin::HydroPin(int classType, pintype_t pinNumber, Hydro_PinMode pinMode, int8_t pinChannel)
    : type((typeof(type))classType), pin(pinNumber), mode(pinMode),
      channel(pinChannel == hpinchnl_none ? (pinNumber != hpin_none && pinNumber >= hpin_virtual ? pinChannelForExpanderChannel(abs(pinNumber - hpin_virtual)) : hpinchnl_none)
                                          : (pinNumber != hpin_none && pinNumber >= hpin_virtual ? pinChannelForExpanderChannel(abs(pinChannel)) : pinChannelForMuxerChannel(abs(pinChannel))))
{ ; }

HydroPin::HydroPin(const HydroPinData *dataIn)
    : type((typeof(type))(dataIn->type)), pin(dataIn->pin), mode(dataIn->mode), channel(dataIn->channel)
{ ; }

HydroPin::operator HydroDigitalPin() const
{
    return (isDigitalType() || isDigital() || (!isUnknownType() && !isAnalog())) ? HydroDigitalPin(pin, mode, channel) : HydroDigitalPin();
}

HydroPin::operator HydroAnalogPin() const
{
    return (isAnalogType() || isAnalog() || (!isUnknownType() && !isDigital())) ? HydroAnalogPin(pin, mode, isOutput() ? DAC_RESOLUTION : ADC_RESOLUTION, channel) : HydroAnalogPin();
}

void HydroPin::saveToData(HydroPinData *dataOut) const
{
    dataOut->type = (int8_t)type;
    dataOut->pin = pin;
    dataOut->mode = mode;
    dataOut->channel = channel;
}

void HydroPin::init()
{
    #if !HYDRO_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (!(isExpanded() || isVirtual())) {
                HYDRO_SOFT_ASSERT(!isMuxed() || channel == pinChannelForMuxerChannel(abs(channel)), SFP(HStr_Err_NotConfiguredProperly));

                switch (mode) {
                    case Hydro_PinMode_Digital_Input:
                    case Hydro_PinMode_Analog_Input:
                        pinMode(pin, INPUT);
                        break;

                    case Hydro_PinMode_Digital_Input_PullUp:
                        pinMode(pin, INPUT_PULLUP);
                        break;

                    case Hydro_PinMode_Digital_Input_PullDown:
                        #if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_MBED) || defined(ESP32) || defined(ARDUINO_ARCH_STM32) || defined(CORE_TEENSY) || defined(INPUT_PULLDOWN)
                            pinMode(pin, INPUT_PULLDOWN);
                        #else
                            pinMode(pin, INPUT);
                        #endif
                        break;

                    case Hydro_PinMode_Digital_Output:
                    case Hydro_PinMode_Digital_Output_PushPull:
                    case Hydro_PinMode_Analog_Output:
                        pinMode(pin, OUTPUT);
                        break;

                    default:
                        break;
                }
            } else {
                #ifndef HYDRO_DISABLE_MULTITASKING
                    HYDRO_SOFT_ASSERT(isVirtual() && pin == pinNumberForPinChannel(channel), SFP(HStr_Err_NotConfiguredProperly));
                    HYDRO_SOFT_ASSERT(channel == pinChannelForExpanderChannel(abs(channel)), SFP(HStr_Err_NotConfiguredProperly));

                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderForPinChannel(channel) : expanderForPinNumber(pin)) : nullptr;
                    if (expander) {
                        #if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_MBED) || defined(ESP32) || defined(ARDUINO_ARCH_STM32) || defined(CORE_TEENSY) || defined(INPUT_PULLDOWN)
                            expander->getIoAbstraction()->pinDirection(abs(channel), isOutput() ? OUTPUT : mode == Hydro_PinMode_Digital_Input_PullUp ? INPUT_PULLUP : mode == Hydro_PinMode_Digital_Input_PullDown ? INPUT_PULLDOWN : INPUT);
                        #else
                            expander->getIoAbstraction()->pinDirection(abs(channel), isOutput() ? OUTPUT : mode == Hydro_PinMode_Digital_Input_PullUp ? INPUT_PULLUP : INPUT);
                        #endif
                    }
                #else
                    HYDRO_HARD_ASSERT(false, SFP(HStr_Err_NotConfiguredProperly));
                #endif
            }
        }
    #endif
}

void HydroPin::deinit()
{
    #if !HYDRO_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (!(isExpanded() || isVirtual())) {
                pinMode(pin, INPUT);
            } else {
                #ifndef HYDRO_DISABLE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderForPinChannel(channel) : expanderForPinNumber(pin)) : nullptr;
                    if (expander) {
                        expander->getIoAbstraction()->pinDirection(abs(channel), INPUT);
                    }
                #else
                    HYDRO_HARD_ASSERT(false, SFP(HStr_Err_NotConfiguredProperly));
                #endif
            }
        }
    #endif
}

bool HydroPin::enablePin(int step)
{
    #if !HYDRO_SYS_DRY_RUN_ENABLE
        if (isValid() && isValidChannel(channel)) {
            if (isMuxed()) {
                SharedPtr<HydroPinMuxer> muxer = getController() ? getController()->getPinMuxer(pin) : nullptr;
                if (muxer) {
                    switch (step) {
                        case 0: muxer->selectChannel(channel); muxer->activate(); return true;
                        case 1: muxer->selectChannel(channel); return true;
                        case 2: muxer->activate(); return true;
                        default: return false;
                    }
                }
            } else if (isExpanded() || isVirtual()) {
                #ifndef HYDRO_DISABLE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderForPinChannel(channel) : expanderForPinNumber(pin)) : nullptr;
                    return expander && expander->syncChannel();
                #else
                    HYDRO_HARD_ASSERT(false, SFP(HStr_Err_NotConfiguredProperly));
                #endif
            }
        }
        return false;
    #else
        return isValid() && isValidChannel(channel);
    #endif
}


HydroDigitalPin::HydroDigitalPin()
    : HydroPin(Digital), activeLow(false)
{ ; }

HydroDigitalPin::HydroDigitalPin(pintype_t pinNumber, ard_pinmode_t pinMode, int8_t pinChannel)
    : HydroPin(Digital, pinNumber, pinMode != OUTPUT ? (pinMode != INPUT ? (pinMode == INPUT_PULLUP ? Hydro_PinMode_Digital_Input_PullUp : Hydro_PinMode_Digital_Input_PullDown)
                                                                         : Hydro_PinMode_Digital_Input)
                                                     : (pinMode == OUTPUT ? Hydro_PinMode_Digital_Output : Hydro_PinMode_Digital_Output_PushPull), pinChannel),
      activeLow(pinMode == INPUT || pinMode == INPUT_PULLUP || pinMode == OUTPUT)
{ ; }

HydroDigitalPin::HydroDigitalPin(pintype_t pinNumber, Hydro_PinMode pinMode, int8_t pinChannel)
    : HydroPin(Digital, pinNumber, pinMode, pinChannel),
      activeLow(pinMode == Hydro_PinMode_Digital_Input ||
                pinMode == Hydro_PinMode_Digital_Input_PullUp ||
                pinMode == Hydro_PinMode_Digital_Output)
{ ; }

HydroDigitalPin::HydroDigitalPin(pintype_t pinNumber, ard_pinmode_t pinMode, bool isActiveLow, int8_t pinChannel)
    : HydroPin(Digital, pinNumber, pinMode != OUTPUT ? (isActiveLow ? Hydro_PinMode_Digital_Input_PullUp : Hydro_PinMode_Digital_Input_PullDown)
                                                     : (isActiveLow ? Hydro_PinMode_Digital_Output : Hydro_PinMode_Digital_Output_PushPull), pinChannel),
      activeLow(isActiveLow)
{ ; }

HydroDigitalPin::HydroDigitalPin(pintype_t pinNumber, Hydro_PinMode pinMode, bool isActiveLow, int8_t pinChannel)
    : HydroPin(Digital, pinNumber, pinMode, pinChannel),
      activeLow(isActiveLow)
{ ; }

HydroDigitalPin::HydroDigitalPin(const HydroPinData *dataIn)
    : HydroPin(dataIn), activeLow(dataIn->dataAs.digitalPin.activeLow)
{ ; }

void HydroDigitalPin::saveToData(HydroPinData *dataOut) const
{
    HydroPin::saveToData(dataOut);

    dataOut->dataAs.digitalPin.activeLow = activeLow;
}

ard_pinstatus_t HydroDigitalPin::digitalRead()
{
    #if !HYDRO_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (isValidChannel(channel)) { selectAndActivatePin(); }
            if (!(isExpanded() || isVirtual())) {
                return ::digitalRead(pin);
            } else {
                #ifndef HYDRO_DISABLE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderForPinChannel(channel) : expanderForPinNumber(pin)) : nullptr;
                    if (expander) {
                        return (ard_pinstatus_t)(expander->getIoAbstraction()->readValue(abs(channel)));
                    }
                #else
                    HYDRO_HARD_ASSERT(false, SFP(HStr_Err_NotConfiguredProperly));
                #endif
            }
        }
    #endif
    return (ard_pinstatus_t)-1;
}

void HydroDigitalPin::digitalWrite(ard_pinstatus_t status)
{
    #if !HYDRO_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (!(isExpanded() || isVirtual())) {
                if (isMuxed()) { selectPin(); }
                ::digitalWrite(pin, status);
            } else {
                #ifndef HYDRO_DISABLE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderForPinChannel(channel) : expanderForPinNumber(pin)) : nullptr;
                    if (expander) {
                        expander->getIoAbstraction()->writeValue(abs(channel), (uint8_t)status);
                    }
                #else
                    HYDRO_HARD_ASSERT(false, SFP(HStr_Err_NotConfiguredProperly));
                #endif
            }
            if (isValidChannel(channel)) { activatePin(); }
        }
    #endif
}


HydroAnalogPin::HydroAnalogPin()
    : HydroPin(Analog), bitRes(0)
#ifdef ESP32
      , pwmChannel(-1)
#endif
#ifdef ESP_PLATFORM
      , pwmFrequency(0)
#endif
{ ; }

HydroAnalogPin::HydroAnalogPin(pintype_t pinNumber, ard_pinmode_t pinMode, uint8_t analogBitRes,
#ifdef ESP32
                               uint8_t pinPWMChannel,
#endif
#ifdef ESP_PLATFORM
                               float pinPWMFrequency,
#endif
                               int8_t pinChannel)
    : HydroPin(Analog, pinNumber, pinMode != OUTPUT ? Hydro_PinMode_Analog_Input : Hydro_PinMode_Analog_Output, pinChannel),
      bitRes(analogBitRes ? analogBitRes : (pinMode == OUTPUT ? DAC_RESOLUTION : ADC_RESOLUTION))
#ifdef ESP32
      , pwmChannel(pinPWMChannel)
#endif
#ifdef ESP_PLATFORM
      , pwmFrequency(pinPWMFrequency)
#endif
{ ; }

HydroAnalogPin::HydroAnalogPin(pintype_t pinNumber, Hydro_PinMode pinMode, uint8_t analogBitRes,
#ifdef ESP32
                               uint8_t pinPWMChannel,
#endif
#ifdef ESP_PLATFORM
                               float pinPWMFrequency,
#endif
                               int8_t pinChannel)
    : HydroPin(Analog, pinNumber, pinMode, pinChannel),
      bitRes(analogBitRes ? analogBitRes : (pinMode == Hydro_PinMode_Analog_Output ? DAC_RESOLUTION : ADC_RESOLUTION))
#ifdef ESP32
      , pwmChannel(pinPWMChannel)
#endif
#ifdef ESP_PLATFORM
      , pwmFrequency(pinPWMFrequency)
#endif
{ ; }

HydroAnalogPin::HydroAnalogPin(const HydroPinData *dataIn)
    : HydroPin(dataIn), bitRes(dataIn->dataAs.analogPin.bitRes)
#ifdef ESP32
      , pwmChannel(dataIn->dataAs.analogPin.pwmChannel)
#endif
#ifdef ESP_PLATFORM
      , pwmFrequency(dataIn->dataAs.analogPin.pwmFrequency)
#endif
{ ; }

void HydroAnalogPin::init()
{
    #if !HYDRO_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (!(isExpanded() || isVirtual())) {
                HydroPin::init();

                #ifdef ESP32
                    ledcAttachPin(pin, pwmChannel);
                    ledcSetup(pwmChannel, pwmFrequency, bitRes.bits);
                #endif
            } else {
                #ifndef HYDRO_DISABLE_MULTITASKING
                    HYDRO_SOFT_ASSERT(isVirtual() && pin == pinNumberForPinChannel(channel), SFP(HStr_Err_NotConfiguredProperly));
                    HYDRO_SOFT_ASSERT(channel == pinChannelForExpanderChannel(abs(channel)), SFP(HStr_Err_NotConfiguredProperly));

                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderForPinChannel(channel) : expanderForPinNumber(pin)) : nullptr;
                    if (expander) {
                        auto ioDir = isOutput() ? AnalogDirection::DIR_OUT : AnalogDirection::DIR_IN;
                        auto analogIORef = (AnalogDevice *)(expander->getIoAbstraction());
                        analogIORef->initPin(abs(channel), ioDir);

                        auto ioRefBits = analogIORef->getBitDepth(ioDir, abs(channel));
                        if (bitRes.bits != ioRefBits) {
                            bitRes = BitResolution(ioRefBits);
                        }
                    }
                #else
                    HYDRO_HARD_ASSERT(false, SFP(HStr_Err_NotConfiguredProperly));
                #endif
            }
        }
    #endif
}

void HydroAnalogPin::saveToData(HydroPinData *dataOut) const
{
    HydroPin::saveToData(dataOut);

    dataOut->dataAs.analogPin.bitRes = bitRes.bits;
    #ifdef ESP32
        dataOut->dataAs.analogPin.pwmChannel = pwmChannel;
    #endif
    #ifdef ESP_PLATFORM
        dataOut->dataAs.analogPin.pwmFrequency = pwmFrequency;
    #endif
}

float HydroAnalogPin::analogRead()
{
    return bitRes.transform(analogRead_raw());
}

int HydroAnalogPin::analogRead_raw()
{
    #if !HYDRO_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (isValidChannel(channel)) { selectAndActivatePin(); }
            if (!(isExpanded() || isVirtual())) {
                #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
                    analogReadResolution(bitRes.bits);
                #endif
                return ::analogRead(pin);
            } else {
                #ifndef HYDRO_DISABLE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderForPinChannel(channel) : expanderForPinNumber(pin)) : nullptr;
                    if (expander) {
                        auto analogIORef = (AnalogDevice *)(expander->getIoAbstraction());
                        analogIORef->getCurrentValue(abs(channel));
                    }
                #else
                    HYDRO_HARD_ASSERT(false, SFP(HStr_Err_NotConfiguredProperly));
                #endif
            }
        }
    #endif
    return 0;
}

void HydroAnalogPin::analogWrite(float amount)
{
    analogWrite_raw(bitRes.inverseTransform(amount));
}

void HydroAnalogPin::analogWrite_raw(int amount)
{
    #if !HYDRO_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            if (!(isExpanded() || isVirtual())) {
                if (isMuxed()) { selectPin(); }
                #ifdef ESP32
                    ledcWrite(pwmChannel, amount);
                #else
                    #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
                        analogWriteResolution(bitRes.bits);
                    #elif defined(ESP8266)
                        analogWriteRange(bitRes.maxVal);
                        analogWriteFreq(pwmFrequency);
                    #endif
                    ::analogWrite(pin, amount);
                #endif
            } else {
                #ifndef HYDRO_DISABLE_MULTITASKING
                    auto expander = getController() ? getController()->getPinExpander(isValidChannel(channel) ? expanderForPinChannel(channel) : expanderForPinNumber(pin)) : nullptr;
                    if (expander) {
                        auto analogIORef = (AnalogDevice *)(expander->getIoAbstraction());
                        analogIORef->setCurrentValue(abs(channel), amount);
                    }
                #else
                    HYDRO_HARD_ASSERT(false, SFP(HStr_Err_NotConfiguredProperly));
                #endif
            }
            if (isValidChannel(channel)) { activatePin(); }
        }
    #endif
}


HydroPinData::HydroPinData()
    : HydroSubData((int8_t)HydroPin::Unknown), pin(hpin_none), mode(Hydro_PinMode_Undefined), channel(hpinchnl_none), dataAs{0}
{ ; }

void HydroPinData::toJSONObject(JsonObject &objectOut) const
{
    HydroSubData::toJSONObject(objectOut);

    if (isValidPin(pin)) { objectOut[SFP(HStr_Key_Pin)] = pin; }
    if (mode != Hydro_PinMode_Undefined) { objectOut[SFP(HStr_Key_Mode)] = pinModeToString(mode); }
    if (isValidChannel(channel)) { objectOut[SFP(HStr_Key_Channel)] = channel; }

    if (mode != Hydro_PinMode_Undefined) {
        if (!(mode == Hydro_PinMode_Analog_Input || mode == Hydro_PinMode_Analog_Output)) {
            objectOut[SFP(HStr_Key_ActiveLow)] = dataAs.digitalPin.activeLow;
        } else {
            objectOut[SFP(HStr_Key_BitRes)] = dataAs.analogPin.bitRes;
            #ifdef ESP32
                objectOut[SFP(HStr_Key_PWMChannel)] = dataAs.analogPin.pwmChannel;
            #endif
            #ifdef ESP_PLATFORM
                objectOut[SFP(HStr_Key_PWMFrequency)] = dataAs.analogPin.pwmFrequency;
            #endif
        }
    }
}

void HydroPinData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroSubData::fromJSONObject(objectIn);

    pin = objectIn[SFP(HStr_Key_Pin)] | pin;
    mode = pinModeFromString(objectIn[SFP(HStr_Key_Mode)]);
    channel = objectIn[SFP(HStr_Key_Channel)] | channel;

    if (mode != Hydro_PinMode_Undefined) {
        if (!(mode == Hydro_PinMode_Analog_Input || mode == Hydro_PinMode_Analog_Output)) {
            type = (int8_t)HydroPin::Digital;
            dataAs.digitalPin.activeLow = objectIn[SFP(HStr_Key_ActiveLow)] | dataAs.digitalPin.activeLow;
        } else {
            type = (int8_t)HydroPin::Analog;
            dataAs.analogPin.bitRes = objectIn[SFP(HStr_Key_BitRes)] | dataAs.analogPin.bitRes;
            #ifdef ESP32
                dataAs.analogPin.pwmChannel = objectIn[SFP(HStr_Key_PWMChannel)] | dataAs.analogPin.pwmChannel;
            #endif
            #ifdef ESP_PLATFORM
                dataAs.analogPin.pwmFrequency = objectIn[SFP(HStr_Key_PWMFrequency)] | dataAs.analogPin.pwmFrequency;
            #endif
        }
    } else {
        type = (int8_t)HydroPin::Unknown;
    }
}


HydroPinMuxer::HydroPinMuxer()
    : _signal(), _chipEnable(), _channelPins{hpin_none},
      _channelBits(0), _channelSelect(-1)
{
    _signal.channel = hpinchnl_none; // unused
}

HydroPinMuxer::HydroPinMuxer(HydroPin signalPin,
                             pintype_t *muxChannelPins, int8_t muxChannelBits,
                             HydroDigitalPin chipEnablePin)
    : _signal(signalPin), _chipEnable(chipEnablePin),
      _channelPins{ muxChannelBits > 0 ? muxChannelPins[0] : hpin_none,
                    muxChannelBits > 1 ? muxChannelPins[1] : hpin_none,
                    muxChannelBits > 2 ? muxChannelPins[2] : hpin_none,
                    muxChannelBits > 3 ? muxChannelPins[3] : hpin_none },
      _channelBits(muxChannelBits), _channelSelect(-1)
{
    _signal.channel = hpinchnl_none; // unused
}

void HydroPinMuxer::init()
{
    _signal.deinit();
    _chipEnable.init();
    _chipEnable.deactivate();

    if (isValidPin(_channelPins[0])) {
        pinMode(_channelPins[0], OUTPUT);
        ::digitalWrite(_channelPins[0], LOW);

        if (isValidPin(_channelPins[1])) {
            pinMode(_channelPins[1], OUTPUT);
            ::digitalWrite(_channelPins[1], LOW);

            if (isValidPin(_channelPins[2])) {
                pinMode(_channelPins[2], OUTPUT);
                ::digitalWrite(_channelPins[2], LOW);

                if (isValidPin(_channelPins[3])) {
                    pinMode(_channelPins[3], OUTPUT);
                    ::digitalWrite(_channelPins[3], LOW);
                }
            }
        }
    }
    _channelSelect = 0;
}

void HydroPinMuxer::selectChannel(uint8_t channelNumber)
{
    if (_channelSelect != channelNumber) {
        // While we could be a bit smarter about which muxers we disable, storing that
        // wouldn't necessarily be worth the gain. The assumption is all that muxers in
        // system occupy the same channel select bus, even if that isn't the case.
        if (getController()) { getController()->deactivatePinMuxers(); }

        if (isValidPin(_channelPins[0])) {
            ::digitalWrite(_channelPins[0], (channelNumber >> 0) & 1 ? HIGH : LOW);

            if (isValidPin(_channelPins[1])) {
                ::digitalWrite(_channelPins[1], (channelNumber >> 1) & 1 ? HIGH : LOW);

                if (isValidPin(_channelPins[2])) {
                    ::digitalWrite(_channelPins[2], (channelNumber >> 2) & 1 ? HIGH : LOW);

                    if (isValidPin(_channelPins[3])) {
                        ::digitalWrite(_channelPins[3], (channelNumber >> 3) & 1 ? HIGH : LOW);
                    }
                }
            }
        }
        _channelSelect = channelNumber;
    }
}

void HydroPinMuxer::setIsActive(bool isActive)
{
    if (isActive) {
        _signal.init();
        _chipEnable.activate();
    } else {
        _chipEnable.deactivate();
        _signal.deinit();
    }
}

#ifndef HYDRO_DISABLE_MULTITASKING

HydroPinExpander::HydroPinExpander()
    : _channelBits(0), _ioRef(nullptr)
{ ; }

HydroPinExpander::HydroPinExpander(uint8_t channelBits, IoAbstractionRef ioRef)
    : _channelBits(channelBits), _ioRef(ioRef)
{ ; }

bool HydroPinExpander::syncChannel()
{
    return _ioRef->sync();
}

#endif // /ifndef HYDRO_DISABLE_MULTITASKING
