#ifndef SD_H
#define SD_H
#include "Arduino.h"
#include <fstream>
#ifndef FILE_READ
#define FILE_READ 0
#endif
#ifndef FILE_WRITE
#define FILE_WRITE 1
#endif
class File : public Stream {
public:
    File()=default; explicit File(bool ok):ok_(ok){}
    operator bool() const {return ok_;}
    int available() override {return 0;}
    int read() override {return -1;}
    size_t write(uint8_t) override {return 1;}
    size_t write(const uint8_t*,size_t n) override {return n;}
    bool seek(uint32_t){return true;}
    uint32_t position() const{return 0;}
    uint32_t size() const{return 0;}
    void close(){ok_=false;}
    void flush() override {}
    const char* name() const{return "";}
private: bool ok_=false;
};
class SDClass { public: bool begin(uint8_t=0){return true;} bool begin(uint32_t,uint8_t){return true;} void end(){} bool exists(const char*){return false;} bool exists(const String&s){return exists(s.c_str());} bool mkdir(const char*){return true;} bool mkdir(const String&s){return mkdir(s.c_str());} bool remove(const char*){return true;} bool remove(const String&s){return remove(s.c_str());} File open(const char*, uint8_t=FILE_READ){return File(true);} File open(const String&s,uint8_t mode=FILE_READ){return open(s.c_str(),mode);} };
inline SDClass SD;
#endif
