#ifndef SOLAR_CALCULATOR_H
#define SOLAR_CALCULATOR_H
#include <cmath>
#include <ctime>
class JulianDay
{
public:
    explicit JulianDay(std::time_t unixTime) : _value(2440587.5 + static_cast<double>(unixTime) / 86400.0) { ; }
    operator double() const { return _value; }
private:
    double _value;
};

#ifndef SUNRISESET_STD_ALTITUDE
#define SUNRISESET_STD_ALTITUDE -0.833
#endif
inline void calcHorizontalCoordinates(double, double, double, double &azimuth, double &elevation)
{
    azimuth = 180.0;
    elevation = 45.0;
}
inline void calcEquatorialCoordinates(const JulianDay &, double &rightAscension, double &declination, double &radius)
{
    rightAscension = 0.0;
    declination = 0.0;
    radius = 1.0;
}
inline void calcSunriseSunset(unsigned long time, double, double, double &transit, double &sunrise, double &sunset,
                              double = SUNRISESET_STD_ALTITUDE, int = 3)
{
    transit = static_cast<double>(time) + 12.0 * 60.0 * 60.0;
    sunrise = static_cast<double>(time) + 6.0 * 60.0 * 60.0;
    sunset = static_cast<double>(time) + 18.0 * 60.0 * 60.0;
}
#endif
