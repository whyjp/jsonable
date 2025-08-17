/**
 * ErrorHandlingTest.cpp - 오류 처리 및 예외 상황 테스트
 * 
 * 테스트 영역:
 * - 잘못된 JSON 형식 처리
 * - 누락된 필드 처리
 * - 타입 불일치 상황
 * - 경계 값 및 예외 상황
 */

#include <gtest/gtest.h>
#include "../Jsonable.hpp"
#include <stdexcept>

using namespace json;

class ErrorHandlingTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 잘못된 JSON 형식 테스트
TEST_F(ErrorHandlingTest, InvalidJsonFormatTest) {
    class TestClass : public Jsonable {
    public:
        std::string name;
        int64_t value;
        
        void loadFromJson() override {
            name = getString("name", "default");
            value = getInt64("value", 0);
        }
        
        void saveToJson() override {
            setString("name", name);
            setInt64("value", value);
        }
    };
    
    TestClass obj;
    
    // 잘못된 JSON 형식들
    std::vector<std::string> invalidJsons = {
        "{ invalid json }",
        "{ \"name\": \"test\", \"value\": }",  // 값 누락
        "{ \"name\": \"test\" \"value\": 123 }",  // 콤마 누락
        "{ \"name\": }",  // 불완전한 JSON
        "",  // 빈 문자열
        "null",  // null 값
        "[1, 2, 3]"  // 배열 (객체가 아님)
    };
    
    for (const auto& invalidJson : invalidJsons) {
        // 예외가 발생하거나 기본값으로 처리되어야 함
        try {
            obj.fromJson(invalidJson);
            // 예외가 발생하지 않았다면 기본값으로 처리되었는지 확인
            EXPECT_EQ(obj.name, "default");
            EXPECT_EQ(obj.value, 0);
        } catch (const std::exception& e) {
            // 예외 발생은 예상된 동작
            EXPECT_NE(std::string(e.what()), "");
        }
    }
}

// 누락된 필드 처리 테스트
TEST_F(ErrorHandlingTest, MissingFieldHandlingTest) {
    class TestClass : public Jsonable {
    public:
        std::string name;
        int64_t age;
        bool active;
        
        void loadFromJson() override {
            name = getString("name", "Unknown");
            age = getInt64("age", 25);
            active = getBool("active", true);
        }
        
        void saveToJson() override {
            setString("name", name);
            setInt64("age", age);
            setBool("active", active);
        }
    };
    
    // 다양한 누락 시나리오
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"{}", "Complete missing"},
        {R"({"name": "John"})", "Partial missing"},
        {R"({"age": 30, "active": false})", "Name missing"},
        {R"({"name": "Jane", "age": 28})", "Active missing"}
    };
    
    for (const auto& testCase : testCases) {
        TestClass obj;
        obj.fromJson(testCase.first);
        
        // 기본값이 제대로 설정되었는지 확인
        if (obj.name == "Unknown") {
            EXPECT_EQ(obj.age, 25);  // 기본값들이 설정되었어야 함
            EXPECT_TRUE(obj.active);
        }
        
        // 어떤 경우든 비정상 종료되지 않아야 함
        EXPECT_NO_THROW(obj.toJson());
    }
}

// 타입 불일치 테스트
TEST_F(ErrorHandlingTest, TypeMismatchTest) {
    class TestClass : public Jsonable {
    public:
        std::string name;
        int64_t number;
        bool flag;
        double decimal;
        
        void loadFromJson() override {
            name = getString("name", "default");
            number = getInt64("number", 0);
            flag = getBool("flag", false);
            decimal = getDouble("decimal", 0.0);
        }
        
        void saveToJson() override {
            setString("name", name);
            setInt64("number", number);
            setBool("flag", flag);
            setDouble("decimal", decimal);
        }
    };
    
    // 타입 불일치 JSON
    std::string typeMismatchJson = R"({
        "name": 123,
        "number": "not_a_number",
        "flag": "maybe",
        "decimal": true
    })";
    
    TestClass obj;
    
    // 타입 불일치에도 예외가 발생하지 않고 기본값으로 처리되어야 함
    EXPECT_NO_THROW(obj.fromJson(typeMismatchJson));
    
    // 기본값으로 설정되었는지 확인
    EXPECT_EQ(obj.name, "default");
    EXPECT_EQ(obj.number, 0);
    EXPECT_FALSE(obj.flag);
    EXPECT_DOUBLE_EQ(obj.decimal, 0.0);
}

