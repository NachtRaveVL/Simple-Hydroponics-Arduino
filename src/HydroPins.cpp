/*  Hydruino: Simple automation controller for solar tracking systems.
    Copyright (C) 2023 NachtRaveVL          <nachtravevl@gmail.com>
    Hydruino Pins
*/

#include "Hydruino.h"

HydroPin *newPinObjectFromSubData(const HydroPinData *dataIn)
{
    if (dataIn && dataIn->type == -1) return nullptr;
    HYDRO_SOFT_ASSERT(dataIn && dataIn->type >= 0, SFP(HStr_Err_InvalidParameter));

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
    : type(Unknown), pin(-1), mode(Hydro_PinMode_Undefined), channel(-1)
{ ; }

HydroPin::HydroPin(int classType, pintype_t pinNumber, Hydro_PinMode pinMode, uint8_t muxChannel)
    : type((typeof(type))classType), pin(pinNumber), mode(pinMode), channel(muxChannel)
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
            switch (mode) {
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

                case Hydro_PinMode_Digital_Input_Floating:
                case Hydro_PinMode_Analog_Input:
                    pinMode(pin, INPUT);
                    break;

                case Hydro_PinMode_Digital_Output_OpenDrain:
                case Hydro_PinMode_Digital_Output_PushPull:
                case Hydro_PinMode_Analog_Output:
                    pinMode(pin, OUTPUT);
                    break;

                default:
                    break;
            }
        }
    #endif
}

bool HydroPin::tryEnableMuxer()
{
    #if !HYDRO_SYS_DRY_RUN_ENABLE
        if (isValid() && isMuxed()) {
            SharedPtr<HydroPinMuxer> muxer = getHydroInstance() ? getHydroInstance()->getPinMuxer(pin) : nullptr;
            if (muxer) {
                muxer->selectChannel(channel);
                return true;
            }
        }
        return false;
    #else
        return isValid() && isMuxed();
    #endif
}


HydroDigitalPin::HydroDigitalPin()
    : HydroPin(Digital)
{ ; }

HydroDigitalPin::HydroDigitalPin(pintype_t pinNumber, Arduino_PinModeType pinMode, uint8_t muxChannel)
    : HydroPin(Digital, pinNumber, pinMode != OUTPUT ? (pinMode != INPUT ? (pinMode == INPUT_PULLUP ? Hydro_PinMode_Digital_Input_PullUp : Hydro_PinMode_Digital_Input_PullDown)
                                                                         : Hydro_PinMode_Digital_Input_Floating)
                                                     : (pinMode == OUTPUT ? Hydro_PinMode_Digital_Output_OpenDrain : Hydro_PinMode_Digital_Output_PushPull), muxChannel),
      activeLow(pinMode == INPUT || pinMode == INPUT_PULLUP || pinMode == OUTPUT)
{ ; }

HydroDigitalPin::HydroDigitalPin(pintype_t pinNumber, Hydro_PinMode pinMode, uint8_t muxChannel)
    : HydroPin(Digital, pinNumber, pinMode, muxChannel),
      activeLow(pinMode == Hydro_PinMode_Digital_Input_Floating ||
                pinMode == Hydro_PinMode_Digital_Input_PullUp ||
                pinMode == Hydro_PinMode_Digital_Output_OpenDrain)
{ ; }

HydroDigitalPin::HydroDigitalPin(pintype_t pinNumber, Arduino_PinModeType pinMode, bool isActiveLow, uint8_t muxChannel)
    : HydroPin(Digital, pinNumber, pinMode != OUTPUT ? (isActiveLow ? Hydro_PinMode_Digital_Input_PullUp : Hydro_PinMode_Digital_Input_PullDown)
                                                     : (isActiveLow ? Hydro_PinMode_Digital_Output_OpenDrain : Hydro_PinMode_Digital_Output_PushPull), muxChannel),
      activeLow(isActiveLow)
{ ; }

