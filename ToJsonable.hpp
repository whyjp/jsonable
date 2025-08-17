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
    // 편의 메서드들 (JsonableBase에서 상속됨)
    // ========================================
    
    // 이미 JsonableBase에서 제공되므로 여기서는 주석으로만 명시
    // setString(key, value), setInt64(key, value), setArray<T>(key, values) 등
    // beginObject(key), endObject(), beginArray(key), endArray()
    // pushString(value), pushInt64(value) 등
    
    

protected:
    // 파생 클래스 전용 영역 (필요시 확장)
};


} // namespace json 