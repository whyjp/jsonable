/**
 * InheritanceTest.cpp - 다중상속 구조 및 상속 체인 테스트
 * 
 * 테스트 영역:
 * - 다이아몬드 다중상속 구조 검증
 * - Virtual 상속 동작 확인
 * - ToJsonable, FromJsonable 각각의 인터페이스
 * - Jsonable 통합 인터페이스
 */

#include <gtest/gtest.h>
#include "../Jsonable.hpp"
#include "../ToJsonable.hpp"
#include "../FromJsonable.hpp"
#include "../JsonableBase.hpp"

using namespace json;

class InheritanceTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ToJsonable 전용 클래스 테스트
TEST_F(InheritanceTest, ToJsonableOnlyTest) {
    class WriteOnlyClass : public ToJsonable {
    public:
        std::string name = "WriteOnly";
        int64_t value = 42;
        
        void saveToJson() override {
            setString("name", name);
            setInt64("value", value);
        }
    };
    
    WriteOnlyClass obj;
    std::string json = obj.toJson();
    
    EXPECT_FALSE(json.empty());
    EXPECT_TRUE(json.find("WriteOnly") != std::string::npos);
    EXPECT_TRUE(json.find("42") != std::string::npos);
}

// FromJsonable 전용 클래스 테스트
TEST_F(InheritanceTest, FromJsonableOnlyTest) {
    class ReadOnlyClass : public FromJsonable {
    public:
        std::string name;
        int64_t value;
        
        void loadFromJson() override {
            name = getString("name");
            value = getInt64("value");
        }
    };
    
    std::string jsonStr = R"({
        "name": "ReadOnly",
        "value": 123
    })";
    
    ReadOnlyClass obj;
    obj.fromJson(jsonStr);
    
    EXPECT_EQ(obj.name, "ReadOnly");
    EXPECT_EQ(obj.value, 123);
}

// Jsonable 통합 인터페이스 테스트
TEST_F(InheritanceTest, FullJsonableTest) {
    class FullClass : public Jsonable {
    public:
        std::string name = "Full";
        int64_t value = 999;
        
        void loadFromJson() override {
            name = getString("name");
            value = getInt64("value");
        }
        
        void saveToJson() override {
            setString("name", name);
            setInt64("value", value);
        }
        
        bool operator==(const FullClass& other) const {
            return name == other.name && value == other.value;
        }
    };
    
    FullClass original;
    std::string json = original.toJson();
    
    FullClass restored;
    restored.fromJson(json);
    
    EXPECT_EQ(original, restored);
}

// Virtual 상속 검증 테스트
TEST_F(InheritanceTest, VirtualInheritanceVerification) {
    // JsonableBase의 인스턴스가 하나만 존재하는지 확인
    class TestClass : public Jsonable {
    public:
        void loadFromJson() override {}
        void saveToJson() override {
            setString("test", "value");
        }
    };
    
    TestClass obj;
    
    // 다이아몬드 문제가 없다면 성공적으로 컴파일되고 실행됨
    std::string json = obj.toJson();
    EXPECT_TRUE(json.find("test") != std::string::npos);
    
    // ToJsonable과 FromJsonable로의 캐스팅이 가능해야 함
    ToJsonable* toPtr = &obj;
    FromJsonable* fromPtr = &obj;
    
    EXPECT_NE(toPtr, nullptr);
    EXPECT_NE(fromPtr, nullptr);
}

// 다중 인터페이스 동시 사용 테스트
TEST_F(InheritanceTest, MultipleInterfaceUsageTest) {
    class DualUseClass : public Jsonable {
    public:
        std::string mode;
        int64_t count = 0;
        
        void loadFromJson() override {
            mode = getString("mode", "default");
            count = getInt64("count", 0);
        }
        
        void saveToJson() override {
            setString("mode", mode);
            setInt64("count", count);
        }
    };
    
    DualUseClass obj;
    
    // FromJsonable 인터페이스 사용
    std::string inputJson = R"({"mode": "test", "count": 5})";
    static_cast<FromJsonable&>(obj).fromJson(inputJson);
    
    EXPECT_EQ(obj.mode, "test");
    EXPECT_EQ(obj.count, 5);
    
    // ToJsonable 인터페이스 사용
    obj.mode = "output";
    obj.count = 10;
    std::string outputJson = static_cast<ToJsonable&>(obj).toJson();
    
    EXPECT_TRUE(outputJson.find("output") != std::string::npos);
    EXPECT_TRUE(outputJson.find("10") != std::string::npos);
}

// 타입 트레이트 검증 테스트
TEST_F(InheritanceTest, TypeTraitVerificationTest) {
    class TestJsonable : public Jsonable {
    public:
        void loadFromJson() override {}
        void saveToJson() override {}
    };
    
    class NonJsonable {
    public:
        int value = 0;
    };
    
    // is_jsonable 타입 트레이트 테스트
    EXPECT_TRUE(is_jsonable_v<TestJsonable>);
    EXPECT_FALSE(is_jsonable_v<NonJsonable>);
    EXPECT_FALSE(is_jsonable_v<int>);
    EXPECT_FALSE(is_jsonable_v<std::string>);
}

// 깊은 복사 테스트
TEST_F(InheritanceTest, DeepCopyTest) {
    class CopyableClass : public Jsonable {
    public:
        std::string data = "original";
        int64_t version = 1;
        
        void loadFromJson() override {
            data = getString("data");
            version = getInt64("version");
        }
        
        void saveToJson() override {
            setString("data", data);
            setInt64("version", version);
        }
        
        bool operator==(const CopyableClass& other) const {
            return data == other.data && version == other.version;
        }
    };
    
    CopyableClass original;
    original.data = "test_data";
    original.version = 5;
    
    // 깊은 복사 수행
    auto copy = original.deepCopy<CopyableClass>();
    
    // 동등성 검증
    EXPECT_EQ(original, copy);
    
    // 독립성 검증
    copy.data = "modified";
    EXPECT_NE(original.data, copy.data);
}

// 연산자 오버로딩 테스트
TEST_F(InheritanceTest, OperatorOverloadingTest) {
    class ComparableClass : public Jsonable {
    public:
        std::string id = "test";
        int64_t value = 100;
        
        void loadFromJson() override {
            id = getString("id");
            value = getInt64("value");
        }
        
        void saveToJson() override {
            setString("id", id);
            setInt64("value", value);
        }
    };
    
    ComparableClass obj1, obj2;
    obj1.id = "same";
    obj1.value = 123;
    
    obj2.id = "same";
    obj2.value = 123;
    
    // 등호 연산자 테스트
    EXPECT_TRUE(obj1 == obj2);
    EXPECT_FALSE(obj1 != obj2);
    
    // 값 변경 후 비교
    obj2.value = 456;
    EXPECT_FALSE(obj1 == obj2);
    EXPECT_TRUE(obj1 != obj2);
}
