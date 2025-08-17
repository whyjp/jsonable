#pragma once

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <cstdint>

namespace json {

/**
 * @brief JSON 직렬화/역직렬화를 위한 기본 인터페이스
 * 
 * 이 클래스는 내재적 직렬화(Intrusive Serialization) 패턴을 제공합니다.
 * 사용자는 이 클래스를 상속받아 fromDocument()와 toValue()만 구현하면 됩니다.
 * 
 * 핵심 설계 원칙:
 * 1. 매크로 없는 순수 함수 기반 접근법
 * 2. Iteration Functor 패턴으로 배열/객체 처리
 * 3. 헬퍼 함수로 모든 JSON 타입 지원
 * 4. 사용자가 구조를 직접 정의하고 제어
 */
class Jsonable {
public:
    virtual ~Jsonable() = default;

    // ========================================
    // 주요 인터페이스 (사용자 구현 필수)
    // ========================================
    
    /**
     * @brief JSON 문자열에서 객체로 역직렬화
     */
    virtual void fromJson(const std::string& jsonStr) {
        auto doc = parseJson(jsonStr);
        fromDocument(doc);
    }

    /**
     * @brief 객체에서 JSON 문자열로 직렬화
     */
    virtual std::string toJson() const {
        rapidjson::Document doc;
        doc.SetObject();
        auto value = toValue(doc.GetAllocator());
        doc.CopyFrom(value, doc.GetAllocator());
        return valueToString(doc);
    }

    /**
     * @brief JSON Document에서 객체로 변환 (사용자 구현)
     */
    virtual void fromDocument(const rapidjson::Value& value) = 0;

    /**
     * @brief 객체에서 JSON Value로 변환 (사용자 구현)
     */
    virtual rapidjson::Value toValue(rapidjson::Document::AllocatorType& allocator) const = 0;

    // ========================================
    // JSON 파싱 및 문자열 변환 헬퍼
    // ========================================

    /**
     * @brief JSON 문자열을 Document로 파싱
     */
    static rapidjson::Document parseJson(const std::string& jsonStr) {
        rapidjson::Document doc;
        doc.Parse(jsonStr.c_str());
        if (doc.HasParseError()) {
            throw std::runtime_error("JSON parse error at offset " + 
                                   std::to_string(doc.GetErrorOffset()));
        }
        return doc;
    }

    /**
     * @brief JSON Value를 문자열로 변환
     */
    static std::string valueToString(const rapidjson::Value& value) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        value.Accept(writer);
        return buffer.GetString();
    }

    // ========================================
    // 기본 타입 추출 헬퍼 함수들
    // ========================================

    /**
     * @brief 문자열 값 추출
     */
    static std::string getString(const rapidjson::Value& obj, const char* key, 
                                const std::string& defaultValue = "") {
        if (obj.HasMember(key) && obj[key].IsString()) {
            return obj[key].GetString();
        }
        return defaultValue;
    }

    /**
     * @brief 64비트 정수 값 추출 (JSON 표준 권장)
     */
    static int64_t getInt64(const rapidjson::Value& obj, const char* key, 
                           int64_t defaultValue = 0) {
        if (obj.HasMember(key) && obj[key].IsNumber()) {
            if (obj[key].IsInt64()) {
                return obj[key].GetInt64();
            } else if (obj[key].IsDouble()) {
                return static_cast<int64_t>(obj[key].GetDouble());
            }
        }
        return defaultValue;
    }

    /**
     * @brief double 값 추출 (JSON 표준 권장)
     */
    static double getDouble(const rapidjson::Value& obj, const char* key, 
                           double defaultValue = 0.0) {
        if (obj.HasMember(key) && obj[key].IsNumber()) {
            return obj[key].GetDouble();
        }
        return defaultValue;
    }

    /**
     * @brief float 값 추출 (정밀도 주의)
     */
    static float getFloat(const rapidjson::Value& obj, const char* key, 
                         float defaultValue = 0.0f) {
        if (obj.HasMember(key) && obj[key].IsNumber()) {
            return static_cast<float>(obj[key].GetDouble());
        }
        return defaultValue;
    }

