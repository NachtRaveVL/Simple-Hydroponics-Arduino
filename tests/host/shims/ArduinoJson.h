#ifndef ARDUINO_JSON_H
#define ARDUINO_JSON_H
#include "Arduino.h"
#include <variant>
#include <map>
#include <vector>
#include <type_traits>
#include <sstream>

namespace arduinojson_stub {
struct Node {
    using Object = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using Value = std::variant<std::monostate, bool, int64_t, double, std::string, Object, Array>;
    Value value;
    Node() : value(std::monostate{}) {}
};
inline std::string keyString(const String &s){ return std::string(s.c_str()); }
inline std::string keyString(const char *s){ return s ? std::string(s) : std::string(); }
inline std::string keyString(const __FlashStringHelper *s){ return s ? std::string(reinterpret_cast<const char*>(s)) : std::string(); }
}

class JsonObject;
class JsonObjectConst;
class JsonArrayConst;
class JsonVariantConst;
class JsonVariantRef;
using JsonVariant = JsonVariantRef;

class JsonVariantConst {
public:
    JsonVariantConst() = default;
    explicit JsonVariantConst(const arduinojson_stub::Node *n) : n_(n) {}
    bool isNull() const { return !n_ || std::holds_alternative<std::monostate>(n_->value); }

    template<class T> bool is() const;
    template<class T> T as() const;

    template<class T>
    T operator|(T defaultValue) const {
        if (isNull()) return defaultValue;
        if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, char*>) {
            const char *v = as<const char*>(); return v ? (T)v : defaultValue;
        } else {
            return as<T>();
        }
    }

    const char *operator|(std::nullptr_t) const {
        return isNull() ? nullptr : as<const char*>();
    }

    operator String() const { return as<String>(); }
    operator const char *() const { return as<const char*>(); }
    operator JsonObjectConst() const;
    operator JsonArrayConst() const;

protected:
    const arduinojson_stub::Node *n_ = nullptr;
    friend class JsonObjectConst;
    friend class JsonVariantRef;
    friend class JsonArrayConst;
};

class JsonVariantRef : public JsonVariantConst {
public:
    JsonVariantRef() = default;
    explicit JsonVariantRef(arduinojson_stub::Node *n) : JsonVariantConst(n), m_(n) {}

    JsonVariantRef &operator=(std::nullptr_t) { if(m_) m_->value=std::monostate{}; return *this; }
    JsonVariantRef &operator=(const String &v) { if(m_) m_->value=std::string(v.c_str()); return *this; }
    JsonVariantRef &operator=(const char *v) { if(m_) m_->value=v?std::string(v):std::string(); return *this; }
    JsonVariantRef &operator=(const __FlashStringHelper *v) { if(m_) m_->value=v?std::string(reinterpret_cast<const char*>(v)):std::string(); return *this; }
    JsonVariantRef &operator=(bool v) { if(m_) m_->value=v; return *this; }
    JsonVariantRef &operator=(float v) { if(m_) m_->value=(double)v; return *this; }
    JsonVariantRef &operator=(double v) { if(m_) m_->value=v; return *this; }
    template<class T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T,bool>, int> = 0>
    JsonVariantRef &operator=(T v) { if(m_) m_->value=(int64_t)v; return *this; }
    template<class T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
    JsonVariantRef &operator=(T v) { if(m_) m_->value=(int64_t)v; return *this; }

private:
    arduinojson_stub::Node *m_ = nullptr;
};

class JsonObjectConst {
public:
    JsonObjectConst() = default;
    explicit JsonObjectConst(const arduinojson_stub::Node *n) : n_(n && std::holds_alternative<arduinojson_stub::Node::Object>(n->value) ? n : nullptr) {}
    bool isNull() const { return n_ == nullptr; }
    size_t size() const { return n_ ? std::get<arduinojson_stub::Node::Object>(n_->value).size() : 0; }

    template<class K> JsonVariantConst operator[](const K &key) const {
        if(!n_) return JsonVariantConst();
        const auto &obj=std::get<arduinojson_stub::Node::Object>(n_->value);
        auto it=obj.find(arduinojson_stub::keyString(key));
        return it==obj.end()?JsonVariantConst():JsonVariantConst(&it->second);
    }
protected:
    const arduinojson_stub::Node *n_=nullptr;
    friend class JsonVariantConst;
};

class JsonObject {
public:
    JsonObject() = default;
    explicit JsonObject(arduinojson_stub::Node *n) : n_(n) { if(n_ && !std::holds_alternative<arduinojson_stub::Node::Object>(n_->value)) n_->value=arduinojson_stub::Node::Object{}; }
    bool isNull() const { return n_==nullptr; }
    size_t size() const { return n_ ? std::get<arduinojson_stub::Node::Object>(n_->value).size() : 0; }

