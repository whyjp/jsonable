#pragma once

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"
#include <string>
#include <stdexcept>

namespace json {

// JSON serialization/deserialization interface
class Jsonable {
public:
    virtual ~Jsonable() = default;
    
    // Convert from JSON string to object
    virtual void fromJson(const std::string& jsonStr) = 0;
    
    // Convert object to JSON string
    virtual std::string toJson() const = 0;
    
    // Convert from RapidJSON Document to object
    virtual void fromDocument(const rapidjson::Value& value) = 0;
    
    // Convert object to RapidJSON Value
    virtual rapidjson::Value toValue(rapidjson::Document::AllocatorType& allocator) const = 0;

protected:
    // JSON parsing helper
    static rapidjson::Document parseJson(const std::string& jsonStr) {
        rapidjson::Document doc;
        doc.Parse(jsonStr.c_str());
        
        if (doc.HasParseError()) {
            throw std::runtime_error("JSON parsing error: " + 
                std::string(rapidjson::GetParseError_En(doc.GetParseError())));
        }
        
        return doc;
    }
    
    // JSON string generation helper
    static std::string valueToString(const rapidjson::Value& value) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        value.Accept(writer);
        return buffer.GetString();
    }
    
    // Safe string extraction
    static std::string getString(const rapidjson::Value& value, const char* key, const std::string& defaultValue = "") {
        if (value.HasMember(key) && value[key].IsString()) {
            return value[key].GetString();
        }
        return defaultValue;
    }
    
    // Safe integer extraction
    static int64_t getInt64(const rapidjson::Value& value, const char* key, int64_t defaultValue = 0) {
        if (value.HasMember(key) && value[key].IsInt64()) {
            return value[key].GetInt64();
        }
        return defaultValue;
    }
    
    // Safe boolean extraction
    static bool getBool(const rapidjson::Value& value, const char* key, bool defaultValue = false) {
        if (value.HasMember(key) && value[key].IsBool()) {
            return value[key].GetBool();
        }
        return defaultValue;
    }
    
    // Safe array check
    static bool isArray(const rapidjson::Value& value, const char* key) {
        return value.HasMember(key) && value[key].IsArray();
    }
    
    // Safe object check
    static bool isObject(const rapidjson::Value& value, const char* key) {
        return value.HasMember(key) && value[key].IsObject();
    }
};

} // namespace json