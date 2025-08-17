#pragma once

/**
 * Null Key 처리 안전성 데모
 * 
 * 배열 컨텍스트에서 다양한 key 값 처리 테스트
 */

#define JSONABLE_IMPLEMENTATION
#include "../Jsonable.hpp"

class NullKeyHandlingDemo : public json::Jsonable {
private:
    std::vector<std::string> testArray_;

public:
    NullKeyHandlingDemo() = default;
    
    void setupTest() {
        testArray_ = {"test1", "test2", "test3"};
    }

    void loadFromJson() override {
        testArray_ = getArray<std::string>("test_array");
    }

    void saveToJson() override {
        beginObject();
        {
            // ========================================
            // 🧪 배열 컨텍스트에서 다양한 key 처리 테스트
            // ========================================
            
            beginArray("test_array");
            {
                // ✅ 모든 방식이 동일한 결과 생성
                setString("", testArray_[0]);           // 빈 문자열
                setString(nullptr, testArray_[1]);      // nullptr
                setString("ignored_key", testArray_[2]); // 임의의 key (무시됨)
            }
            endArray();
            
            // ========================================
            // 🧪 객체 컨텍스트에서 null key 처리 테스트  
            // ========================================
            
            beginObject("object_test");
            {
                // ✅ 정상적인 key 사용
                setString("valid_key", "valid_value");
                
                // ⚠️ null key는 필드 생성되지 않음 (안전하게 무시)
                setString("", "empty_key_value");      // 필드 생성 안됨
                setString(nullptr, "null_key_value");  // 필드 생성 안됨
                
                // ✅ 다시 정상적인 key
                setString("another_key", "another_value");
            }
            endObject();
            
            // ========================================
            // 🧪 중첩 구조에서의 key 처리
            // ========================================
            
            beginArray("nested_test");
            {
                // 배열 내 객체
                beginObject();  // key 없음 (배열 요소)
                {
                    setString("inner_key", "inner_value");
                    setString("", "ignored_in_object");     // 필드 생성 안됨
                    setString(nullptr, "also_ignored");     // 필드 생성 안됨
                }
                endObject();
                
                // 배열 내 배열
                beginArray();  // key 없음 (배열 요소)
                {
                    setString("", "array_item1");          // key 무시됨
                    setString(nullptr, "array_item2");     // key 무시됨
                    setString("whatever", "array_item3");  // key 무시됨
                }
                endArray();
            }
            endArray();
        }
        endObject();
    }
    
    // 🎯 예상 JSON 출력:
    /*
    {
        "test_array": ["test1", "test2", "test3"],    // 모든 key가 무시되고 값만 배열에 추가
        "object_test": {
            "valid_key": "valid_value",                // 정상적인 key-value
            "another_key": "another_value"             // null/empty key는 무시되어 필드 생성 안됨
        },
        "nested_test": [
            {
                "inner_key": "inner_value"             // 객체에서 null key는 무시됨
            },
            ["array_item1", "array_item2", "array_item3"]  // 배열에서 모든 key 무시됨
        ]
    }
    */
};

/**
 * 🛡️ Null Key 처리 안전성 원칙:
 * 
 * 1. ✅ **배열 컨텍스트**:
 *    - 모든 key 값이 무시됨 (null, empty, any string)
 *    - 값만 배열에 순서대로 추가
 *    - key 매개변수는 API 일관성을 위해 존재하지만 사용되지 않음
 * 
 * 2. ✅ **객체 컨텍스트**:
 *    - null이나 empty key는 안전하게 무시
 *    - 필드가 생성되지 않으므로 JSON 구조가 깨지지 않음
 *    - 유효한 key만 객체 필드로 생성
 * 
 * 3. ✅ **메모리 안전성**:
 *    - `key && strlen(key) > 0` 체크로 null pointer 안전성 보장
 *    - strlen 호출 전에 null 체크 수행
 *    - 빈 문자열도 안전하게 처리
 * 
 * 4. ✅ **API 일관성**:
 *    - 모든 상황에서 동일한 setXX(key, value) 시그니처
 *    - 배열 전용 메서드가 따로 필요 없음
 *    - 컨텍스트에 따라 자동으로 적절한 동작 수행
 * 
 * 5. ✅ **개발자 친화적**:
 *    - key를 ""나 nullptr로 써도 안전
 *    - 배열에서는 key 신경쓰지 않아도 됨
 *    - 실수로 잘못된 key를 써도 크래시 없음
 * 
 * 📋 **구현 세부사항**:
 * 
 * ```cpp
 * // JsonableImpl::setString 내부 로직
 * if (contextStack_.back().isArray) {
 *     // 배열: key 완전 무시, 값만 추가
 *     current->PushBack(std::move(valueVal), allocator);
 * } else {
 *     // 객체: 유효한 key만 처리
 *     if (key && strlen(key) > 0) {
 *         // key가 유효할 때만 필드 생성
 *         current->AddMember(keyVal, valueVal, allocator);
 *     }
 *     // 무효한 key는 조용히 무시 (안전)
 * }
 * ```
 * 
 * 이 설계로 인해 사용자는:
 * - 배열에서 어떤 key든 자유롭게 사용 가능
 * - 객체에서 null key 실수해도 안전
 * - 하나의 API로 모든 상황 처리 가능
 */ 