    /**
     * @brief bool 값 추출
     */
    static bool getBool(const rapidjson::Value& obj, const char* key, 
                       bool defaultValue = false) {
        if (obj.HasMember(key)) {
            if (obj[key].IsBool()) {
                return obj[key].GetBool();
            } else if (obj[key].IsNumber()) {
                return obj[key].GetInt() != 0;
            }
        }
        return defaultValue;
    }

    /**
     * @brief uint32_t 값 추출 (범위 주의)
     */
    static uint32_t getUInt32(const rapidjson::Value& obj, const char* key, 
                             uint32_t defaultValue = 0) {
        if (obj.HasMember(key) && obj[key].IsNumber()) {
            if (obj[key].IsUint()) {
                return obj[key].GetUint();
            } else if (obj[key].IsInt()) {
                int value = obj[key].GetInt();
                if (value >= 0) {
                    return static_cast<uint32_t>(value);
                }
            } else if (obj[key].IsInt64()) {
                int64_t value = obj[key].GetInt64();
                if (value >= 0 && value <= UINT32_MAX) {
                    return static_cast<uint32_t>(value);
                }
            } else if (obj[key].IsUint64()) {
                uint64_t value = obj[key].GetUint64();
                if (value <= UINT32_MAX) {
                    return static_cast<uint32_t>(value);
                }
            }
        }
        return defaultValue;
    }

    /**
     * @brief uint64_t 값 추출 (범위 주의)
     */
    static uint64_t getUInt64(const rapidjson::Value& obj, const char* key, 
                             uint64_t defaultValue = 0) {
        if (obj.HasMember(key) && obj[key].IsNumber()) {
            if (obj[key].IsUint64()) {
                return obj[key].GetUint64();
            } else if (obj[key].IsInt64()) {
                int64_t value = obj[key].GetInt64();
                if (value >= 0) {
                    return static_cast<uint64_t>(value);
                }
            } else if (obj[key].IsUint()) {
                return static_cast<uint64_t>(obj[key].GetUint());
            } else if (obj[key].IsInt()) {
                int value = obj[key].GetInt();
                if (value >= 0) {
                    return static_cast<uint64_t>(value);
                }
            }
        }
        return defaultValue;
    }

    // ========================================
    // Optional 타입 헬퍼 함수들 (null 안전)
    // ========================================

    /**
     * @brief 선택적 문자열 값 추출
     */
    static std::optional<std::string> getOptionalString(const rapidjson::Value& obj, const char* key) {
        if (obj.HasMember(key) && !obj[key].IsNull() && obj[key].IsString()) {
            return obj[key].GetString();
        }
        return std::nullopt;
    }

    /**
     * @brief 선택적 정수 값 추출
     */
    static std::optional<int64_t> getOptionalInt64(const rapidjson::Value& obj, const char* key) {
        if (obj.HasMember(key) && !obj[key].IsNull() && obj[key].IsNumber()) {
            return obj[key].GetInt64();
        }
        return std::nullopt;
    }

    /**
     * @brief 선택적 실수 값 추출
     */
    static std::optional<double> getOptionalDouble(const rapidjson::Value& obj, const char* key) {
        if (obj.HasMember(key) && !obj[key].IsNull() && obj[key].IsNumber()) {
            return obj[key].GetDouble();
        }
        return std::nullopt;
    }

    /**
     * @brief 선택적 float 값 추출
     */
    static std::optional<float> getOptionalFloat(const rapidjson::Value& obj, const char* key) {
        if (obj.HasMember(key) && !obj[key].IsNull() && obj[key].IsNumber()) {
            return static_cast<float>(obj[key].GetDouble());
        }
        return std::nullopt;
    }

    /**
     * @brief 선택적 불린 값 추출
     */
    static std::optional<bool> getOptionalBool(const rapidjson::Value& obj, const char* key) {
        if (obj.HasMember(key) && !obj[key].IsNull() && obj[key].IsBool()) {
            return obj[key].GetBool();
        }
        return std::nullopt;
    }

    // ========================================
    // 배열/객체 존재 확인 헬퍼
    // ========================================

    /**
     * @brief 배열 존재 확인
     */
    static bool isArray(const rapidjson::Value& obj, const char* key) {
        return obj.HasMember(key) && obj[key].IsArray();
    }

    /**
     * @brief 객체 존재 확인
     */
    static bool isObject(const rapidjson::Value& obj, const char* key) {
        return obj.HasMember(key) && obj[key].IsObject();
    }