    template<class K> JsonVariantRef operator[](const K &key) {
        if(!n_) return JsonVariantRef();
        auto &obj=std::get<arduinojson_stub::Node::Object>(n_->value);
        return JsonVariantRef(&obj[arduinojson_stub::keyString(key)]);
    }
    template<class K> JsonVariantConst operator[](const K &key) const { return JsonObjectConst(n_)[key]; }
    template<class K> JsonObject createNestedObject(const K &key) {
        if(!n_) return JsonObject();
        auto &obj=std::get<arduinojson_stub::Node::Object>(n_->value);
        auto &child=obj[arduinojson_stub::keyString(key)]; child.value=arduinojson_stub::Node::Object{}; return JsonObject(&child);
    }
    template<class K> void remove(const K &key) { if(n_) std::get<arduinojson_stub::Node::Object>(n_->value).erase(arduinojson_stub::keyString(key)); }
    operator JsonObjectConst() const { return JsonObjectConst(n_); }
private:
    arduinojson_stub::Node *n_=nullptr;
};

class JsonArrayConst {
public:
    JsonArrayConst() = default;
    explicit JsonArrayConst(const arduinojson_stub::Node *n) : n_(n && std::holds_alternative<arduinojson_stub::Node::Array>(n->value) ? n : nullptr) {}
    bool isNull() const { return n_==nullptr; }
    size_t size() const { return n_?std::get<arduinojson_stub::Node::Array>(n_->value).size():0; }
    struct iterator {
        using Inner=arduinojson_stub::Node::Array::const_iterator; Inner it;
        JsonVariantConst operator*() const {return JsonVariantConst(&*it);} iterator&operator++(){++it;return *this;} bool operator!=(const iterator&o)const{return it!=o.it;}
    };
    iterator begin() const { static const arduinojson_stub::Node::Array empty; const auto &a=n_?std::get<arduinojson_stub::Node::Array>(n_->value):empty; return {a.begin()}; }
    iterator end() const { static const arduinojson_stub::Node::Array empty; const auto &a=n_?std::get<arduinojson_stub::Node::Array>(n_->value):empty; return {a.end()}; }
private: const arduinojson_stub::Node*n_=nullptr; friend class JsonVariantConst;
};

inline JsonVariantConst::operator JsonObjectConst() const { return JsonObjectConst(n_); }
inline JsonVariantConst::operator JsonArrayConst() const { return JsonArrayConst(n_); }

template<class T> bool JsonVariantConst::is() const {
    if(!n_) return false;
    if constexpr (std::is_same_v<T,JsonObjectConst> || std::is_same_v<T,JsonObject>) return std::holds_alternative<arduinojson_stub::Node::Object>(n_->value);
    else if constexpr (std::is_same_v<T,JsonArrayConst>) return std::holds_alternative<arduinojson_stub::Node::Array>(n_->value);
    else if constexpr (std::is_same_v<T,String> || std::is_same_v<T,const char*>) return std::holds_alternative<std::string>(n_->value);
    else if constexpr (std::is_same_v<T,bool>) return std::holds_alternative<bool>(n_->value);
    else if constexpr (std::is_integral_v<T> || std::is_enum_v<T>) return std::holds_alternative<int64_t>(n_->value) || std::holds_alternative<double>(n_->value) || std::holds_alternative<bool>(n_->value);
    else if constexpr (std::is_floating_point_v<T>) return std::holds_alternative<double>(n_->value) || std::holds_alternative<int64_t>(n_->value);
    else return false;
}

template<class T> T JsonVariantConst::as() const {
    if constexpr (std::is_same_v<T,JsonObjectConst>) return JsonObjectConst(n_);
    else if constexpr (std::is_same_v<T,JsonArrayConst>) return JsonArrayConst(n_);
    else if constexpr (std::is_same_v<T,String>) {
        if(!n_) return String();
        if(auto p=std::get_if<std::string>(&n_->value)) return String(*p);
        if(auto p=std::get_if<int64_t>(&n_->value)) return String((long long)*p);
        if(auto p=std::get_if<double>(&n_->value)) return String(*p,6);
        if(auto p=std::get_if<bool>(&n_->value)) return String(*p?"true":"false");
        return String();
    } else if constexpr (std::is_same_v<T,const char*>) {
        if(!n_) return nullptr; if(auto p=std::get_if<std::string>(&n_->value)) return p->c_str(); return nullptr;
    } else if constexpr (std::is_same_v<T,bool>) {
        if(!n_) return false; if(auto p=std::get_if<bool>(&n_->value)) return *p; if(auto p=std::get_if<int64_t>(&n_->value)) return *p!=0; if(auto p=std::get_if<double>(&n_->value)) return *p!=0; return false;
    } else if constexpr (std::is_enum_v<T>) {
        return static_cast<T>(as<std::underlying_type_t<T>>());
    } else if constexpr (std::is_integral_v<T>) {
        if(!n_) return T{}; if(auto p=std::get_if<int64_t>(&n_->value)) return static_cast<T>(*p); if(auto p=std::get_if<double>(&n_->value)) return static_cast<T>(*p); if(auto p=std::get_if<bool>(&n_->value)) return static_cast<T>(*p); return T{};
    } else if constexpr (std::is_floating_point_v<T>) {
        if(!n_) return T{}; if(auto p=std::get_if<double>(&n_->value)) return static_cast<T>(*p); if(auto p=std::get_if<int64_t>(&n_->value)) return static_cast<T>(*p); return T{};
    } else return T{};
}

