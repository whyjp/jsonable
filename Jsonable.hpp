#pragma once

/**
 * Jsonable.hpp - 최종 사용자 인터페이스
 * 
 * 역할: ToJsonable + FromJsonable 다중상속으로 완전한 JSON 처리 제공
 */

#include "ToJsonable.hpp"
#include "FromJsonable.hpp"
#include <type_traits>
#include <utility>

namespace json {

// ========================================
// 타입 트레이트 (컴파일 타임 타입 검증)
// ========================================

// 전방 선언
class Jsonable;

/**
 * @brief Jsonable 상속 체크
 */
template<typename T>
struct is_jsonable : std::is_base_of<Jsonable, T> {};

template<typename T>
constexpr bool is_jsonable_v = is_jsonable<T>::value;

// is_json_primitive_v는 JsonableBase.hpp에서 정의됨

/**
 * @brief 최종 사용자 인터페이스 - 완전한 JSON 처리
 * 
 * 다중상속 구조:
 * - ToJsonable: 객체 → JSON 직렬화 책임
 * - FromJsonable: JSON → 객체 역직렬화 책임
 * - JsonableBase: 기본 JSON 조작 (virtual 상속으로 다이아몬드 문제 해결)
 * 
 * 사용자는 이 클래스만 상속받으면 됨:
 * @code
 * class MyClass : public json::Jsonable {
 *     void saveToJson() override { ... }   // ToJsonable에서
 *     void loadFromJson() override { ... } // FromJsonable에서
 * };
 * @endcode
 */
class Jsonable : public ToJsonable, public FromJsonable {
public:
    // ========================================
    // 생성자/소멸자
    // ========================================
    
    /**
     * @brief 기본 생성자
     */
    Jsonable() = default;
    
    /**
     * @brief 가상 소멸자 (다형성 지원)
     */
    virtual ~Jsonable() = default;
    
    /**
     * @brief 복사 생성자
     */
    Jsonable(const Jsonable& other) = default;
    
    /**
     * @brief 이동 생성자
     */
    Jsonable(Jsonable&& other) noexcept = default;
    
    /**
     * @brief 복사 할당 연산자
     */
    Jsonable& operator=(const Jsonable& other) = default;
    
    /**
     * @brief 이동 할당 연산자
     */
    Jsonable& operator=(Jsonable&& other) noexcept = default;
    
    // ========================================
    // 편의 생성자들
    // ========================================
    
    /**
     * @brief JSON 문자열로부터 생성
     * 
     * @param jsonStr JSON 문자열
     * 
     * 주의: 생성자에서는 virtual function을 호출할 수 없으므로
     * 객체 생성 후 명시적으로 fromJson() 호출 필요
     */
    explicit Jsonable(const std::string& jsonStr) {
        // 생성자에서 virtual function 호출은 위험하므로 제거
        // 사용자는 객체 생성 후 fromJson()을 명시적으로 호출해야 함
    }
    
    // ========================================
    // 상속받은 순수 가상 함수들 (사용자 구현 필수)
    // ========================================
    
    /**
     * @brief JSON → 객체 데이터 로딩 (FromJsonable에서 상속)
     * 
     * 이 메서드에서 JSON 필드들을 읽어서 멤버 변수에 할당
     */
    void loadFromJson() override = 0;
    
    /**
     * @brief 객체 데이터 → JSON 저장 (ToJsonable에서 상속)
     * 
     * 이 메서드에서 멤버 변수들을 JSON 필드로 설정
     */
    void saveToJson() override = 0;
    
    // ========================================
    // 편의 메서드들
    // ========================================
    
    
    /**
     * @brief 깊은 복사 생성
     * 
     * @return JSON을 통한 깊은 복사본
     */
    template<typename T>
    T deepCopy() const {
        static_assert(std::is_base_of_v<Jsonable, T>, "T must inherit from Jsonable");
        T copy;
        copy.fromJson(toJson());
        return copy;
    }
    
    // ========================================
    // 연산자 오버로딩
    // ========================================
    
    /**
     * @brief 등호 연산자 (JSON 기반 비교)
     */
    bool operator==(const Jsonable& other) const {
        return toJson() == other.toJson();
    }
    
    /**
     * @brief 부등호 연산자
     */
    bool operator!=(const Jsonable& other) const {
        return toJson() != other.toJson();
    }    
};


} // namespace json
