#pragma once

/**
 * FromJsonable.hpp - JSON 역직렬화 전용 클래스
 * 
 * 역할: JSON 문자열 → 객체 변환 책임
 */

#include "JsonableBase.hpp"

namespace json {

/**
 * @brief JSON 역직렬화 전용 클래스
 * 
 * 책임:
 * - JSON 문자열 파싱
 * - JSON → 객체 데이터 로딩
 * - 사용자 정의 loadFromJson() 인터페이스 제공
 * 
 * 상속: JsonableBase (기본 JSON 조작 기능)
 */
class FromJsonable : public virtual JsonableBase {
protected:
    // 파생 클래스에서만 생성 가능
    FromJsonable() = default;
    virtual ~FromJsonable() = default;

public:
    // ========================================
    // JSON 역직렬화 핵심 인터페이스
    // ========================================
    
    /**
     * @brief JSON 문자열에서 객체로 역직렬화
     * 
     * @param jsonStr JSON 문자열
     * 
     * 내부 동작:
     * 1. JSON 문자열 파싱하여 내부 document 설정
     * 2. loadFromJson() 호출하여 사용자가 데이터 로드
     */
    virtual void fromJson(const std::string& jsonStr) {
        // JSON 문자열 파싱하여 내부 document 설정
        parseFromString(jsonStr);
        
        // 사용자 정의 역직렬화 로직 호출
        loadFromJson();
    }
    
    /**
     * @brief 내부 JSON 객체에서 데이터 로드 (사용자 구현 필수)
     * 
     * 사용자는 이 메서드에서:
     * - getString(), getInt64() 등으로 JSON 필드 읽기
     * - getArray<T>()로 배열 데이터 읽기
     * - iterateArray(), iterateObject()로 복잡한 구조 처리
     * 
     * 예시:
     * @code
     * void loadFromJson() override {
     *     name_ = getString("name");
     *     age_ = static_cast<int>(getInt64("age"));
     *     hobbies_ = getArray<std::string>("hobbies");
     * }
     * @endcode
     */
    virtual void loadFromJson() = 0;
    
    // ========================================
    // 편의 메서드들 (JsonableImpl에서 상속됨)
    // ========================================
    
    // 이미 JsonableBase에서 제공되므로 여기서는 주석으로만 명시
    // getString(key), getInt64(key), getArray<T>(key) 등
    // hasKey(key), isArray(key), isObject(key)
    // iterateArray(key, func), iterateObject(key, func)
    // getOptionalString(key) 등
    
    // ========================================
    // 고급 로딩 유틸리티
    // ========================================
    
    /**
     * @brief 조건부 필드 로딩
     * 
     * @param key 필드명
     * @param target 로딩할 변수 참조
     * @param validator 검증 함수 (선택적)
     * @return 로딩 성공 여부
     */
    template<typename T>
    bool loadField(const char* key, T& target, std::function<bool(const T&)> validator = nullptr);
    
    /**
     * @brief 배열 필드 조건부 로딩
     * 
     * @param key 배열 필드명
     * @param target 로딩할 벡터 참조
     * @param maxSize 최대 크기 (0 = 제한 없음)
     * @return 로딩 성공 여부
     */
    template<typename T>
    bool loadArrayField(const char* key, std::vector<T>& target, size_t maxSize = 0);
    

protected:
    /**
     * @brief JSON 파싱 오류 시 호출되는 가상 함수
     * 
     * 파생 클래스에서 오버라이드하여 커스텀 오류 처리 가능
     */
    virtual void onParseError(const std::string& error) { (void)error; }
    
    /**
     * @brief 필드 로딩 실패 시 호출되는 가상 함수
     * 
     * 파생 클래스에서 오버라이드하여 커스텀 오류 처리 가능
     */
    virtual void onLoadFieldError(const char* key, const std::string& error) { (void)key; (void)error; }
};

/**
 * 템플릿 메서드 구현
 */
template<typename T>
bool FromJsonable::loadField(const char* key, T& target, std::function<bool(const T&)> validator) {
    if (!hasKey(key)) {
        return false;
    }
    
    try {
        if constexpr (std::is_same_v<T, std::string>) {
            target = getString(key);
        } else if constexpr (std::is_same_v<T, int>) {
            target = static_cast<int>(getInt64(key));
        } else if constexpr (std::is_same_v<T, int64_t>) {
            target = getInt64(key);
        } else if constexpr (std::is_same_v<T, double>) {
            target = getDouble(key);
        } else if constexpr (std::is_same_v<T, bool>) {
            target = getBool(key);
        } else {
            static_assert(std::is_same_v<T, void>, "Unsupported type for loadField");
        }
        
        if (validator && !validator(target)) {
            onLoadFieldError(key, "Validation failed");
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        onLoadFieldError(key, e.what());
        return false;
    }
}

template<typename T>
bool FromJsonable::loadArrayField(const char* key, std::vector<T>& target, size_t maxSize) {
    if (!hasKey(key) || !isArray(key)) {
        return false;
    }
    
    try {
        target = getArray<T>(key);
        
        if (maxSize > 0 && target.size() > maxSize) {
            target.resize(maxSize);
            onLoadFieldError(key, "Array size exceeded limit, truncated");
        }
        
        return true;
    } catch (const std::exception& e) {
        onLoadFieldError(key, e.what());
        return false;
    }
}

} // namespace json 