class JsonDocumentBase {
public:
    template<class T> T to() {
        if constexpr (std::is_same_v<T,JsonObject>) { root_.value=arduinojson_stub::Node::Object{}; return JsonObject(&root_); }
        else return T{};
    }
    template<class T> T as() const {
        if constexpr (std::is_same_v<T,JsonObjectConst>) return JsonObjectConst(&root_);
        else if constexpr (std::is_same_v<T,JsonObject>) return JsonObject(const_cast<arduinojson_stub::Node*>(&root_));
        else if constexpr (std::is_same_v<T,JsonVariantConst>) return JsonVariantConst(&root_);
        else return T{};
    }
    void clear(){root_=arduinojson_stub::Node();}
    arduinojson_stub::Node &root(){return root_;} const arduinojson_stub::Node&root()const{return root_;}
protected: arduinojson_stub::Node root_;
};
template<size_t N> class StaticJsonDocument : public JsonDocumentBase { public: StaticJsonDocument(){(void)N;} };
class DynamicJsonDocument : public JsonDocumentBase { public: explicit DynamicJsonDocument(size_t){} };

class DeserializationError { public: DeserializationError(bool failed=false):failed_(failed){} operator bool()const{return failed_;} const char* c_str()const{return failed_?"InvalidInput":"Ok";} private:bool failed_; };

namespace arduinojson_stub {
class Parser {
public:
    explicit Parser(const std::string &text) : text_(text), pos_(0) {}

    bool parse(Node &out) {
        skipWs();
        if (!parseValue(out)) return false;
        skipWs();
        return pos_ == text_.size();
    }

private:
    const std::string &text_;
    size_t pos_;

    void skipWs() {
        while (pos_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[pos_]))) ++pos_;
    }

    bool consume(char c) {
        skipWs();
        if (pos_ >= text_.size() || text_[pos_] != c) return false;
        ++pos_;
        return true;
    }

    bool parseValue(Node &out) {
        skipWs();
        if (pos_ >= text_.size()) return false;
        char c = text_[pos_];
        if (c == '{') return parseObject(out);
        if (c == '[') return parseArray(out);
        if (c == '"') {
            std::string value;
            if (!parseString(value)) return false;
            out.value = value;
            return true;
        }
        if (c == 't' && text_.compare(pos_, 4, "true") == 0) { pos_ += 4; out.value = true; return true; }
        if (c == 'f' && text_.compare(pos_, 5, "false") == 0) { pos_ += 5; out.value = false; return true; }
        if (c == 'n' && text_.compare(pos_, 4, "null") == 0) { pos_ += 4; out.value = std::monostate{}; return true; }
        return parseNumber(out);
    }

    bool parseObject(Node &out) {
        if (!consume('{')) return false;
        Node::Object object;
        skipWs();
        if (consume('}')) { out.value = std::move(object); return true; }
        while (true) {
            std::string key;
            if (!parseString(key) || !consume(':')) return false;
            Node value;
            if (!parseValue(value)) return false;
            object.emplace(std::move(key), std::move(value));
            skipWs();
            if (consume('}')) break;
            if (!consume(',')) return false;
        }
        out.value = std::move(object);
        return true;
    }

    bool parseArray(Node &out) {
        if (!consume('[')) return false;
        Node::Array array;
        skipWs();
        if (consume(']')) { out.value = std::move(array); return true; }
        while (true) {
            Node value;
            if (!parseValue(value)) return false;
            array.push_back(std::move(value));
            skipWs();
            if (consume(']')) break;
            if (!consume(',')) return false;
        }
        out.value = std::move(array);
        return true;
    }

    bool parseString(std::string &out) {
        skipWs();
        if (pos_ >= text_.size() || text_[pos_] != '"') return false;
        ++pos_;
        out.clear();
        while (pos_ < text_.size()) {
            char c = text_[pos_++];
            if (c == '"') return true;
            if (c == '\\') {
                if (pos_ >= text_.size()) return false;
                char esc = text_[pos_++];
                switch (esc) {
                    case '"': out.push_back('"'); break;
                    case '\\': out.push_back('\\'); break;
                    case '/': out.push_back('/'); break;
                    case 'b': out.push_back('\b'); break;
                    case 'f': out.push_back('\f'); break;
                    case 'n': out.push_back('\n'); break;
                    case 'r': out.push_back('\r'); break;
                    case 't': out.push_back('\t'); break;
                    default: return false;
                }
            } else {
                out.push_back(c);
            }
        }
        return false;
    }

    bool parseNumber(Node &out) {
        skipWs();
        size_t begin = pos_;
        if (pos_ < text_.size() && text_[pos_] == '-') ++pos_;
        while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) ++pos_;
        bool floating = false;
        if (pos_ < text_.size() && text_[pos_] == '.') {
            floating = true;
            ++pos_;
            while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) ++pos_;
        }
        if (pos_ < text_.size() && (text_[pos_] == 'e' || text_[pos_] == 'E')) {
            floating = true;
            ++pos_;
            if (pos_ < text_.size() && (text_[pos_] == '+' || text_[pos_] == '-')) ++pos_;
            while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) ++pos_;
        }
        if (pos_ == begin || (pos_ == begin + 1 && text_[begin] == '-')) return false;
        try {
            std::string token = text_.substr(begin, pos_ - begin);
            if (floating) out.value = std::stod(token);
            else out.value = static_cast<int64_t>(std::stoll(token));
            return true;
        } catch (...) {
            return false;
        }
    }
};

