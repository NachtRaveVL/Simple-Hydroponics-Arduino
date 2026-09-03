#ifndef RTCLIB_H
#define RTCLIB_H
#include "Arduino.h"
#include "TimeLib.h"
class TimeSpan;
class DateTime {
public:
    enum timestampOpt { TIMESTAMP_FULL, TIMESTAMP_TIME, TIMESTAMP_DATE };
    DateTime():t_(0){}
    explicit DateTime(uint32_t t):t_(t){}
    DateTime(uint16_t y,uint8_t m,uint8_t d,uint8_t hh=0,uint8_t mm=0,uint8_t ss=0){std::tm tm{};tm.tm_year=y-1900;tm.tm_mon=m-1;tm.tm_mday=d;tm.tm_hour=hh;tm.tm_min=mm;tm.tm_sec=ss;t_=(uint32_t)timegm(&tm);}
    uint16_t year() const{return parts().tm_year+1900;} uint8_t month() const{return parts().tm_mon+1;} uint8_t day() const{return parts().tm_mday;} uint8_t hour() const{return parts().tm_hour;} uint8_t minute() const{return parts().tm_min;} uint8_t second() const{return parts().tm_sec;} uint8_t dayOfTheWeek() const{return parts().tm_wday;}
    uint32_t unixtime() const{return t_;}
    String timestamp(timestampOpt opt=TIMESTAMP_FULL) const {char b[32]; auto p=parts(); if(opt==TIMESTAMP_TIME) std::snprintf(b,sizeof(b),"%02d:%02d:%02d",p.tm_hour,p.tm_min,p.tm_sec); else if(opt==TIMESTAMP_DATE) std::snprintf(b,sizeof(b),"%04d-%02d-%02d",p.tm_year+1900,p.tm_mon+1,p.tm_mday); else std::snprintf(b,sizeof(b),"%04d-%02d-%02dT%02d:%02d:%02d",p.tm_year+1900,p.tm_mon+1,p.tm_mday,p.tm_hour,p.tm_min,p.tm_sec); return String(b);}
    bool isValid() const{return true;}
    friend bool operator==(const DateTime&a,const DateTime&b){return a.t_==b.t_;} friend bool operator!=(const DateTime&a,const DateTime&b){return a.t_!=b.t_;} friend bool operator<(const DateTime&a,const DateTime&b){return a.t_<b.t_;} friend bool operator<=(const DateTime&a,const DateTime&b){return a.t_<=b.t_;} friend bool operator>(const DateTime&a,const DateTime&b){return a.t_>b.t_;} friend bool operator>=(const DateTime&a,const DateTime&b){return a.t_>=b.t_;}
    DateTime operator+(const TimeSpan&) const; DateTime operator-(const TimeSpan&) const; TimeSpan operator-(const DateTime&) const;
private: uint32_t t_; std::tm parts() const {time_t tt=t_; std::tm x{}; gmtime_r(&tt,&x); return x;}
};
class TimeSpan { public: explicit TimeSpan(int32_t seconds=0):s_(seconds){} TimeSpan(int16_t days,int8_t hours,int8_t minutes,int8_t seconds):s_((int32_t)days*86400L+(int32_t)hours*3600L+(int32_t)minutes*60L+seconds){} int16_t days() const{return s_/86400L;} int8_t hours() const{return (s_%86400L)/3600L;} int8_t minutes() const{return (s_%3600L)/60L;} int8_t seconds() const{return s_%60L;} int32_t totalseconds() const{return s_;} private:int32_t s_; friend class DateTime;};
inline DateTime DateTime::operator+(const TimeSpan&s)const{return DateTime((uint32_t)((int64_t)t_+s.s_));}
inline DateTime DateTime::operator-(const TimeSpan&s)const{return DateTime((uint32_t)((int64_t)t_-s.s_));}
inline TimeSpan DateTime::operator-(const DateTime&o)const{return TimeSpan((int32_t)(t_-o.t_));}
class RTC_DS1307 { public: bool begin(TwoWire * = nullptr){return true;} bool isrunning(){return true;} void adjust(const DateTime&d){dt_=d;} DateTime now(){return dt_.unixtime()?dt_:DateTime((uint32_t)std::time(nullptr));} protected: DateTime dt_;};
class RTC_DS3231 { public: bool begin(TwoWire * = nullptr){return true;} bool lostPower(){return false;} void adjust(const DateTime&d){dt_=d;} DateTime now(){return dt_.unixtime()?dt_:DateTime((uint32_t)std::time(nullptr));} protected: DateTime dt_;};
class RTC_PCF8523 : public RTC_DS3231 { public: bool initialized(){return true;} bool lostPower(){return false;} };
class RTC_PCF8563 : public RTC_DS3231 {};
#endif
