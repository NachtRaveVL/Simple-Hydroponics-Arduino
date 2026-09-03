#ifndef ARDUINO_H
#define ARDUINO_H
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <limits>
#include <fstream>
#include <variant>

using std::size_t;
typedef uint8_t byte;
typedef bool boolean;
typedef unsigned int word;
typedef uint8_t pin_size_t;
typedef int PinStatus;
typedef int PinMode;

#ifndef HIGH
#define HIGH 0x1
#endif
#ifndef LOW
#define LOW 0x0
#endif
#ifndef INPUT
#define INPUT 0x0
#endif
#ifndef OUTPUT
#define OUTPUT 0x1
#endif
#ifndef INPUT_PULLUP
#define INPUT_PULLUP 0x2
#endif
#ifndef INPUT_PULLDOWN
#define INPUT_PULLDOWN 0x3
#endif
#ifndef CHANGE
#define CHANGE 1
#endif
#ifndef FALLING
#define FALLING 2
#endif
#ifndef RISING
#define RISING 3
#endif
#ifndef SERIAL_8N1
#define SERIAL_8N1 0
#endif
#ifndef LSBFIRST
#define LSBFIRST 0
#endif
#ifndef MSBFIRST
#define MSBFIRST 1
#endif
#ifndef SPI_MODE0
#define SPI_MODE0 0
#endif
#ifndef DEC
#define DEC 10
#endif
#ifndef HEX
#define HEX 16
#endif
#ifndef OCT
#define OCT 8
#endif
#ifndef BIN
#define BIN 2
#endif
#ifndef PI
#define PI 3.14159265358979323846
#endif
#ifndef HALF_PI
#define HALF_PI 1.57079632679489661923
#endif
#ifndef TWO_PI
#define TWO_PI 6.28318530717958647692
#endif
#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.01745329251994329577
#endif
#ifndef RAD_TO_DEG
#define RAD_TO_DEG 57.2957795130823208768
#endif
#ifndef PROGMEM
#define PROGMEM
#endif
#ifndef PGM_P
#define PGM_P const char *
#endif
#ifndef PSTR
#define PSTR(s) (s)
#endif
#ifndef FPSTR
#define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))
#endif
#ifndef F
#define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))
#endif
#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const uint8_t *)(addr))
#endif
#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const uint16_t *)(addr))
#endif
#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const uint32_t *)(addr))
#endif
#ifndef memcpy_P
#define memcpy_P memcpy
#endif
#ifndef strlen_P
#define strlen_P strlen
#endif
#ifndef strcmp_P
#define strcmp_P strcmp
#endif
#ifndef strcpy_P
#define strcpy_P strcpy
#endif
#ifndef strncpy_P
#define strncpy_P strncpy
#endif


class __FlashStringHelper;

class String {
public:
    String() = default;
    String(const char *s) : s_(s ? s : "") {}
    String(const __FlashStringHelper *s) : s_(s ? reinterpret_cast<const char *>(s) : "") {}
    String(const std::string &s) : s_(s) {}
    String(char c) : s_(1, c) {}
    String(unsigned char v, unsigned char base = 10) { assignIntegral(v, base); }
    String(int v, unsigned char base = 10) { assignIntegral(v, base); }
    String(unsigned int v, unsigned char base = 10) { assignIntegral(v, base); }
    String(long v, unsigned char base = 10) { assignIntegral(v, base); }
    String(unsigned long v, unsigned char base = 10) { assignIntegral(v, base); }
    String(long long v, unsigned char base = 10) { assignIntegral(v, base); }
    String(unsigned long long v, unsigned char base = 10) { assignIntegral(v, base); }
    String(float v, unsigned char decimals = 2) { assignFloat(v, decimals); }
    String(double v, unsigned char decimals = 2) { assignFloat(v, decimals); }

    const char *c_str() const { return s_.c_str(); }
    unsigned int length() const { return static_cast<unsigned int>(s_.size()); }
    bool reserve(unsigned int n) { s_.reserve(n); return true; }
    long toInt() const { try { return std::stol(s_); } catch (...) { return 0; } }
    float toFloat() const { try { return std::stof(s_); } catch (...) { return 0.0f; } }
    double toDouble() const { try { return std::stod(s_); } catch (...) { return 0.0; } }
    char charAt(unsigned int i) const { return i < s_.size() ? s_[i] : '\0'; }
    char operator[](unsigned int i) const { return charAt(i); }
    char &operator[](unsigned int i) { return s_[i]; }