    // ========================================
    // Iteration Functor 패턴 - 배열 처리
    // ========================================

    /**
     * @brief 배열 반복 처리 (Functor 패턴)
     * 사용자가 각 배열 요소를 어떻게 처리할지 직접 정의
     */
    static void iterateArray(const rapidjson::Value& obj, const char* key,
                            std::function<void(const rapidjson::Value&)> elementProcessor) {
        if (isArray(obj, key)) {
            const auto& array = obj[key];
            for (const auto& element : array.GetArray()) {
                elementProcessor(element);
            }
        }
    }

    /**
     * @brief 루트 배열 반복 처리
     */
    static void iterateArray(const rapidjson::Value& array,
                            std::function<void(const rapidjson::Value&)> elementProcessor) {
        if (array.IsArray()) {
            for (const auto& element : array.GetArray()) {
                elementProcessor(element);
            }
        }
    }

    /**
     * @brief 배열 추출 (타입별 변환 함수 사용)
     */
    template<typename T>
    static std::vector<T> extractArray(const rapidjson::Value& obj, const char* key,
                                      std::function<T(const rapidjson::Value&)> converter) {
        std::vector<T> result;
        iterateArray(obj, key, [&result, &converter](const rapidjson::Value& element) {
            result.push_back(converter(element));
        });
        return result;
    }

    /**
     * @brief 문자열 배열 추출 (편의 함수)
     */
    static std::vector<std::string> getStringArray(const rapidjson::Value& obj, const char* key) {
        return extractArray<std::string>(obj, key, [](const rapidjson::Value& val) -> std::string {
            return val.IsString() ? val.GetString() : "";
        });
    }

    /**
     * @brief 정수 배열 추출 (편의 함수)
     */
    static std::vector<int64_t> getInt64Array(const rapidjson::Value& obj, const char* key) {
        return extractArray<int64_t>(obj, key, [](const rapidjson::Value& val) -> int64_t {
            return val.IsNumber() ? val.GetInt64() : 0;
        });
    }

    /**
     * @brief 실수 배열 추출 (편의 함수)
     */
    static std::vector<double> getDoubleArray(const rapidjson::Value& obj, const char* key) {
        return extractArray<double>(obj, key, [](const rapidjson::Value& val) -> double {
            return val.IsNumber() ? val.GetDouble() : 0.0;
        });
    }

    // ========================================
    // Iteration Functor 패턴 - 객체 처리
    // ========================================

    /**
     * @brief 객체 반복 처리 (Functor 패턴)
     * 사용자가 각 객체 멤버를 어떻게 처리할지 직접 정의
     */
    static void iterateObject(const rapidjson::Value& obj, const char* key,
                             std::function<void(const char*, const rapidjson::Value&)> memberProcessor) {
        if (isObject(obj, key)) {
            const auto& object = obj[key];
            for (auto it = object.MemberBegin(); it != object.MemberEnd(); ++it) {
                memberProcessor(it->name.GetString(), it->value);
            }
        }
    }

    /**
     * @brief 루트 객체 반복 처리
     */
    static void iterateObject(const rapidjson::Value& object,
                             std::function<void(const char*, const rapidjson::Value&)> memberProcessor) {
        if (object.IsObject()) {
            for (auto it = object.MemberBegin(); it != object.MemberEnd(); ++it) {
                memberProcessor(it->name.GetString(), it->value);
            }
        }
    }

    // ========================================
    // 배열/객체 생성 헬퍼 (Functor 패턴)
    // ========================================

    /**
     * @brief 배열 생성 (타입별 변환 함수 사용)
     */
    template<typename T>
    static rapidjson::Value createArray(const std::vector<T>& items,
                                       rapidjson::Document::AllocatorType& allocator,
                                       std::function<rapidjson::Value(const T&)> converter) {
        rapidjson::Value array(rapidjson::kArrayType);
        for (const auto& item : items) {
            array.PushBack(converter(item), allocator);
        }
        return array;
    }

    /**
     * @brief 문자열 배열 생성 (편의 함수)
     */
    static rapidjson::Value createStringArray(const std::vector<std::string>& strings,
                                             rapidjson::Document::AllocatorType& allocator) {
        return createArray<std::string>(strings, allocator, 
            [&allocator](const std::string& str) -> rapidjson::Value {
                rapidjson::Value val;
                val.SetString(str.c_str(), static_cast<rapidjson::SizeType>(str.length()), allocator);
                return val;
            });
    }

