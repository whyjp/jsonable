#pragma once

/**
 * 배열 컨텍스트 테스트
 * 
 * 배열에서는 key 없이 값만 있어야 하고,
 * 일반적인 setXX 메서드가 컨텍스트에 따라 자동으로 처리되는지 확인
 */

#include "../Jsonable.hpp"

class ArrayContextTestClass : public json::Jsonable {
private:
    std::string title_;
    std::vector<std::string> stringArray_;
    std::vector<int> numberArray_;
    std::vector<bool> boolArray_;
    
    struct Person {
        std::string name;
        int age;
        bool active;
    };
    std::vector<Person> objectArray_;

public:
    ArrayContextTestClass() = default;
    
    void setupTestData() {
        title_ = "Array Context Test";
        stringArray_ = {"apple", "banana", "cherry"};
        numberArray_ = {10, 20, 30, 40};
        boolArray_ = {true, false, true};
        
        objectArray_ = {
            {"Alice", 25, true},
            {"Bob", 30, false},
            {"Charlie", 35, true}
        };
    }

    void loadFromJson() override {
        title_ = getString("title");
        stringArray_ = getArray<std::string>("strings");
        
        // 숫자 배열은 int로 받되 int64_t에서 변환
        auto int64Array = getArray<int64_t>("numbers");
        numberArray_.clear();
        for (auto val : int64Array) {
            numberArray_.push_back(static_cast<int>(val));
        }
        
        boolArray_ = getArray<bool>("booleans");
        
        // 객체 배열은 수동 로딩
        objectArray_.clear();
        if (hasKey("people") && isArray("people")) {
            iterateArray("people", [this](size_t index) {
                Person person;
                // 실제로는 중첩 객체 접근이 필요하지만 여기서는 스킵
                objectArray_.push_back(person);
            });
        }
    }

    void saveToJson() override {
        beginObject();
        {
            setString("title", title_);
            
            // 🎯 문자열 배열 - key 없이 값만 추가되어야 함
            beginArray("strings");
            {
                for (const auto& str : stringArray_) {
                    // ✅ key는 무시되고 배열에 값만 추가
                    setString("", str);           // 빈 문자열 key
                    // setString(nullptr, str);   // 또는 nullptr도 가능해야 함
                }
            }
            endArray();
            
            // 🎯 숫자 배열 - key 없이 값만 추가되어야 함
            beginArray("numbers");
            {
                for (int num : numberArray_) {
                    // ✅ 배열 컨텍스트에서는 key 무시
                    setInt64("ignored_key", static_cast<int64_t>(num));
                }
            }
            endArray();
            
            // 🎯 불린 배열 - key 없이 값만 추가되어야 함
            beginArray("booleans");
            {
                for (bool flag : boolArray_) {
                    // ✅ 어떤 key를 써도 무시됨
                    setBool("whatever", flag);
                }
            }
            endArray();
            
            // 🎯 객체 배열 - 중첩 객체들
            beginArray("people");
            {
                for (const auto& person : objectArray_) {
                    beginObject();  // 배열 내 객체 시작 (key 없음!)
                    {
                        // ✅ 이제는 객체 컨텍스트이므로 key 사용
                        setString("name", person.name);
                        setInt64("age", static_cast<int64_t>(person.age));
                        setBool("active", person.active);
                    }
                    endObject();    // 배열 내 객체 종료
                }
            }
            endArray();
            
            // 🎯 혼합 배열 (다양한 타입) - 하지만 타입 안전성은 유지
            beginArray("mixed_demo");
            {
                setString("", "text_value");    // 문자열
                setInt64("", 42);               // 숫자
                setBool("", true);              // 불린
                
                // 중첩 객체도 가능
                beginObject();  // key 없는 객체
                {
                    setString("nested_key", "nested_value");
                }
                endObject();
                
                // 중첩 배열도 가능
                beginArray();   // key 없는 배열
                {
                    setString("", "inner1");
                    setString("", "inner2");
                }
                endArray();
            }
            endArray();
        }
        endObject();
    }

    // 🎯 예상되는 JSON 출력
    /*
    {
        "title": "Array Context Test",
        "strings": ["apple", "banana", "cherry"],
        "numbers": [10, 20, 30, 40],
        "booleans": [true, false, true],
        "people": [
            {"name": "Alice", "age": 25, "active": true},
            {"name": "Bob", "age": 30, "active": false},
            {"name": "Charlie", "age": 35, "active": true}
        ],
        "mixed_demo": [
            "text_value",
            42,
            true,
            {"nested_key": "nested_value"},
            ["inner1", "inner2"]
        ]
    }
    */

    // Getter들
    const std::string& getTitle() const { return title_; }
    const std::vector<std::string>& getStringArray() const { return stringArray_; }
    const std::vector<int>& getNumberArray() const { return numberArray_; }
    const std::vector<bool>& getBoolArray() const { return boolArray_; }
    const std::vector<Person>& getObjectArray() const { return objectArray_; }
};

/**
 * 🧪 테스트 함수 예제
 */
void testArrayContext() {
    ArrayContextTestClass test;
    test.setupTestData();
    
    // JSON으로 직렬화
    std::string json = test.toJson();
    
    // 결과 확인
    // 1. 배열들이 key 없이 값만 포함하는지 확인
    // 2. 객체는 key-value 쌍을 포함하는지 확인
    // 3. 중첩 구조가 올바른지 확인
    
    std::cout << "Generated JSON:\n" << json << std::endl;
    
    // 역직렬화 테스트
    ArrayContextTestClass test2;
    test2.fromJson(json);
    
    // 원본과 비교
    bool isEqual = (test == test2);
    std::cout << "Round-trip test: " << (isEqual ? "PASS" : "FAIL") << std::endl;
}

/**
 * ✅ 핵심 확인 사항:
 * 
 * 1. **배열 컨텍스트에서 key 무시**:
 *    - setString("", value) ✅
 *    - setString("ignored", value) ✅
 *    - setString(nullptr, value) ✅
 *    모두 동일하게 배열에 값만 추가
 * 
 * 2. **객체 컨텍스트에서 key 사용**:
 *    - setString("name", value) ✅
 *    - key가 필수이고 JSON 객체 필드로 설정
 * 
 * 3. **컨텍스트 자동 전환**:
 *    - beginArray() → 배열 컨텍스트
 *    - beginObject() → 객체 컨텍스트
 *    - endArray() / endObject() → 이전 컨텍스트로 복귀
 * 
 * 4. **일반적인 메서드**:
 *    - setString(), setInt64(), setBool() 등
 *    - 배열 전용 메서드처럼 보이지 않음
 *    - 컨텍스트에 따라 자동으로 적절히 처리
 * 
 * 5. **중첩 구조 지원**:
 *    - 배열 안의 객체
 *    - 배열 안의 배열
 *    - 객체 안의 배열
 *    - 모든 조합이 자연스럽게 처리
 */ 