inline bool parseJsonText(const std::string &text, Node &root) {
    Parser parser(text);
    return parser.parse(root);
}
}

template<class Doc> DeserializationError deserializeJson(Doc &doc, Stream &stream) {
    std::string text;
    while (stream.available()) {
        int value = stream.read();
        if (value < 0) break;
        text.push_back(static_cast<char>(value));
    }
    doc.clear();
    return DeserializationError(!arduinojson_stub::parseJsonText(text, doc.root()));
}
template<class Doc> DeserializationError deserializeJson(Doc &doc, const String &text) {
    doc.clear();
    return DeserializationError(!arduinojson_stub::parseJsonText(std::string(text.c_str()), doc.root()));
}
template<class Doc> DeserializationError deserializeJson(Doc &doc, const char *text) {
    doc.clear();
    return DeserializationError(!arduinojson_stub::parseJsonText(text ? std::string(text) : std::string(), doc.root()));
}

inline void _jsonWriteNode(const arduinojson_stub::Node &n, std::ostringstream &os) {
    if(std::holds_alternative<std::monostate>(n.value)) os<<"null";
    else if(auto p=std::get_if<bool>(&n.value)) os<<(*p?"true":"false");
    else if(auto p=std::get_if<int64_t>(&n.value)) os<<*p;
    else if(auto p=std::get_if<double>(&n.value)) os<<*p;
    else if(auto p=std::get_if<std::string>(&n.value)) {os<<'"'; for(char c:*p){if(c=='"'||c=='\\')os<<'\\';os<<c;} os<<'"';}
    else if(auto p=std::get_if<arduinojson_stub::Node::Object>(&n.value)){os<<'{';bool first=true;for(auto &kv:*p){if(!first)os<<',';first=false;os<<'"'<<kv.first<<"\":";_jsonWriteNode(kv.second,os);}os<<'}';}
    else if(auto p=std::get_if<arduinojson_stub::Node::Array>(&n.value)){os<<'[';bool first=true;for(auto &v:*p){if(!first)os<<',';first=false;_jsonWriteNode(v,os);}os<<']';}
}
template<class Doc> size_t serializeJson(const Doc &doc, Print &out){std::ostringstream os;_jsonWriteNode(doc.root(),os);String s(os.str());return out.print(s);} 
template<class Doc> size_t serializeJsonPretty(const Doc &doc, Print &out){return serializeJson(doc,out);} 
template<class Doc> size_t serializeJson(const Doc &doc, String &out){std::ostringstream os;_jsonWriteNode(doc.root(),os);out=String(os.str());return out.length();}
template<class Doc> size_t serializeJsonPretty(const Doc &doc, String &out){return serializeJson(doc,out);} 
template<class Doc> size_t measureJson(const Doc &doc){std::ostringstream os;_jsonWriteNode(doc.root(),os);return os.str().size();}
template<class Doc> size_t measureJsonPretty(const Doc &doc){return measureJson(doc);} 

#endif
