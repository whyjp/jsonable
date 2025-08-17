#pragma once

/**
 * JsonableBase.hpp - 기본 JSON 조작 구현 (완전 inline)
 * 
 * 역할: RapidJSON 의존성 캡슐화 및 모든 기본 JSON 조작 제공
 */

#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

// RapidJSON 헤더들 (사용자에게 숨겨짐)
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace json {

// ========================================
// 타입 트레이트 (컴파일 타임 타입 검증)
// ========================================

/**
 * @brief JSON 기본 타입 체크 (타입 안전성)
 */
template<typename T>
constexpr bool is_json_primitive_v = std::disjunction_v<
    std::is_same<T, std::string>,
    std::is_same<T, int>,
    std::is_same<T, int64_t>,
    std::is_same<T, double>,
    std::is_same<T, float>,
    std::is_same<T, bool>,
    std::is_same<T, uint32_t>,
    std::is_same<T, uint64_t>
>;

/**
 * @brief 기본 JSON 조작 클래스 - RapidJSON 구현 캡슐화
 * 
 * 책임:
 * - RapidJSON document 관리
 * - 기본 타입 읽기/쓰기 (getString, setString 등)
 * - Begin/End 스택 관리
 * - 배열/객체 존재 확인
 * - RapidJSON 의존성 100% 숨김
 */
class JsonableBase {
private:
    rapidjson::Document document_;
    
    // 컨텍스트 스택 관리 (Begin/End 스타일용)
    struct JsonContext {
        rapidjson::Value* current;
        bool isArray;
        std::string key;
    };
    std::vector<JsonContext> contextStack_;

protected:
    // 파생 클래스에서만 생성/소멸 가능
    JsonableBase() {
        document_.SetObject();
    }
    
    virtual ~JsonableBase() = default;
    
    // 복사/이동 (RapidJSON document 처리)
    JsonableBase(const JsonableBase& other) : document_() {
        document_.CopyFrom(other.document_, document_.GetAllocator());
        // contextStack_는 복사하지 않음 (런타임 상태)
    }
    
    JsonableBase(JsonableBase&& other) noexcept 
        : document_(std::move(other.document_)), contextStack_(std::move(other.contextStack_)) {}
    
    JsonableBase& operator=(const JsonableBase& other) {
        if (this != &other) {
            document_.CopyFrom(other.document_, document_.GetAllocator());
            contextStack_.clear(); // 컨텍스트는 초기화
        }
        return *this;
    }
    
    JsonableBase& operator=(JsonableBase&& other) noexcept {
        if (this != &other) {
            document_ = std::move(other.document_);
            contextStack_ = std::move(other.contextStack_);
        }
        return *this;
    }

public:
    // ========================================
    // 기본 타입 읽기 (RapidJSON 완전 숨김)
    // ========================================
    
    inline std::string getString(const char* key, const std::string& defaultValue = "") const {
        if (document_.HasMember(key) && document_[key].IsString()) {
            return document_[key].GetString();
        }
        return defaultValue;
    }
    
    inline int64_t getInt64(const char* key, int64_t defaultValue = 0) const {
        if (document_.HasMember(key) && document_[key].IsNumber()) {
            const auto& value = document_[key];
            if (value.IsInt64()) return value.GetInt64();
            if (value.IsUint64()) return static_cast<int64_t>(value.GetUint64());
            if (value.IsInt()) return static_cast<int64_t>(value.GetInt());
            if (value.IsUint()) return static_cast<int64_t>(value.GetUint());
            if (value.IsDouble()) return static_cast<int64_t>(value.GetDouble());
        }
        return defaultValue;
    }
    
    inline double getDouble(const char* key, double defaultValue = 0.0) const {
        if (document_.HasMember(key) && document_[key].IsNumber()) {
            return document_[key].GetDouble();
        }
        return defaultValue;
    }
    
    inline float getFloat(const char* key, float defaultValue = 0.0f) const {
        return static_cast<float>(getDouble(key, static_cast<double>(defaultValue)));
    }
    
    inline bool getBool(const char* key, bool defaultValue = false) const {
        if (document_.HasMember(key) && document_[key].IsBool()) {
            return document_[key].GetBool();
        }
        return defaultValue;
    }
    
    inline uint32_t getUInt32(const char* key, uint32_t defaultValue = 0) const {
        if (document_.HasMember(key) && document_[key].IsNumber()) {
            const auto& value = document_[key];
            if (value.IsUint()) return value.GetUint();
            if (value.IsUint64()) {
                uint64_t val = value.GetUint64();
                return (val <= UINT32_MAX) ? static_cast<uint32_t>(val) : defaultValue;
            }
            if (value.IsInt64()) {
                int64_t val = value.GetInt64();
                return (val >= 0 && val <= UINT32_MAX) ? static_cast<uint32_t>(val) : defaultValue;
            }
        }
        return defaultValue;
    }
    
    inline uint64_t getUInt64(const char* key, uint64_t defaultValue = 0) const {
        if (document_.HasMember(key) && document_[key].IsNumber()) {
            const auto& value = document_[key];
            if (value.IsUint64()) return value.GetUint64();
            if (value.IsUint()) return static_cast<uint64_t>(value.GetUint());
            if (value.IsInt64()) {
                int64_t val = value.GetInt64();
                return (val >= 0) ? static_cast<uint64_t>(val) : defaultValue;
            }
        }
        return defaultValue;
    }
    
    // ========================================
    // 기본 타입 쓰기 (컨텍스트 자동 인식)
    // ========================================
    
    inline void setString(const char* key, const std::string& value) {
        auto& allocator = document_.GetAllocator();
        rapidjson::Value valueVal(value.c_str(), allocator);
        
        if (contextStack_.empty()) {
            // 루트 레벨 - 기존 방식
            ensureObject();
            rapidjson::Value keyVal(key, allocator);
            
            if (document_.HasMember(key)) {
                document_[key] = std::move(valueVal);
            } else {
                document_.AddMember(std::move(keyVal), std::move(valueVal), allocator);
            }
        } else {
            // 컨텍스트 내부
            auto* current = getCurrentContext();
            
            if (contextStack_.back().isArray) {
                // 배열 컨텍스트: key 무시하고 배열에 추가
                current->PushBack(std::move(valueVal), allocator);
            } else {
                // 객체 컨텍스트: key를 사용하여 필드 설정
                if (key && strlen(key) > 0) {
                    rapidjson::Value keyVal(key, allocator);
                    if (current->HasMember(key)) {
                        (*current)[key] = std::move(valueVal);
                    } else {
                        current->AddMember(std::move(keyVal), std::move(valueVal), allocator);
                    }
                }
            }
        }
    }
    
    inline void setInt64(const char* key, int64_t value) {
        auto& allocator = document_.GetAllocator();
        rapidjson::Value valueVal(value);
        
        if (contextStack_.empty()) {
            ensureObject();
            rapidjson::Value keyVal(key, allocator);
            
            if (document_.HasMember(key)) {
                document_[key] = std::move(valueVal);
            } else {
                document_.AddMember(std::move(keyVal), std::move(valueVal), allocator);
            }
        } else {
            auto* current = getCurrentContext();
            
            if (contextStack_.back().isArray) {
                current->PushBack(std::move(valueVal), allocator);
            } else {
                if (key && strlen(key) > 0) {
                    rapidjson::Value keyVal(key, allocator);
                    if (current->HasMember(key)) {
                        (*current)[key] = std::move(valueVal);
                    } else {
                        current->AddMember(std::move(keyVal), std::move(valueVal), allocator);
                    }
                }
            }
        }
    }
    
    inline void setDouble(const char* key, double value) {
        auto& allocator = document_.GetAllocator();
        rapidjson::Value valueVal(value);
        
        if (contextStack_.empty()) {
            ensureObject();
            rapidjson::Value keyVal(key, allocator);
            
            if (document_.HasMember(key)) {
                document_[key] = std::move(valueVal);
            } else {
                document_.AddMember(std::move(keyVal), std::move(valueVal), allocator);
            }
        } else {
            auto* current = getCurrentContext();
            
            if (contextStack_.back().isArray) {
                current->PushBack(std::move(valueVal), allocator);
            } else {
                if (key && strlen(key) > 0) {
                    rapidjson::Value keyVal(key, allocator);
                    if (current->HasMember(key)) {
                        (*current)[key] = std::move(valueVal);
                    } else {
                        current->AddMember(std::move(keyVal), std::move(valueVal), allocator);
                    }
                }
            }
        }
    }
    
    inline void setFloat(const char* key, float value) {
        setDouble(key, static_cast<double>(value));
    }
    
    inline void setBool(const char* key, bool value) {
        auto& allocator = document_.GetAllocator();
        rapidjson::Value valueVal(value);
        
        if (contextStack_.empty()) {
            ensureObject();
            rapidjson::Value keyVal(key, allocator);
            
            if (document_.HasMember(key)) {
                document_[key] = std::move(valueVal);
            } else {
                document_.AddMember(std::move(keyVal), std::move(valueVal), allocator);
            }
        } else {
            auto* current = getCurrentContext();
            
            if (contextStack_.back().isArray) {
                current->PushBack(std::move(valueVal), allocator);
            } else {
                if (key && strlen(key) > 0) {
                    rapidjson::Value keyVal(key, allocator);
                    if (current->HasMember(key)) {
                        (*current)[key] = std::move(valueVal);
                    } else {
                        current->AddMember(std::move(keyVal), std::move(valueVal), allocator);
                    }
                }
            }
        }
    }
    
    inline void setUInt32(const char* key, uint32_t value) {
        auto& allocator = document_.GetAllocator();
        rapidjson::Value valueVal(value);
        
        if (contextStack_.empty()) {
            ensureObject();
            rapidjson::Value keyVal(key, allocator);
            
            if (document_.HasMember(key)) {
                document_[key] = std::move(valueVal);
            } else {
                document_.AddMember(std::move(keyVal), std::move(valueVal), allocator);
            }
        } else {
            auto* current = getCurrentContext();
            
            if (contextStack_.back().isArray) {
                current->PushBack(std::move(valueVal), allocator);
            } else {
                if (key && strlen(key) > 0) {
                    rapidjson::Value keyVal(key, allocator);
                    if (current->HasMember(key)) {
                        (*current)[key] = std::move(valueVal);
                    } else {
                        current->AddMember(std::move(keyVal), std::move(valueVal), allocator);
                    }
                }
            }
        }
    }
    
    inline void setUInt64(const char* key, uint64_t value) {
        auto& allocator = document_.GetAllocator();
        rapidjson::Value valueVal(value);
        
        if (contextStack_.empty()) {
            ensureObject();
            rapidjson::Value keyVal(key, allocator);
            
            if (document_.HasMember(key)) {
                document_[key] = std::move(valueVal);
            } else {
                document_.AddMember(std::move(keyVal), std::move(valueVal), allocator);
            }
        } else {
            auto* current = getCurrentContext();
            
            if (contextStack_.back().isArray) {
                current->PushBack(std::move(valueVal), allocator);
            } else {
                if (key && strlen(key) > 0) {
                    rapidjson::Value keyVal(key, allocator);
                    if (current->HasMember(key)) {
                        (*current)[key] = std::move(valueVal);
                    } else {
                        current->AddMember(std::move(keyVal), std::move(valueVal), allocator);
                    }
                }
            }
        }
    }
    
    // ========================================
    // 배열 처리 (타입 안전성 보장)
    // ========================================
    
    template<typename T>
    inline std::vector<T> getArray(const char* key) const {
        // 타입 안전성 보장: JSON 기본 타입만 허용
        static_assert(is_json_primitive_v<T>, 
                     "Array elements must be JSON primitive types only");
        
        std::vector<T> result;
        
        if (document_.HasMember(key) && document_[key].IsArray()) {
            const auto& array = document_[key];
            for (const auto& element : array.GetArray()) {
                result.push_back(convertFromValue<T>(element));
            }
        }
        
        return result;
    }
    
    template<typename T>
    inline void setArray(const char* key, const std::vector<T>& values) {
        // 타입 안전성 보장: JSON 기본 타입만 허용
        static_assert(is_json_primitive_v<T>, 
                     "Array elements must be JSON primitive types only");
        
        ensureObject();
        
        rapidjson::Value array(rapidjson::kArrayType);
        auto& allocator = document_.GetAllocator();
        
        for (const auto& value : values) {
            array.PushBack(convertToValue(value), allocator);
        }
        
        if (document_.HasMember(key)) {
            document_[key] = std::move(array);
        } else {
            document_.AddMember(rapidjson::Value(key, allocator), std::move(array), allocator);
        }
    }
    
    // ========================================
    // 객체/배열 존재 확인
    // ========================================
    
    inline bool hasKey(const char* key) const {
        return document_.HasMember(key);
    }
    
    inline bool isArray(const char* key) const {
        return document_.HasMember(key) && document_[key].IsArray();
    }
    
    inline bool isObject(const char* key) const {
        return document_.HasMember(key) && document_[key].IsObject();
    }
    
    // ========================================
    // Iteration 함수들
    // ========================================
    
    inline void iterateArray(const char* key, std::function<void(size_t index)> processor) const {
        if (document_.HasMember(key) && document_[key].IsArray()) {
            const auto& array = document_[key];
            for (size_t i = 0; i < array.Size(); ++i) {
                processor(i);
            }
        }
    }
    
    inline void iterateObject(const char* key, std::function<void(const std::string& key)> processor) const {
        if (document_.HasMember(key) && document_[key].IsObject()) {
            const auto& obj = document_[key];
            for (auto it = obj.MemberBegin(); it != obj.MemberEnd(); ++it) {
                processor(it->name.GetString());
            }
        }
    }
    
    // ========================================
    // Begin/End 스타일 구조적 JSON 생성
    // ========================================
    
    inline void beginObject(const char* key = nullptr) {
        ensureObject();
        auto& allocator = document_.GetAllocator();
        
        rapidjson::Value* targetObject = nullptr;
        
        if (contextStack_.empty()) {
            if (key) {
                rapidjson::Value newObj(rapidjson::kObjectType);
                rapidjson::Value keyVal(key, allocator);
                document_.AddMember(keyVal, newObj, allocator);
                targetObject = &document_[key];
            } else {
                targetObject = &document_;
            }
        } else {
            auto* current = getCurrentContext();
            rapidjson::Value newObj(rapidjson::kObjectType);
            
            if (contextStack_.back().isArray) {
                current->PushBack(newObj, allocator);
                targetObject = &(*current)[current->Size() - 1];
            } else {
                if (key) {
                    rapidjson::Value keyVal(key, allocator);
                    current->AddMember(keyVal, newObj, allocator);
                    targetObject = &(*current)[key];
                }
            }
        }
        
        if (targetObject) {
            pushContext(targetObject, false, key ? key : "");
        }
    }
    
    inline void endObject() {
        if (!contextStack_.empty() && !contextStack_.back().isArray) {
            contextStack_.pop_back();
        }
    }
    
    inline void beginArray(const char* key = nullptr) {
        ensureObject();
        auto& allocator = document_.GetAllocator();
        
        rapidjson::Value* targetArray = nullptr;
        
        if (contextStack_.empty()) {
            if (key) {
                rapidjson::Value newArray(rapidjson::kArrayType);
                rapidjson::Value keyVal(key, allocator);
                document_.AddMember(keyVal, newArray, allocator);
                targetArray = &document_[key];
            }
        } else {
            auto* current = getCurrentContext();
            rapidjson::Value newArray(rapidjson::kArrayType);
            
            if (contextStack_.back().isArray) {
                current->PushBack(newArray, allocator);
                targetArray = &(*current)[current->Size() - 1];
            } else {
                if (key) {
                    rapidjson::Value keyVal(key, allocator);
                    current->AddMember(keyVal, newArray, allocator);
                    targetArray = &(*current)[key];
                }
            }
        }
        
        if (targetArray) {
            pushContext(targetArray, true, key ? key : "");
        }
    }
    
    inline void endArray() {
        if (!contextStack_.empty() && contextStack_.back().isArray) {
            contextStack_.pop_back();
        }
    }
    
    // 배열 요소 추가 편의 메서드들
    inline void pushString(const std::string& value) {
        auto* current = getCurrentContext();
        if (current && current->IsArray()) {
            auto& allocator = document_.GetAllocator();
            rapidjson::Value valueVal(value.c_str(), allocator);
            current->PushBack(valueVal, allocator);
        }
    }
    
    inline void pushInt64(int64_t value) {
        auto* current = getCurrentContext();
        if (current && current->IsArray()) {
            auto& allocator = document_.GetAllocator();
            rapidjson::Value valueVal(value);
            current->PushBack(valueVal, allocator);
        }
    }
    
    inline void pushDouble(double value) {
        auto* current = getCurrentContext();
        if (current && current->IsArray()) {
            auto& allocator = document_.GetAllocator();
            rapidjson::Value valueVal(value);
            current->PushBack(valueVal, allocator);
        }
    }
    
    inline void pushBool(bool value) {
        auto* current = getCurrentContext();
        if (current && current->IsArray()) {
            auto& allocator = document_.GetAllocator();
            rapidjson::Value valueVal(value);
            current->PushBack(valueVal, allocator);
        }
    }
    
    inline void pushObject() {
        auto* current = getCurrentContext();
        if (current && current->IsArray()) {
            auto& allocator = document_.GetAllocator();
            rapidjson::Value newObj(rapidjson::kObjectType);
            current->PushBack(newObj, allocator);
            pushContext(&(*current)[current->Size() - 1], false, "");
        }
    }
    
    inline void pushArray() {
        auto* current = getCurrentContext();
        if (current && current->IsArray()) {
            auto& allocator = document_.GetAllocator();
            rapidjson::Value newArray(rapidjson::kArrayType);
            current->PushBack(newArray, allocator);
            pushContext(&(*current)[current->Size() - 1], true, "");
        }
    }
    
    // ========================================
    // 메타프로그래밍 기반 자동 처리
    // ========================================
    
    template<typename T>
    inline void setField(const char* key, const T& value) {
        static_assert(is_json_primitive_v<T>, 
                     "setField supports only JSON primitive types for type safety");
        
        if constexpr (std::is_same_v<T, std::string>) {
            setString(key, value);
        } else if constexpr (std::is_same_v<T, int>) {
            setInt64(key, static_cast<int64_t>(value));
        } else if constexpr (std::is_same_v<T, int64_t>) {
            setInt64(key, value);
        } else if constexpr (std::is_same_v<T, double>) {
            setDouble(key, value);
        } else if constexpr (std::is_same_v<T, float>) {
            setFloat(key, value);
        } else if constexpr (std::is_same_v<T, bool>) {
            setBool(key, value);
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            setUInt32(key, value);
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            setUInt64(key, value);
        }
    }
    
    template<typename T>
    inline T getField(const char* key) const {
        static_assert(is_json_primitive_v<T>, 
                     "getField supports only JSON primitive types for type safety");
        
        if constexpr (std::is_same_v<T, std::string>) {
            return getString(key);
        } else if constexpr (std::is_same_v<T, int>) {
            return static_cast<int>(getInt64(key));
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return getInt64(key);
        } else if constexpr (std::is_same_v<T, double>) {
            return getDouble(key);
        } else if constexpr (std::is_same_v<T, float>) {
            return getFloat(key);
        } else if constexpr (std::is_same_v<T, bool>) {
            return getBool(key);
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            return getUInt32(key);
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return getUInt64(key);
        } else {
            return T{};
        }
    }
    
    // ========================================
    // Optional getter (안전한 접근)
    // ========================================
    
    inline std::optional<std::string> getOptionalString(const char* key) const {
        if (hasKey(key)) {
            return getString(key);
        }
        return std::nullopt;
    }
    
    inline std::optional<int64_t> getOptionalInt64(const char* key) const {
        if (hasKey(key)) {
            return getInt64(key);
        }
        return std::nullopt;
    }
    
    inline std::optional<double> getOptionalDouble(const char* key) const {
        if (hasKey(key)) {
            return getDouble(key);
        }
        return std::nullopt;
    }
    
    inline std::optional<bool> getOptionalBool(const char* key) const {
        if (hasKey(key)) {
            return getBool(key);
        }
        return std::nullopt;
    }

protected:
    // ========================================
    // 내부 헬퍼 함수들
    // ========================================
    
    inline void ensureObject() {
        if (!document_.IsObject()) {
            document_.SetObject();
        }
    }
    
    inline rapidjson::Value* getCurrentContext() {
        if (!contextStack_.empty()) {
            return contextStack_.back().current;
        }
        return nullptr;
    }
    
    inline void pushContext(rapidjson::Value* value, bool isArray, const std::string& key) {
        contextStack_.push_back({value, isArray, key});
    }
    
    // JSON 문자열 변환
    inline std::string documentToString() const {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document_.Accept(writer);
        return buffer.GetString();
    }
    
    // JSON 문자열 파싱
    inline void parseFromString(const std::string& jsonStr) {
        document_.Parse(jsonStr.c_str());
        contextStack_.clear(); // 파싱 후 컨텍스트 초기화
    }
    
    // ========================================
    // 타입 변환 헬퍼들
    // ========================================
    
    template<typename T>
    inline T convertFromValue(const rapidjson::Value& value) const {
        if constexpr (std::is_same_v<T, std::string>) {
            return value.IsString() ? value.GetString() : std::string{};
        } else if constexpr (std::is_same_v<T, int>) {
            return value.IsNumber() ? static_cast<int>(value.GetInt64()) : int{};
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return value.IsNumber() ? value.GetInt64() : int64_t{};
        } else if constexpr (std::is_same_v<T, double>) {
            return value.IsNumber() ? value.GetDouble() : double{};
        } else if constexpr (std::is_same_v<T, float>) {
            return value.IsNumber() ? static_cast<float>(value.GetDouble()) : float{};
        } else if constexpr (std::is_same_v<T, bool>) {
            return value.IsBool() ? value.GetBool() : bool{};
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            return value.IsNumber() ? static_cast<uint32_t>(value.GetUint64()) : uint32_t{};
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return value.IsNumber() ? value.GetUint64() : uint64_t{};
        } else {
            return T{};
        }
    }
    
    template<typename T>
    inline rapidjson::Value convertToValue(const T& item) const {
        auto& allocator = const_cast<rapidjson::Document&>(document_).GetAllocator();
        
        if constexpr (std::is_same_v<T, std::string>) {
            return rapidjson::Value(item.c_str(), allocator);
        } else if constexpr (std::is_arithmetic_v<T>) {
            return rapidjson::Value(item);
        } else {
            return rapidjson::Value{};
        }
    }
};

} // namespace json

// 모든 구현이 이 파일에 inline으로 포함됨 