HydroDigitalPin::HydroDigitalPin(pintype_t pinNumber, Hydro_PinMode pinMode, bool isActiveLow, uint8_t muxChannel)
    : HydroPin(Digital, pinNumber, pinMode, muxChannel),
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

Arduino_PinStatusType HydroDigitalPin::digitalRead()
{
    #if !HYDRO_SYS_DRY_RUN_ENABLE
        if (isValid() && (!isMuxed() || tryEnableMuxer())) {
            return ::digitalRead(pin);
        }
    #endif
    return (Arduino_PinStatusType)-1;
}

void HydroDigitalPin::digitalWrite(Arduino_PinStatusType status)
{
    #if !HYDRO_SYS_DRY_RUN_ENABLE
        if (isValid() && (!isMuxed() || tryEnableMuxer())) {
            ::digitalWrite(pin, status);
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

HydroAnalogPin::HydroAnalogPin(pintype_t pinNumber, Arduino_PinModeType pinMode, uint8_t analogBitRes,
#ifdef ESP32
                               uint8_t pinPWMChannel,
#endif
#ifdef ESP_PLATFORM
                               float pinPWMFrequency,
#endif
                               uint8_t muxChannel)
    : HydroPin(Analog, pinNumber, pinMode != OUTPUT ? Hydro_PinMode_Analog_Input : Hydro_PinMode_Analog_Output, muxChannel),
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
                               uint8_t muxChannel)
    : HydroPin(Analog, pinNumber, pinMode, muxChannel),
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
      , pinPWMChannel(dataIn->dataAs.analogPin.pwmChannel)
#endif
#ifdef ESP_PLATFORM
      , pinPWMFrequency(dataIn->dataAs.analogPin.pwmFrequency)
#endif
{ ; }

void HydroAnalogPin::init()
{
    #if !HYDRO_SYS_DRY_RUN_ENABLE
        if (isValid()) {
            HydroPin::init();
            #ifdef ESP32
                ledcAttachPin(pin, pwmChannel);
                ledcSetup(pwmChannel, pwmFrequency, bitRes.bits);
            #endif
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
        if (isValid() && (!isMuxed() || tryEnableMuxer())) {
            #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
                analogReadResolution(bitRes.bits);
            #endif
            return ::analogRead(pin);
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
        if (isValid() && (!isMuxed() || tryEnableMuxer())) {
            #ifdef ESP32
                ledcWrite(pwmChannel, val);
            #else
                #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
                    analogWriteResolution(bitRes.bits);
                #elif defined(ESP8266)
                    analogWriteRange(bitRes.maxVal);
                    analogWriteFreq(pwmFrequency);
                #endif
                ::analogWrite(pin, amount);
            #endif
        }
    #endif
}


HydroPinData::HydroPinData()
    : HydroSubData((int8_t)HydroPin::Unknown), pin(-1), mode(Hydro_PinMode_Undefined), channel(-1), dataAs{0}
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
    : _signal(), _chipEnable(), _channelPins{-1,-1,-1,-1,-1}, _channelSelect(0)
{ ; }

HydroPinMuxer::HydroPinMuxer(HydroPin signalPin,
                             pintype_t *muxChannelPins, uint8_t muxChannelBits,
                             HydroDigitalPin chipEnablePin)
    : _signal(signalPin), _chipEnable(chipEnablePin),
      _channelPins{ muxChannelBits > 0 ? muxChannelPins[0] : -1,
                    muxChannelBits > 1 ? muxChannelPins[1] : -1,
                    muxChannelBits > 2 ? muxChannelPins[2] : -1,
                    muxChannelBits > 3 ? muxChannelPins[3] : -1,
                    muxChannelBits > 4 ? muxChannelPins[4] : -1 },
      _channelBits(muxChannelBits), _channelSelect(0)
{
    _signal.channel = -1; // unused
}

HydroPinMuxer::HydroPinMuxer(const HydroPinMuxerData *dataIn)
    : _signal(&dataIn->signal), _chipEnable(&dataIn->chipEnable),
      _channelPins{ dataIn->channelPins[0],
                    dataIn->channelPins[1],
                    dataIn->channelPins[2],
                    dataIn->channelPins[3],
                    dataIn->channelPins[4] },
      _channelBits(dataIn->channelBits), _channelSelect(0)
{
    _signal.channel = -1; // unused
}

void HydroPinMuxer::saveToData(HydroPinMuxerData *dataOut) const
{
    _signal.saveToData(&dataOut->signal);
    _chipEnable.saveToData(&dataOut->chipEnable);
    dataOut->channelPins[0] = _channelPins[0];
    dataOut->channelPins[1] = _channelPins[1];
    dataOut->channelPins[2] = _channelPins[2];
    dataOut->channelPins[3] = _channelPins[3];
    dataOut->channelPins[4] = _channelPins[4];
    dataOut->channelBits = _channelBits;
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

                    if (isValidPin(_channelPins[4])) {
                        pinMode(_channelPins[4], OUTPUT);
                        ::digitalWrite(_channelPins[4], LOW);
                    }
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
        if (getHydroInstance()) { getHydroInstance()->deselectPinMuxers(); }

        if (isValidPin(_channelPins[0])) {
            ::digitalWrite(_channelPins[0], (channelNumber >> 0) & 1 ? HIGH : LOW);

            if (isValidPin(_channelPins[1])) {
                ::digitalWrite(_channelPins[1], (channelNumber >> 1) & 1 ? HIGH : LOW);

                if (isValidPin(_channelPins[2])) {
                    ::digitalWrite(_channelPins[2], (channelNumber >> 2) & 1 ? HIGH : LOW);

                    if (isValidPin(_channelPins[3])) {
                        ::digitalWrite(_channelPins[3], (channelNumber >> 3) & 1 ? HIGH : LOW);

                        if (isValidPin(_channelPins[4])) {
                            ::digitalWrite(_channelPins[4], (channelNumber >> 4) & 1 ? HIGH : LOW);
                        }
                    }
                }
            }
        }
        _channelSelect = channelNumber;
    }

    _signal.init();
    _chipEnable.activate();
}

void HydroPinMuxer::deselect()
{
    _chipEnable.deactivate();
    _signal.deinit();
}


HydroPinMuxerData::HydroPinMuxerData()
    : HydroSubData(0), signal(), chipEnable(), channelPins{-1,-1,-1,-1,-1}, channelBits(0)
{ ; }

void HydroPinMuxerData::toJSONObject(JsonObject &objectOut) const
{
    //HydroSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    if (isValidPin(signal.pin)) {
        JsonObject signalPinObj = objectOut.createNestedObject(SFP(HStr_Key_SignalPin));
        signal.toJSONObject(signalPinObj);
    }
    if (isValidPin(chipEnable.pin)) {
        JsonObject chipEnablePinObj = objectOut.createNestedObject(SFP(HStr_Key_ChipEnablePin));
        chipEnable.toJSONObject(chipEnablePinObj);
    }
    if (channelBits && isValidPin(channelPins[0])) {
        objectOut[SFP(HStr_Key_ChannelPins)] = commaStringFromArray(channelPins, channelBits);
    }
}

void HydroPinMuxerData::fromJSONObject(JsonObjectConst &objectIn)
{
    //HydroSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    JsonObjectConst signalPinObj = objectIn[SFP(HStr_Key_SignalPin)];
    if (!signalPinObj.isNull()) { signal.fromJSONObject(signalPinObj); }
    JsonObjectConst chipEnablePinObj = objectIn[SFP(HStr_Key_ChipEnablePin)];
    if (!chipEnablePinObj.isNull()) { chipEnable.fromJSONObject(chipEnablePinObj); }
    JsonVariantConst channelPinsVar = objectIn[SFP(HStr_Key_ChannelPins)];
    commaStringToArray(channelPinsVar, channelPins, 5);
    for (channelBits = 0; channelBits < 5 && isValidPin(channelPins[channelBits]); ++channelBits) { ; }
}