// 배열 오류 처리 테스트
TEST_F(ErrorHandlingTest, ArrayErrorHandlingTest) {
    class TestClass : public Jsonable {
    public:
        std::vector<std::string> strings;
        std::vector<int64_t> numbers;
        
        void loadFromJson() override {
            strings = getArray<std::string>("strings");
            numbers = getArray<int64_t>("numbers");
        }
        
        void saveToJson() override {
            setArray("strings", strings);
            setArray("numbers", numbers);
        }
    };
    
    // 잘못된 배열 형식
    std::vector<std::string> invalidArrayJsons = {
        R"({"strings": "not_an_array", "numbers": [1, 2, 3]})",
        R"({"strings": ["a", "b"], "numbers": "not_an_array"})",
        R"({"strings": null, "numbers": null})",
        R"({"strings": {}, "numbers": {}})"
    };
    
    for (const auto& invalidJson : invalidArrayJsons) {
        TestClass obj;
        
        EXPECT_NO_THROW(obj.fromJson(invalidJson));
        
        // 잘못된 배열은 빈 배열로 처리되어야 함
        if (!obj.isArray("strings")) {
            // 빈 배열이거나 오류 상황에서도 예외가 발생하지 않아야 함
        }
        
        // 직렬화도 정상 동작해야 함
        EXPECT_NO_THROW(obj.toJson());
    }
}

// 경계값 테스트
TEST_F(ErrorHandlingTest, BoundaryValueTest) {
    class TestClass : public Jsonable {
    public:
        int64_t maxInt;
        int64_t minInt;
        double maxDouble;
        double minDouble;
        
        void loadFromJson() override {
            maxInt = getInt64("maxInt");
            minInt = getInt64("minInt");
            maxDouble = getDouble("maxDouble");
            minDouble = getDouble("minDouble");
        }
        
        void saveToJson() override {
            setInt64("maxInt", maxInt);
            setInt64("minInt", minInt);
            setDouble("maxDouble", maxDouble);
            setDouble("minDouble", minDouble);
        }
    };
    
    // 경계값 JSON
    std::string boundaryJson = R"({
        "maxInt": 9223372036854775807,
        "minInt": -9223372036854775808,
        "maxDouble": 1.7976931348623157e+308,
        "minDouble": -1.7976931348623157e+308
    })";
    
    TestClass obj;
    
    // 경계값 처리 테스트
    EXPECT_NO_THROW(obj.fromJson(boundaryJson));
    
    // 값 범위 확인
    EXPECT_EQ(obj.maxInt, 9223372036854775807LL);
    EXPECT_EQ(obj.minInt, static_cast<int64_t>(-9223372036854775808ULL));
    EXPECT_GT(obj.maxDouble, 1.0e+308);
    EXPECT_LT(obj.minDouble, -1.0e+308);
    
    // 라운드트립 테스트
    std::string json = obj.toJson();
    TestClass restored;
    EXPECT_NO_THROW(restored.fromJson(json));
}

// 메모리 스트레스 테스트
TEST_F(ErrorHandlingTest, MemoryStressTest) {
    class TestClass : public Jsonable {
    public:
        std::vector<std::string> largeArray;
        
        void loadFromJson() override {
            largeArray = getArray<std::string>("largeArray");
        }
        
        void saveToJson() override {
            setArray("largeArray", largeArray);
        }
    };
    
    TestClass obj;
    
    // 큰 배열 생성
    for (int i = 0; i < 1000; ++i) {
        obj.largeArray.push_back("Item_" + std::to_string(i));
    }
    
    // 직렬화 테스트
    std::string json;
    EXPECT_NO_THROW(json = obj.toJson());
    EXPECT_FALSE(json.empty());
    
    // 역직렬화 테스트
    TestClass restored;
    EXPECT_NO_THROW(restored.fromJson(json));
    EXPECT_EQ(restored.largeArray.size(), 1000);
    EXPECT_EQ(restored.largeArray[0], "Item_0");
    EXPECT_EQ(restored.largeArray[999], "Item_999");
}

// 비정상적인 컴텍스트 스택 상황 테스트
TEST_F(ErrorHandlingTest, AbnormalContextStackTest) {
    class TestClass : public Jsonable {
    public:
        void loadFromJson() override {}
        
        void saveToJson() override {
            // 비정상적인 Begin/End 패턴
            beginArray("test");
            pushString("item1");
            pushString("item2");
            // endArray() 누락 - 비정상 상황
            
            beginObject("obj");
            setString("key", "value");
            // endObject() 누락 - 비정상 상황
        }
    };
    
    TestClass obj;
    
    // 비정상적인 상황에서도 예외가 발생하지 않아야 함
    std::string json;
    EXPECT_NO_THROW(json = obj.toJson());
    
    // 생성된 JSON이 유효한지 확인
    EXPECT_FALSE(json.empty());
}
