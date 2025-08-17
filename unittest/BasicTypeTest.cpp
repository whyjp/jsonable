/**
 * BasicTypeTest.cpp - 기본 타입 처리 및 JSON 변환 테스트
 * 
 * 테스트 영역:
 * - 기본 타입 읽기/쓰기 (getString, setString, getInt64, setInt64 등)
 * - 타입 안전성 검증
 * - 기본값 처리
 * - 타입 변환 유연성
 */

#include <gtest/gtest.h>
#include "../JsonableBase.hpp"
#include "../Jsonable.hpp"

using namespace json;

class BasicTypeTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 기본 타입 읽기 테스트
TEST_F(BasicTypeTest, BasicTypeReading) {
    std::string jsonStr = R"({
        "name": "TestName",
        "age": 25,
        "height": 175.5,
        "active": true,
        "uint32Val": 4294967290,
        "uint64Val": 9223372036854775807
    })";
    
    class TestClass : public Jsonable {
    public:
        std::string name;
        int64_t age;
        double height;
        bool active;
        uint32_t uint32Val;
        uint64_t uint64Val;
        
        void loadFromJson() override {
            name = getString("name");
            age = getInt64("age");
            height = getDouble("height");
            active = getBool("active");
            uint32Val = getUInt32("uint32Val");
            uint64Val = getUInt64("uint64Val");
        }
        
        void saveToJson() override {
            setString("name", name);
            setInt64("age", age);
            setDouble("height", height);
            setBool("active", active);
            setUInt32("uint32Val", uint32Val);
            setUInt64("uint64Val", uint64Val);
        }
    };
    
    TestClass obj;
    obj.fromJson(jsonStr);
    
    EXPECT_EQ(obj.name, "TestName");
    EXPECT_EQ(obj.age, 25);
    EXPECT_DOUBLE_EQ(obj.height, 175.5);
    EXPECT_TRUE(obj.active);
    EXPECT_EQ(obj.uint32Val, 4294967290u);
    EXPECT_EQ(obj.uint64Val, 9223372036854775807ull);
}

// 기본값 처리 테스트
TEST_F(BasicTypeTest, DefaultValueHandling) {
    std::string jsonStr = R"({})";
    
    class TestClass : public Jsonable {
    public:
        std::string name;
        int64_t age;
        double height;
        bool active;
        
        void loadFromJson() override {
            name = getString("name", "DefaultName");
            age = getInt64("age", 30);
            height = getDouble("height", 170.0);
            active = getBool("active", false);
        }
        
        void saveToJson() override {
            setString("name", name);
            setInt64("age", age);
            setDouble("height", height);
            setBool("active", active);
        }
    };
    
    TestClass obj;
    obj.fromJson(jsonStr);
    
    EXPECT_EQ(obj.name, "DefaultName");
    EXPECT_EQ(obj.age, 30);
    EXPECT_DOUBLE_EQ(obj.height, 170.0);
    EXPECT_FALSE(obj.active);
}

// 타입 변환 유연성 테스트
TEST_F(BasicTypeTest, TypeConversionFlexibility) {
    std::string jsonStr = R"({
        "intAsFloat": 42,
        "floatAsInt": 3.99,
        "boolAsInt": true,
        "stringAsNumber": "not_a_number"
    })";
    
    class TestClass : public Jsonable {
    public:
        double intAsFloat;
        int64_t floatAsInt;
        int64_t boolAsInt;
        int64_t stringAsNumber;
        
        void loadFromJson() override {
            intAsFloat = getDouble("intAsFloat");
            floatAsInt = getInt64("floatAsInt");
            boolAsInt = getInt64("boolAsInt");
            stringAsNumber = getInt64("stringAsNumber", 999);
        }
        
        void saveToJson() override {
            setDouble("intAsFloat", intAsFloat);
            setInt64("floatAsInt", floatAsInt);
            setInt64("boolAsInt", boolAsInt);
            setInt64("stringAsNumber", stringAsNumber);
        }
    };
    
    TestClass obj;
    obj.fromJson(jsonStr);
    
    EXPECT_DOUBLE_EQ(obj.intAsFloat, 42.0);
    EXPECT_EQ(obj.floatAsInt, 3);
    EXPECT_EQ(obj.boolAsInt, 1);
    EXPECT_EQ(obj.stringAsNumber, 999); // 기본값
}

// 라운드트립 테스트
TEST_F(BasicTypeTest, RoundTripBasicTypes) {
    class TestClass : public Jsonable {
    public:
        std::string name = "TestName";
        int64_t age = 25;
        float height = 175.5f;
        bool active = true;
        
        void loadFromJson() override {
            name = getString("name");
            age = getInt64("age");
            height = getFloat("height");
            active = getBool("active");
        }
        
        void saveToJson() override {
            setString("name", name);
            setInt64("age", age);
            setFloat("height", height);
            setBool("active", active);
        }
        
        bool operator==(const TestClass& other) const {
            return name == other.name && age == other.age && 
                   std::abs(height - other.height) < 0.001f && active == other.active;
        }
    };
    
    TestClass original;
    std::string json = original.toJson();
    
    TestClass restored;
    restored.fromJson(json);
    
    EXPECT_EQ(original, restored);
}