    /**
     * @brief 정수 배열 생성 (편의 함수)
     */
    static rapidjson::Value createInt64Array(const std::vector<int64_t>& numbers,
                                            rapidjson::Document::AllocatorType& allocator) {
        return createArray<int64_t>(numbers, allocator,
            [](int64_t num) -> rapidjson::Value {
                rapidjson::Value val(num);
                return val;
            });
    }

    /**
     * @brief 실수 배열 생성 (편의 함수)
     */
    static rapidjson::Value createDoubleArray(const std::vector<double>& numbers,
                                             rapidjson::Document::AllocatorType& allocator) {
        return createArray<double>(numbers, allocator,
            [](double num) -> rapidjson::Value {
                rapidjson::Value val(num);
                return val;
            });
    }

    // ========================================
    // 중첩 경로 탐색 헬퍼
    // ========================================

    /**
     * @brief 중첩 경로로 문자열 값 추출 (예: "user.profile.name")
     */
    static std::string getNestedString(const rapidjson::Value& obj, const std::string& path,
                                      const std::string& defaultValue = "") {
        const rapidjson::Value* current = &obj;
        std::string currentPath = path;
        
        while (!currentPath.empty()) {
            size_t dotPos = currentPath.find('.');
            std::string key = (dotPos == std::string::npos) ? currentPath : currentPath.substr(0, dotPos);
            
            if (!current->HasMember(key.c_str())) {
                return defaultValue;
            }
            
            current = &(*current)[key.c_str()];
            
            if (dotPos == std::string::npos) {
                break;
            }
            currentPath = currentPath.substr(dotPos + 1);
        }
        
        return current->IsString() ? current->GetString() : defaultValue;
    }

    /**
     * @brief 중첩 경로로 정수 값 추출
     */
    static int64_t getNestedInt64(const rapidjson::Value& obj, const std::string& path,
                                 int64_t defaultValue = 0) {
        const rapidjson::Value* current = &obj;
        std::string currentPath = path;
        
        while (!currentPath.empty()) {
            size_t dotPos = currentPath.find('.');
            std::string key = (dotPos == std::string::npos) ? currentPath : currentPath.substr(0, dotPos);
            
            if (!current->HasMember(key.c_str())) {
                return defaultValue;
            }
            
            current = &(*current)[key.c_str()];
            
            if (dotPos == std::string::npos) {
                break;
            }
            currentPath = currentPath.substr(dotPos + 1);
        }
        
        return current->IsNumber() ? current->GetInt64() : defaultValue;
    }

    /**
     * @brief 중첩 경로 존재 확인
     */
    static bool hasNestedPath(const rapidjson::Value& obj, const std::string& path) {
        const rapidjson::Value* current = &obj;
        std::string currentPath = path;
        
        while (!currentPath.empty()) {
            size_t dotPos = currentPath.find('.');
            std::string key = (dotPos == std::string::npos) ? currentPath : currentPath.substr(0, dotPos);
            
            if (!current->HasMember(key.c_str())) {
                return false;
            }
            
            current = &(*current)[key.c_str()];
            
            if (dotPos == std::string::npos) {
                break;
            }
            currentPath = currentPath.substr(dotPos + 1);
        }
        
        return true;
    }

    /**
     * @brief 중첩 배열 반복 처리
     */
    static void iterateNestedArray(const rapidjson::Value& obj, const std::string& path,
                                  std::function<void(const rapidjson::Value&)> elementProcessor) {
        const rapidjson::Value* current = &obj;
        std::string currentPath = path;
        
        while (!currentPath.empty()) {
            size_t dotPos = currentPath.find('.');
            std::string key = (dotPos == std::string::npos) ? currentPath : currentPath.substr(0, dotPos);
            
            if (!current->HasMember(key.c_str())) {
                return;
            }
            
            current = &(*current)[key.c_str()];
            
            if (dotPos == std::string::npos) {
                break;
            }
            currentPath = currentPath.substr(dotPos + 1);
        }
        
        if (current->IsArray()) {
            for (const auto& element : current->GetArray()) {
                elementProcessor(element);
            }
        }
    }
};

} // namespace json 