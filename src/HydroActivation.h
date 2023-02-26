/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Activations
*/

#ifndef HydroActivation_H
#define HydroActivation_H

struct HydroActivation;
struct HydroActivationHandle;

#include "Hydruino.h"

// Activation Flags
enum Hydro_ActivationFlags : unsigned char {
    Hydro_ActivationFlags_Forced        = 0x01,             // Force enable / ignore cursory canEnable checks
    Hydro_ActivationFlags_None          = 0x00              // Placeholder
};

// Activation Data
// Activation setup data that is its own object for ease of use. Used to define what
// encapsulates an activation.
struct HydroActivation {
    Hydro_DirectionMode direction;                          // Normalized driving direction
    float intensity;                                        // Normalized driving intensity ([0.0,1.0])
    millis_t duration;                                      // Duration time remaining, in milliseconds, else -1 for non-diminishing/unlimited or 0 for finished
    Hydro_ActivationFlags flags;                            // Activation flags

    inline HydroActivation(Hydro_DirectionMode directionIn, float intensityIn, millis_t durationIn, Hydro_ActivationFlags flagsIn) : direction(directionIn), intensity(constrain(intensityIn, 0.0f, 1.0f)), duration(durationIn), flags(flagsIn) { ; }
    inline HydroActivation() : HydroActivation(Hydro_DirectionMode_Undefined, 0.0f, 0, Hydro_ActivationFlags_None) { ; }

    inline bool isValid() const { return direction != Hydro_DirectionMode_Undefined; }
    inline bool isDone() const { return duration == millis_none; }
    inline bool isUntimed() const { return duration == -1; }
    inline bool isForced() const { return flags & Hydro_ActivationFlags_Forced; }
    inline float getDriveIntensity() const { return direction == Hydro_DirectionMode_Forward ? intensity :
                                                    direction == Hydro_DirectionMode_Reverse ? -intensity : 0.0f; }
};

// Activation Handle
// Since actuators are shared objects, those wishing to enable any actuator must receive
// a valid handle. Actuators may customize how they handle multiple activation handles.
// Handles represent a driving intensity value ranged [0,1] or [-1,1] depending on the
// capabilities of the attached actuator. Handles do not guarantee activation unless their
// forced flag is set (also see Actuator activation signal), but can be set up to ensure
// actuators are enabled for a specified duration, which is able to be async updated.
struct HydroActivationHandle {
    SharedPtr<HydroActuator> actuator;                      // Actuator owner, set only when activation requested (use operator= to set)
    HydroActivation activation;                             // Activation data
    millis_t checkTime;                                     // Last check timestamp, in milliseconds, else 0 for not started
    millis_t elapsed;                                       // Elapsed time accumulator, in milliseconds, else 0

    // Handle constructor that specifies a normalized enablement, ranged: [0.0,1.0] for specified direction
    HydroActivationHandle(SharedPtr<HydroActuator> actuator, Hydro_DirectionMode direction, float intensity = 1.0f, millis_t duration = -1, bool force = false);

    // Default constructor for empty handles
    inline HydroActivationHandle() : HydroActivationHandle(nullptr, Hydro_DirectionMode_Undefined, 0.0f, 0, false) { ; }
    HydroActivationHandle(const HydroActivationHandle &handle);
    ~HydroActivationHandle();
    HydroActivationHandle &operator=(SharedPtr<HydroActuator> actuator);
    inline HydroActivationHandle &operator=(const HydroActivation &activationIn) { activation = activationIn; return *this; }
    inline HydroActivationHandle &operator=(const HydroActivationHandle &handle) { activation = handle.activation; return operator=(handle.actuator); }

    // Disconnects activation from an actuator (removes handle reference from actuator)
    void unset();

    // Elapses activation by delta, updating relevant activation values
    void elapseBy(millis_t delta);
    inline void elapseTo(millis_t time = nzMillis()) { elapseBy(time - checkTime); }

    inline bool isActive() const { return actuator && isValidTime(checkTime); }
    inline bool isValid() const { return activation.isValid(); }
    inline bool isDone() const { return activation.isDone(); }
    inline bool isUntimed() const { return activation.isUntimed(); }
    inline bool isForced() const { return activation.isForced(); }

    inline millis_t getTimeLeft() const { return activation.duration; }
    inline millis_t getTimeActive(millis_t time = nzMillis()) const { return isActive() ? (time - checkTime) + elapsed : elapsed; }

    // De-normalized driving intensity value [-1.0,1.0]
    inline float getDriveIntensity() const { return activation.getDriveIntensity(); }
};

#endif // /ifndef HydroActivation_H
