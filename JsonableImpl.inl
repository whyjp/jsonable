// JsonableCleanImpl.inl - 내부 구현부 (사용자 직접 include 금지)
// 이 파일은 JSONABLE_IMPLEMENTATION이 정의된 경우에만 포함됩니다.

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace json {

// ========================================
// Jsonable::Impl 클래스 정의 (PIMPL)
// ========================================

class Jsonable::Impl {
private:
    rapidjson::Document document_;
    
    // 컨텍스트 스택 관리 (Begin/End 스타일용)
    struct JsonContext {
        rapidjson::Value* current;
        bool isArray;
        std::string key;
    };
    std::vector<JsonContext> contextStack_;
    
public:
    Impl() {
        document_.SetObject();
    }
    
    ~Impl() = default;
    
    // 복사/이동 생성자
    Impl(const Impl& other) : document_() {
        document_.CopyFrom(other.document_, document_.GetAllocator());
    }
    
    Impl(Impl&& other) noexcept : document_(std::move(other.document_)) {}
    
    Impl& operator=(const Impl& other) {
        if (this != &other) {
            document_.CopyFrom(other.document_, document_.GetAllocator());
        }
        return *this;
    }
    
    Impl& operator=(Impl&& other) noexcept {
        if (this != &other) {
            document_ = std::move(other.document_);
        }
        return *this;
    }
    
    // ========================================
    // JSON 파싱 및 문자열 변환
    // ========================================
    
    void parseFromString(const std::string& jsonStr) {
        document_.Parse(jsonStr.c_str());
        if (document_.HasParseError()) {
            document_.SetObject(); // 파싱 실패 시 빈 객체로 초기화
        }
    }
    
    std::string toString() const {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document_.Accept(writer);
        return buffer.GetString();
    }
    
    // ========================================
    // 기본 타입 읽기 구현
    // ========================================
    
    std::string getString(const char* key, const std::string& defaultValue) const {
        if (document_.HasMember(key) && document_[key].IsString()) {
            return document_[key].GetString();
        }
        return defaultValue;
    }
    
    int64_t getInt64(const char* key, int64_t defaultValue) const {
        if (document_.HasMember(key) && document_[key].IsNumber()) {
            if (document_[key].IsInt64()) {
                return document_[key].GetInt64();
            } else if (document_[key].IsInt()) {
                return static_cast<int64_t>(document_[key].GetInt());
            }
        }
        return defaultValue;
    }
    
    double getDouble(const char* key, double defaultValue) const {
        if (document_.HasMember(key) && document_[key].IsNumber()) {
            return document_[key].GetDouble();
        }
        return defaultValue;
    }
    
    float getFloat(const char* key, float defaultValue) const {
        if (document_.HasMember(key) && document_[key].IsNumber()) {
            return document_[key].GetFloat();
        }
        return defaultValue;
    }
    
    bool getBool(const char* key, bool defaultValue) const {
        if (document_.HasMember(key) && document_[key].IsBool()) {
            return document_[key].GetBool();
        }
        return defaultValue;
    }
    
    uint32_t getUInt32(const char* key, uint32_t defaultValue) const {
        if (document_.HasMember(key) && document_[key].IsNumber()) {
            if (document_[key].IsUint()) {
                return document_[key].GetUint();
            } else if (document_[key].IsInt()) {
                int value = document_[key].GetInt();
                if (value >= 0) {
                    return static_cast<uint32_t>(value);
                }
            }
        }
        return defaultValue;
    }
    
    uint64_t getUInt64(const char* key, uint64_t defaultValue) const {
        if (document_.HasMember(key) && document_[key].IsNumber()) {
            if (document_[key].IsUint64()) {
                return document_[key].GetUint64();
            } else if (document_[key].IsInt64()) {
                int64_t value = document_[key].GetInt64();
                if (value >= 0) {
                    return static_cast<uint64_t>(value);
                }
            }
        }
        return defaultValue;
    }
    
    // ========================================
    // 기본 타입 쓰기 구현
    // ========================================
    
    void setString(const char* key, const std::string& value) {
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
    
    void setInt64(const char* key, int64_t value) {
        auto& allocator = document_.GetAllocator();
        rapidjson::Value valueVal(value);
        
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
    
    void setDouble(const char* key, double value) {
        auto& allocator = document_.GetAllocator();
        rapidjson::Value valueVal(value);
        
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
    
    void setFloat(const char* key, float value) {
        setDouble(key, static_cast<double>(value));
    }
    
    void setBool(const char* key, bool value) {
        auto& allocator = document_.GetAllocator();
        rapidjson::Value valueVal(value);
        
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
    
    void setUInt32(const char* key, uint32_t value) {
        auto& allocator = document_.GetAllocator();
        rapidjson::Value valueVal(value);
        
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
    
    void setUInt64(const char* key, uint64_t value) {
        auto& allocator = document_.GetAllocator();
        rapidjson::Value valueVal(value);
        
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
    
    // ========================================
    // 배열 처리 (템플릿 특수화)
    // ========================================
    
    template<typename T>
    std::vector<T> getArray(const char* key) const {
        // 타입 안전성 보장: JSON 기본 타입만 허용
        static_assert(is_json_primitive_v<T>, 
                     "Array elements must be JSON primitive types only. "
                     "Mixed type arrays are not supported for type safety.");
        
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
    void setArray(const char* key, const std::vector<T>& values) {
        // 타입 안전성 보장: JSON 기본 타입만 허용
        static_assert(is_json_primitive_v<T>, 
                     "Array elements must be JSON primitive types only. "
                     "Mixed type arrays are not supported for type safety.");
        
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
    // Begin/End 스타일 구현 (전체 복사)
    // ========================================
    
    void beginObject(const char* key) {
        ensureObject();
        auto& allocator = document_.GetAllocator();
        
        rapidjson::Value* targetObject = nullptr;
        
        if (contextStack_.empty()) {
            // 루트 레벨
            if (key) {
                rapidjson::Value newObj(rapidjson::kObjectType);
                rapidjson::Value keyVal(key, allocator);
                document_.AddMember(keyVal, newObj, allocator);
                targetObject = &document_[key];
            } else {
                targetObject = &document_;
            }
        } else {
            // 중첩 레벨 처리
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
    
    void endObject() {
        if (!contextStack_.empty() && !contextStack_.back().isArray) {
            contextStack_.pop_back();
        }
    }
    
    void beginArray(const char* key) {
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
    
    void endArray() {
        if (!contextStack_.empty() && contextStack_.back().isArray) {
            contextStack_.pop_back();
        }
    }
    
    // 나머지 Begin/End 관련 메서드들도 구현
    void setCurrentString(const char* key, const std::string& value) {
        auto* current = getCurrentContext();
        if (current && current->IsObject()) {
            auto& allocator = document_.GetAllocator();
            rapidjson::Value keyVal(key, allocator);
            rapidjson::Value valueVal(value.c_str(), allocator);
            current->AddMember(keyVal, valueVal, allocator);
        }
    }
    
    void setCurrentInt64(const char* key, int64_t value) {
        auto* current = getCurrentContext();
        if (current && current->IsObject()) {
            auto& allocator = document_.GetAllocator();
            rapidjson::Value keyVal(key, allocator);
            rapidjson::Value valueVal(value);
            current->AddMember(keyVal, valueVal, allocator);
        }
    }
    
    void setCurrentDouble(const char* key, double value) {
        auto* current = getCurrentContext();
        if (current && current->IsObject()) {
            auto& allocator = document_.GetAllocator();
            rapidjson::Value keyVal(key, allocator);
            rapidjson::Value valueVal(value);
            current->AddMember(keyVal, valueVal, allocator);
        }
    }
    
    void setCurrentBool(const char* key, bool value) {
        auto* current = getCurrentContext();
        if (current && current->IsObject()) {
            auto& allocator = document_.GetAllocator();
            rapidjson::Value keyVal(key, allocator);
            rapidjson::Value valueVal(value);
            current->AddMember(keyVal, valueVal, allocator);
        }
    }
    
    void pushString(const std::string& value) {
        auto* current = getCurrentContext();
        if (current && current->IsArray()) {
            auto& allocator = document_.GetAllocator();
            rapidjson::Value valueVal(value.c_str(), allocator);
            current->PushBack(valueVal, allocator);
        }
    }
    
    void pushInt64(int64_t value) {
        auto* current = getCurrentContext();
        if (current && current->IsArray()) {
            auto& allocator = document_.GetAllocator();
            rapidjson::Value valueVal(value);
            current->PushBack(valueVal, allocator);
        }
    }
    
    void pushDouble(double value) {
        auto* current = getCurrentContext();
        if (current && current->IsArray()) {
            auto& allocator = document_.GetAllocator();
            rapidjson::Value valueVal(value);
            current->PushBack(valueVal, allocator);
        }
    }
    
    void pushBool(bool value) {
        auto* current = getCurrentContext();
        if (current && current->IsArray()) {
            auto& allocator = document_.GetAllocator();
            rapidjson::Value valueVal(value);
            current->PushBack(valueVal, allocator);
        }
    }
    
    void pushObject() {
        auto* current = getCurrentContext();
        if (current && current->IsArray()) {
            auto& allocator = document_.GetAllocator();
            rapidjson::Value newObj(rapidjson::kObjectType);
            current->PushBack(newObj, allocator);
            
            pushContext(&(*current)[current->Size() - 1], false);
        }
    }
    
    void pushArray() {
        auto* current = getCurrentContext();
        if (current && current->IsArray()) {
            auto& allocator = document_.GetAllocator();
            rapidjson::Value newArray(rapidjson::kArrayType);
            current->PushBack(newArray, allocator);
            
            pushContext(&(*current)[current->Size() - 1], true);
        }
    }
    
    // 나머지 메서드들도 구현...
    bool hasKey(const char* key) const {
        return document_.HasMember(key);
    }
    
    bool isArray(const char* key) const {
        return document_.HasMember(key) && document_[key].IsArray();
    }
    
    bool isObject(const char* key) const {
        return document_.HasMember(key) && document_[key].IsObject();
    }
    
private:
    void ensureObject() {
        if (!document_.IsObject()) {
            document_.SetObject();
        }
    }
    
    rapidjson::Value* getCurrentContext() {
        if (contextStack_.empty()) {
            return &document_;
        }
        return contextStack_.back().current;
    }
    
    void pushContext(rapidjson::Value* value, bool isArray, const std::string& key = "") {
        contextStack_.push_back({value, isArray, key});
    }
    
    template<typename T>
    T convertFromValue(const rapidjson::Value& value) const {
        if constexpr (std::is_same_v<T, std::string>) {
            return value.IsString() ? std::string(value.GetString()) : std::string();
        }
        else if constexpr (std::is_same_v<T, int>) {
            return value.IsInt() ? value.GetInt() : 0;
        }
        else if constexpr (std::is_same_v<T, int64_t>) {
            return value.IsInt64() ? value.GetInt64() : 0;
        }
        else if constexpr (std::is_same_v<T, double>) {
            return value.IsDouble() ? value.GetDouble() : 0.0;
        }
        else if constexpr (std::is_same_v<T, bool>) {
            return value.IsBool() ? value.GetBool() : false;
        }
        else {
            return T{};
        }
    }
    
    template<typename T>
    rapidjson::Value convertToValue(const T& item) const {
        auto& allocator = document_.GetAllocator();
        
        if constexpr (std::is_same_v<T, std::string>) {
            return rapidjson::Value(item.c_str(), allocator);
        }
        else if constexpr (std::is_arithmetic_v<T>) {
            return rapidjson::Value(item);
        }
        else {
            return rapidjson::Value();
        }
    }
};

// ========================================
// Jsonable 클래스 PIMPL 구현
// ========================================

inline Jsonable::Jsonable() : pImpl(std::make_unique<Impl>()) {}

inline Jsonable::~Jsonable() = default;

inline Jsonable::Jsonable(const Jsonable& other) 
    : pImpl(std::make_unique<Impl>(*other.pImpl)) {}

inline Jsonable::Jsonable(Jsonable&& other) noexcept 
    : pImpl(std::move(other.pImpl)) {}

inline Jsonable& Jsonable::operator=(const Jsonable& other) {
    if (this != &other) {
        pImpl = std::make_unique<Impl>(*other.pImpl);
    }
    return *this;
}

inline Jsonable& Jsonable::operator=(Jsonable&& other) noexcept {
    if (this != &other) {
        pImpl = std::move(other.pImpl);
    }
    return *this;
}

inline Jsonable::Impl& Jsonable::getImpl() {
    return *pImpl;
}

inline const Jsonable::Impl& Jsonable::getImpl() const {
    return *pImpl;
}

// ========================================
// 모든 메서드 구현 (PIMPL 위임)
// ========================================

inline void Jsonable::fromJson(const std::string& jsonStr) {
    pImpl->parseFromString(jsonStr);
    loadFromJson();
}

inline std::string Jsonable::toJson() const {
    const_cast<Jsonable*>(this)->saveToJson();
    return pImpl->toString();
}

// 기본 타입 읽기
inline std::string Jsonable::getString(const char* key, const std::string& defaultValue) const {
    return pImpl->getString(key, defaultValue);
}

inline int64_t Jsonable::getInt64(const char* key, int64_t defaultValue) const {
    return pImpl->getInt64(key, defaultValue);
}

inline double Jsonable::getDouble(const char* key, double defaultValue) const {
    return pImpl->getDouble(key, defaultValue);
}

inline float Jsonable::getFloat(const char* key, float defaultValue) const {
    return pImpl->getFloat(key, defaultValue);
}

inline bool Jsonable::getBool(const char* key, bool defaultValue) const {
    return pImpl->getBool(key, defaultValue);
}

inline uint32_t Jsonable::getUInt32(const char* key, uint32_t defaultValue) const {
    return pImpl->getUInt32(key, defaultValue);
}

inline uint64_t Jsonable::getUInt64(const char* key, uint64_t defaultValue) const {
    return pImpl->getUInt64(key, defaultValue);
}

// 기본 타입 쓰기
inline void Jsonable::setString(const char* key, const std::string& value) {
    pImpl->setString(key, value);
}

inline void Jsonable::setInt64(const char* key, int64_t value) {
    pImpl->setInt64(key, value);
}

inline void Jsonable::setDouble(const char* key, double value) {
    pImpl->setDouble(key, value);
}

inline void Jsonable::setFloat(const char* key, float value) {
    pImpl->setFloat(key, value);
}

inline void Jsonable::setBool(const char* key, bool value) {
    pImpl->setBool(key, value);
}

inline void Jsonable::setUInt32(const char* key, uint32_t value) {
    pImpl->setUInt32(key, value);
}

inline void Jsonable::setUInt64(const char* key, uint64_t value) {
    pImpl->setUInt64(key, value);
}

// Begin/End API
inline void Jsonable::beginObject(const char* key) {
    pImpl->beginObject(key);
}

inline void Jsonable::endObject() {
    pImpl->endObject();
}

inline void Jsonable::beginArray(const char* key) {
    pImpl->beginArray(key);
}

inline void Jsonable::endArray() {
    pImpl->endArray();
}

inline void Jsonable::setCurrentString(const char* key, const std::string& value) {
    pImpl->setCurrentString(key, value);
}

inline void Jsonable::setCurrentInt64(const char* key, int64_t value) {
    pImpl->setCurrentInt64(key, value);
}

inline void Jsonable::setCurrentDouble(const char* key, double value) {
    pImpl->setCurrentDouble(key, value);
}

inline void Jsonable::setCurrentBool(const char* key, bool value) {
    pImpl->setCurrentBool(key, value);
}

inline void Jsonable::pushString(const std::string& value) {
    pImpl->pushString(value);
}

inline void Jsonable::pushInt64(int64_t value) {
    pImpl->pushInt64(value);
}

inline void Jsonable::pushDouble(double value) {
    pImpl->pushDouble(value);
}

inline void Jsonable::pushBool(bool value) {
    pImpl->pushBool(value);
}

inline void Jsonable::pushObject() {
    pImpl->pushObject();
}

inline void Jsonable::pushArray() {
    pImpl->pushArray();
}

// 객체 상태 확인
inline bool Jsonable::hasKey(const char* key) const {
    return pImpl->hasKey(key);
}

inline bool Jsonable::isArray(const char* key) const {
    return pImpl->isArray(key);
}

inline bool Jsonable::isObject(const char* key) const {
    return pImpl->isObject(key);
}

// 템플릿 메서드들
template<typename T>
std::vector<T> Jsonable::getArray(const char* key) const {
    return pImpl->getArray<T>(key);
}

template<typename T>
void Jsonable::setArray(const char* key, const std::vector<T>& values) {
    pImpl->setArray<T>(key, values);
}

// 메타프로그래밍 기반 자동 처리
template<typename T>
void Jsonable::setField(const char* key, const T& value) {
    // 타입 안전성 보장: JSON 기본 타입만 허용
    static_assert(is_json_primitive_v<T>, 
                 "setField supports only JSON primitive types for type safety. "
                 "Use specific setter methods for custom types.");
    
    if constexpr (std::is_same_v<T, std::string>) {
        setString(key, value);
    }
    else if constexpr (std::is_same_v<T, int>) {
        setInt64(key, static_cast<int64_t>(value));
    }
    else if constexpr (std::is_same_v<T, int64_t>) {
        setInt64(key, value);
    }
    else if constexpr (std::is_same_v<T, double>) {
        setDouble(key, value);
    }
    else if constexpr (std::is_same_v<T, float>) {
        setFloat(key, value);
    }
    else if constexpr (std::is_same_v<T, bool>) {
        setBool(key, value);
    }
    else if constexpr (std::is_same_v<T, uint32_t>) {
        setUInt32(key, value);
    }
    else if constexpr (std::is_same_v<T, uint64_t>) {
        setUInt64(key, value);
    }
}

template<typename T>
T Jsonable::getField(const char* key) const {
    // 타입 안전성 보장: JSON 기본 타입만 허용
    static_assert(is_json_primitive_v<T>, 
                 "getField supports only JSON primitive types for type safety. "
                 "Use specific getter methods for custom types.");
    
    if constexpr (std::is_same_v<T, std::string>) {
        return getString(key);
    }
    else if constexpr (std::is_same_v<T, int>) {
        return static_cast<int>(getInt64(key));
    }
    else if constexpr (std::is_same_v<T, int64_t>) {
        return getInt64(key);
    }
    else if constexpr (std::is_same_v<T, double>) {
        return getDouble(key);
    }
    else if constexpr (std::is_same_v<T, float>) {
        return getFloat(key);
    }
    else if constexpr (std::is_same_v<T, bool>) {
        return getBool(key);
    }
    else if constexpr (std::is_same_v<T, uint32_t>) {
        return getUInt32(key);
    }
    else if constexpr (std::is_same_v<T, uint64_t>) {
        return getUInt64(key);
    }
    else {
        return T{};
    }
}

} // namespace json 