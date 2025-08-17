#pragma once

/**
 * ToJsonable.hpp - JSON 직렬화 전용 클래스
 * 
 * 역할: 객체 → JSON 문자열 변환 책임
 */

#include "JsonableBase.hpp"

namespace json {

/**
 * @brief JSON 직렬화 전용 클래스
 * 
 * 책임:
 * - 객체 데이터 → JSON 문자열 변환
 * - 사용자 정의 saveToJson() 인터페이스 제공
 * - Begin/End 스타일 JSON 생성 관리
 * 
 * 상속: JsonableBase (기본 JSON 조작 기능)
 */
class ToJsonable : public virtual JsonableBase {
protected:
    // 파생 클래스에서만 생성 가능
    ToJsonable() = default;
    virtual ~ToJsonable() = default;

public:
    // ========================================
    // JSON 직렬화 핵심 인터페이스
    // ========================================
    
    /**
     * @brief 객체에서 JSON 문자열로 직렬화
     * 
     * @return JSON 문자열
     * 
     * 내부 동작:
     * 1. saveToJson() 호출하여 사용자가 데이터 저장
     * 2. 내부 document를 JSON 문자열로 변환
     */
    virtual std::string toJson() const {
        // 사용자 정의 직렬화 로직 호출
        const_cast<ToJsonable*>(this)->saveToJson();
        
        // 내부 document를 JSON 문자열로 변환
        return documentToString();
    }
    
    /**
     * @brief 데이터를 내부 JSON 객체로 저장 (사용자 구현 필수)
     * 
     * 사용자는 이 메서드에서:
     * - setString(), setInt64() 등으로 JSON 필드 설정
     * - setArray<T>()로 배열 데이터 설정
     * - beginObject()/endObject()로 중첩 객체 생성
     * - beginArray()/endArray()로 배열 생성
     * 
     * 예시 (일반 방식):
     * @code
     * void saveToJson() override {
     *     setString("name", name_);
     *     setInt64("age", static_cast<int64_t>(age_));
     *     setArray("hobbies", hobbies_);
     * }
     * @endcode
     * 
     * 예시 (Begin/End 방식):
     * @code
     * void saveToJson() override {
     *     beginObject();
     *     {
     *         setString("name", name_);
     *         setInt64("age", static_cast<int64_t>(age_));
     *         
     *         beginArray("hobbies");
     *         {
     *             for (const auto& hobby : hobbies_) {
     *                 setString("", hobby);  // 배열 컨텍스트
     *             }
     *         }
     *         endArray();
     *     }
     *     endObject();
     * }
     * @endcode
     */
    virtual void saveToJson() = 0;
    
    // ========================================
    // 편의 메서드들 (JsonableImpl에서 상속됨)
    // ========================================
    
    // 이미 JsonableBase에서 제공되므로 여기서는 주석으로만 명시
    // setString(key, value), setInt64(key, value), setArray<T>(key, values) 등
    // beginObject(key), endObject(), beginArray(key), endArray()
    // pushString(value), pushInt64(value) 등
    // setField<T>(key, value) 등
    
    // ========================================
    // 고급 직렬화 유틸리티
    // ========================================
    
    /**
     * @brief 조건부 필드 저장
     * 
     * @param key 필드명
     * @param value 저장할 값
     * @param condition 저장 조건 (true일 때만 저장)
     */
    template<typename T>
    void saveFieldIf(const char* key, const T& value, bool condition);
    
    /**
     * @brief 조건부 필드 저장 (값 기반 조건)
     * 
     * @param key 필드명
     * @param value 저장할 값
     * @param predicate 저장 조건 함수
     */
    template<typename T>
    void saveFieldIf(const char* key, const T& value, std::function<bool(const T&)> predicate);
    
    /**
     * @brief 배열 필드 조건부 저장
     * 
     * @param key 배열 필드명
     * @param values 저장할 배열
     * @param filter 필터 함수 (선택적)
     */
    template<typename T>
    void saveArrayField(const char* key, const std::vector<T>& values, 
                       std::function<bool(const T&)> filter = nullptr);
    
    /**
     * @brief 중첩 객체 저장 헬퍼
     * 
     * @param key 객체 필드명
     * @param saver 저장 함수
     */
    void saveNestedObject(const char* key, std::function<void()> saver) {
        beginObject(key);
        if (saver) saver();
        endObject();
    }
    
    /**
     * @brief 중첩 배열 저장 헬퍼
     * 
     * @param key 배열 필드명
     * @param saver 저장 함수
     */
    void saveNestedArray(const char* key, std::function<void()> saver) {
        beginArray(key);
        if (saver) saver();
        endArray();
    }

protected:
    /**
     * @brief JSON 직렬화 전 호출되는 가상 함수
     * 
     * 파생 클래스에서 오버라이드하여 전처리 로직 추가 가능
     */
    virtual void onBeforeSerialize() {}
    
    /**
     * @brief JSON 직렬화 후 호출되는 가상 함수
     * 
     * 파생 클래스에서 오버라이드하여 후처리 로직 추가 가능
     */
    virtual void onAfterSerialize() {}
    
    /**
     * @brief 직렬화 오류 시 호출되는 가상 함수
     * 
     * 파생 클래스에서 오버라이드하여 커스텀 오류 처리 가능
     */
    virtual void onSerializeError(const std::string& error) { (void)error; }
};

/**
 * 템플릿 메서드 구현
 */
template<typename T>
void ToJsonable::saveFieldIf(const char* key, const T& value, bool condition) {
    if (condition) {
        if constexpr (std::is_same_v<T, std::string>) {
            setString(key, value);
        } else if constexpr (std::is_same_v<T, int>) {
            setInt64(key, static_cast<int64_t>(value));
        } else if constexpr (std::is_same_v<T, int64_t>) {
            setInt64(key, value);
        } else if constexpr (std::is_same_v<T, double>) {
            setDouble(key, value);
        } else if constexpr (std::is_same_v<T, bool>) {
            setBool(key, value);
        } else {
            static_assert(std::is_same_v<T, void>, "Unsupported type for saveFieldIf");
        }
    }
}

template<typename T>
void ToJsonable::saveFieldIf(const char* key, const T& value, std::function<bool(const T&)> predicate) {
    if (predicate && predicate(value)) {
        saveFieldIf(key, value, true);
    }
}

template<typename T>
void ToJsonable::saveArrayField(const char* key, const std::vector<T>& values, 
                               std::function<bool(const T&)> filter) {
    if (filter) {
        std::vector<T> filteredValues;
        for (const auto& value : values) {
            if (filter(value)) {
                filteredValues.push_back(value);
            }
        }
        setArray(key, filteredValues);
    } else {
        setArray(key, values);
    }
}

} // namespace json 