    bool concat(const String &other) { s_ += other.s_; return true; }
    bool concat(const char *other) { if (other) s_ += other; return true; }
    bool concat(char c) { s_ += c; return true; }
    bool concat(const __FlashStringHelper *other) { if (other) s_ += reinterpret_cast<const char *>(other); return true; }

    String &operator+=(const String &other) { s_ += other.s_; return *this; }
    String &operator+=(const char *other) { if (other) s_ += other; return *this; }
    String &operator+=(const __FlashStringHelper *other) { if (other) s_ += reinterpret_cast<const char *>(other); return *this; }
    String &operator+=(char c) { s_ += c; return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    String &operator+=(T v) { s_ += String(v).s_; return *this; }

    bool equals(const String &other) const { return s_ == other.s_; }
    bool equals(const char *other) const { return s_ == (other ? other : ""); }
    bool equalsIgnoreCase(const String &other) const {
        if (s_.size() != other.s_.size()) return false;
        for (size_t i=0;i<s_.size();++i) if (std::tolower((unsigned char)s_[i]) != std::tolower((unsigned char)other.s_[i])) return false;
        return true;
    }
    bool equalsIgnoreCase(const char *other) const { return equalsIgnoreCase(String(other)); }
    void toLowerCase() { for (char &c : s_) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
    int indexOf(char c, unsigned int fromIndex = 0) const {
        auto p = s_.find(c, fromIndex); return p == std::string::npos ? -1 : static_cast<int>(p);
    }
    int indexOf(const String &sub, unsigned int fromIndex = 0) const {
        auto p = s_.find(sub.s_, fromIndex); return p == std::string::npos ? -1 : static_cast<int>(p);
    }
    int lastIndexOf(char c) const { auto p=s_.rfind(c); return p==std::string::npos?-1:(int)p; }
    int lastIndexOf(const String &sub) const { auto p=s_.rfind(sub.s_); return p==std::string::npos?-1:(int)p; }
    String substring(unsigned int from, unsigned int to) const {
        if (from > s_.size()) from = s_.size(); if (to > s_.size()) to=s_.size(); if (to < from) std::swap(to,from);
        return String(s_.substr(from,to-from));
    }
    String substring(unsigned int from) const { return from < s_.size() ? String(s_.substr(from)) : String(); }
    void remove(unsigned int index) { if(index<s_.size()) s_.erase(index); }
    void remove(unsigned int index, unsigned int count) { if(index<s_.size()) s_.erase(index,count); }
    void replace(const String &find, const String &repl) {
        if(find.s_.empty()) return; size_t p=0; while((p=s_.find(find.s_,p))!=std::string::npos){s_.replace(p,find.s_.size(),repl.s_);p+=repl.s_.size();}
    }

    explicit operator bool() const { return !s_.empty(); }
    operator std::string() const { return s_; }

    friend bool operator==(const String&a,const String&b){return a.s_==b.s_;}
    friend bool operator!=(const String&a,const String&b){return !(a==b);}
    friend bool operator==(const String&a,const char*b){return a.equals(b);}
    friend bool operator==(const char*a,const String&b){return b.equals(a);}
    friend bool operator!=(const String&a,const char*b){return !a.equals(b);}
    friend bool operator<(const String&a,const String&b){return a.s_<b.s_;}
    friend String operator+(String a,const String&b){a+=b;return a;}
    friend String operator+(String a,const char*b){a+=b;return a;}
    friend String operator+(const char*a,const String&b){String r(a);r+=b;return r;}
    friend String operator+(String a,const __FlashStringHelper*b){a+=b;return a;}

private:
    std::string s_;
    template<typename T> void assignIntegral(T v, unsigned char base) {
        if (base == 10) { s_ = std::to_string(v); return; }
        bool neg = false; unsigned long long u;
        if constexpr (std::is_signed_v<T>) { if (v < 0) { neg=true; u=(unsigned long long)(-(long long)v); } else u=(unsigned long long)v; }
        else u=(unsigned long long)v;
        if (base < 2 || base > 36) base = 10;
        static const char digs[]="0123456789abcdefghijklmnopqrstuvwxyz"; std::string out; do { out.push_back(digs[u%base]); u/=base; } while(u); if(neg) out.push_back('-'); std::reverse(out.begin(),out.end()); s_=out;
    }
    void assignFloat(double v, unsigned char decimals) { std::ostringstream os; os<<std::fixed<<std::setprecision(decimals)<<v; s_=os.str(); }
};

class Print {
public:
    virtual ~Print() = default;
    virtual size_t write(uint8_t) { return 1; }
    virtual size_t write(const uint8_t *buffer, size_t size) { (void)buffer; return size; }
    size_t write(const char *s) { return s ? write(reinterpret_cast<const uint8_t*>(s), strlen(s)) : 0; }
    size_t print(const String &s) { return write(reinterpret_cast<const uint8_t*>(s.c_str()), s.length()); }
    size_t print(const char *s) { return write(s); }
    size_t print(const __FlashStringHelper *s) { return print(reinterpret_cast<const char*>(s)); }
    size_t print(char c) { return write((uint8_t)c); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    size_t print(T v) { return print(String(v)); }
    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    size_t print(T v, int base) { return print(String(v, (unsigned char)base)); }
    size_t println() { return print("\n"); }
    template<typename T> size_t println(const T &v) { size_t n=print(v); n+=println(); return n; }
};

class Stream : public Print {
public:
    virtual int available() { return 0; }
    virtual int read() { return -1; }
    virtual int peek() { return -1; }
    virtual void flush() {}
    virtual int availableForWrite() { return 0; }
    virtual size_t readBytes(char *buffer, size_t length) { size_t n=0; while(n<length && available()){int c=read(); if(c<0) break; buffer[n++]=(char)c;} return n; }
    virtual size_t readBytes(uint8_t *buffer, size_t length) { return readBytes(reinterpret_cast<char*>(buffer), length); }
    virtual size_t readBytesUntil(char terminator, char *buffer, size_t length) { size_t n=0; while(n<length && available()){int c=read(); if(c<0||c==terminator) break; buffer[n++]=(char)c;} return n; }
};

class HardwareSerial : public Stream {
public:
    void begin(unsigned long, int = SERIAL_8N1) {}
    operator bool() const { return true; }
    size_t write(uint8_t c) override { std::cout.put((char)c); return 1; }
    size_t write(const uint8_t *buffer, size_t size) override { std::cout.write(reinterpret_cast<const char*>(buffer), size); return size; }
};
inline HardwareSerial Serial;
inline HardwareSerial Serial1;

inline uint32_t millis() {
    static auto start=std::chrono::steady_clock::now();
    return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count();
}
inline uint32_t micros() {
    static auto start=std::chrono::steady_clock::now();
    return (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-start).count();
}
inline void delay(unsigned long ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
inline void delayMicroseconds(unsigned int us) { std::this_thread::sleep_for(std::chrono::microseconds(us)); }
inline void yield() {}
inline void noTone(uint8_t) {}
inline void tone(uint8_t, unsigned int, unsigned long = 0) {}
inline uint8_t *hostDigitalPinStates() { static uint8_t states[256] = {}; return states; }
inline void pinMode(uint8_t, uint8_t) {}
inline int digitalRead(uint8_t pin) { return hostDigitalPinStates()[pin]; }
inline void digitalWrite(uint8_t pin, uint8_t value) { hostDigitalPinStates()[pin] = value; }
inline int analogRead(uint8_t) { return 0; }
inline void analogWrite(uint8_t, int) {}
inline void analogReadResolution(int) {}
inline void analogWriteResolution(int) {}
inline int digitalPinToInterrupt(uint8_t pin) { return pin; }
inline void attachInterrupt(int, void (*)(), int) {}
inline void detachInterrupt(int) {}
inline long random(long max) { return max > 0 ? std::rand()%max : 0; }
inline long random(long min, long max) { return max>min ? min+random(max-min) : min; }
inline void randomSeed(unsigned long seed) { std::srand((unsigned)seed); }


#ifndef constrain
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
inline long map(long x,long in_min,long in_max,long out_min,long out_max){return (x-in_min)*(out_max-out_min)/(in_max-in_min)+out_min;}

inline char *dtostrf(double val, signed char width, unsigned char prec, char *sout) { std::snprintf(sout, 64, "%*.*f", width, prec, val); return sout; }
inline char *ultoa(unsigned long value, char *str, int base) { String s(value,base); std::strcpy(str,s.c_str()); return str; }
inline char *ltoa(long value, char *str, int base) { String s(value,base); std::strcpy(str,s.c_str()); return str; }

#ifndef A0
#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19
#endif

inline char *__brkval = nullptr;

#endif
