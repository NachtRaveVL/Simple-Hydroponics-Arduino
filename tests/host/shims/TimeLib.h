#ifndef TIME_LIB_H
#define TIME_LIB_H
#include "Arduino.h"
#define SECS_PER_MIN 60UL
#define SECS_PER_HOUR 3600UL
#define SECS_PER_DAY 86400UL
#define SECS_PER_WEEK 604800UL
#define DAYS_PER_WEEK 7
#define SECS_YR_2000 946684800UL
#define CalendarYrToTm(Y) ((Y)-1970)
#define tmYearToCalendar(Y) ((Y)+1970)
typedef struct { uint8_t Second,Minute,Hour,Wday,Day,Month; uint8_t Year; } tmElements_t;
inline time_t __time_now = 0;
inline void setTime(time_t t){__time_now=t;}
using getExternalTime = time_t (*)();
inline void setSyncProvider(getExternalTime provider){ if (provider) __time_now=provider(); }
inline void setTime(int hr,int min,int sec,int day,int mon,int yr){std::tm tm{}; tm.tm_year=yr-1900; tm.tm_mon=mon-1; tm.tm_mday=day; tm.tm_hour=hr; tm.tm_min=min; tm.tm_sec=sec; __time_now=timegm(&tm);}
inline time_t now(){return __time_now?__time_now:std::time(nullptr);}
inline void breakTime(time_t t, tmElements_t &tm){std::tm out{}; gmtime_r(&t,&out); tm.Second=out.tm_sec;tm.Minute=out.tm_min;tm.Hour=out.tm_hour;tm.Wday=out.tm_wday+1;tm.Day=out.tm_mday;tm.Month=out.tm_mon+1;tm.Year=(uint8_t)(out.tm_year);}
inline time_t makeTime(const tmElements_t &tm){std::tm in{};in.tm_sec=tm.Second;in.tm_min=tm.Minute;in.tm_hour=tm.Hour;in.tm_mday=tm.Day;in.tm_mon=tm.Month-1;in.tm_year=tm.Year;return timegm(&in);}
inline int year(time_t t=now()){std::tm x{};gmtime_r(&t,&x);return x.tm_year+1900;}
inline int month(time_t t=now()){std::tm x{};gmtime_r(&t,&x);return x.tm_mon+1;}
inline int day(time_t t=now()){std::tm x{};gmtime_r(&t,&x);return x.tm_mday;}
inline int hour(time_t t=now()){std::tm x{};gmtime_r(&t,&x);return x.tm_hour;}
inline int minute(time_t t=now()){std::tm x{};gmtime_r(&t,&x);return x.tm_min;}
inline int second(time_t t=now()){std::tm x{};gmtime_r(&t,&x);return x.tm_sec;}
inline int weekday(time_t t=now()){std::tm x{};gmtime_r(&t,&x);return x.tm_wday+1;}
#endif
