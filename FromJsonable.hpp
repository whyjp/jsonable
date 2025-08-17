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
    // 편의 메서드들 (JsonableBase에서 상속됨)
    // ========================================
    
    // 이미 JsonableBase에서 제공되므로 여기서는 주석으로만 명시
    // getString(key), getInt64(key), getArray<T>(key) 등
    // hasKey(key), isArray(key), isObject(key)
    // iterateArray(key, func), iterateObject(key, func)
    
    

protected:
    // 파생 클래스 전용 영역 (필요시 확장)
};


} // namespace json 