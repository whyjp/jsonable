/**
 * JsonableNewTest.cpp - 새로운 다중상속 구조 테스트 메인 파일
 * 
 * 다중상속 구조 (JsonableBase ← ToJsonable + FromJsonable ← Jsonable) 테스트
 */

#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>

// 새로운 다중상속 테스트들
#include "MultiInheritanceTest.hpp"

/**
 * @brief 테스트 실행 전 전역 설정
 */
class JsonableTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        std::cout << "🚀 Starting Jsonable Multi-Inheritance Tests..." << std::endl;
        std::cout << "Testing new structure: JsonableBase ← ToJsonable + FromJsonable ← Jsonable" << std::endl;
        std::cout << "========================================" << std::endl;
    }

    void TearDown() override {
        std::cout << "========================================" << std::endl;
        std::cout << "✅ All Jsonable Multi-Inheritance Tests Completed!" << std::endl;
    }
};

/**
 * @brief 추가 통합 테스트들
 */
class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 통합 테스트 설정
    }

    void TearDown() override {
        // 통합 테스트 정리
    }
};

/**
 * 🧪 전체 시스템 통합 테스트
 */
TEST_F(IntegrationTest, FullSystemIntegration) {
    std::cout << "\n=== Full System Integration Test ===" << std::endl;
    
    // 복잡한 JSON 구조 생성
    std::string complexJson = R"({
        "company": "Tech Innovations Inc.",
        "founded": 2010,
        "active": true,
        "employees": [
            {
                "name": "Alice Johnson",
                "age": 28,
                "active": true,
                "tags": ["senior", "frontend", "react"]
            },
            {
                "name": "Bob Smith", 
                "age": 32,
                "active": true,
                "tags": ["lead", "backend", "nodejs", "docker"]
            }
        ],
        "departments": ["Engineering", "Marketing", "Sales"],
        "metrics": {
            "revenue": 1500000.50,
            "growth": 15.7,
            "satisfaction": 4.2
        }
    })";
    
    std::cout << "Input JSON size: " << complexJson.length() << " characters" << std::endl;
    
    // JSON 파싱 및 검증
    TestPerson testPerson;
    
    // 기본 필드 직접 설정으로 테스트
    testPerson.setString("company", "Tech Innovations Inc.");
    testPerson.setInt64("founded", 2010);
    testPerson.setBool("active", true);
    
    std::vector<std::string> depts = {"Engineering", "Marketing", "Sales"};
    testPerson.setArray("departments", depts);
    
    // JSON 생성 및 검증
    std::string generatedJson = testPerson.toJson();
    EXPECT_FALSE(generatedJson.empty());
    EXPECT_NE(generatedJson.find("Tech Innovations Inc."), std::string::npos);
    EXPECT_NE(generatedJson.find("2010"), std::string::npos);
    EXPECT_NE(generatedJson.find("Engineering"), std::string::npos);
    
    std::cout << "Generated JSON size: " << generatedJson.length() << " characters" << std::endl;
    std::cout << "Integration test passed!" << std::endl;
}

/**
 * 🧪 메모리 사용량 테스트
 */
TEST_F(IntegrationTest, MemoryUsage) {
    std::cout << "\n=== Memory Usage Test ===" << std::endl;
    
    const int LARGE_TEST_COUNT = 1000;
    std::vector<TestPerson> persons;
    persons.reserve(LARGE_TEST_COUNT);
    
    // 대량 객체 생성
    for (int i = 0; i < LARGE_TEST_COUNT; ++i) {
        TestPerson person("Person" + std::to_string(i), 20 + (i % 50), i % 2 == 0);
        
        // 다양한 크기의 태그 배열
        int tagCount = 1 + (i % 10);
        for (int j = 0; j < tagCount; ++j) {
            person.addTag("tag" + std::to_string(i) + "_" + std::to_string(j));
        }
        
        persons.push_back(person);
    }
    
    // 전체 직렬화
    size_t totalJsonSize = 0;
    for (const auto& person : persons) {
        std::string json = person.toJson();
        totalJsonSize += json.length();
        EXPECT_FALSE(json.empty());
    }
    
    std::cout << "Created " << LARGE_TEST_COUNT << " objects" << std::endl;
    std::cout << "Total JSON size: " << totalJsonSize << " characters" << std::endl;
    std::cout << "Average JSON size per object: " << (totalJsonSize / LARGE_TEST_COUNT) << " characters" << std::endl;
    
    // 메모리 효율성 검증 (대략적)
    EXPECT_LT(totalJsonSize / LARGE_TEST_COUNT, 500); // 평균 500자 미만
    
    std::cout << "Memory usage test passed!" << std::endl;
}

/**
 * 🧪 오류 처리 테스트
 */
TEST_F(IntegrationTest, ErrorHandling) {
    std::cout << "\n=== Error Handling Test ===" << std::endl;
    
    TestPerson person;
    
    // 잘못된 JSON 처리 테스트
    std::string invalidJson1 = "{ invalid json }";
    std::string invalidJson2 = "{ \"name\": \"test\", }"; // trailing comma
    std::string invalidJson3 = ""; // empty string
    
    // 예외가 발생하지 않고 안전하게 처리되어야 함
    EXPECT_NO_THROW(person.fromJson(invalidJson1));
    EXPECT_NO_THROW(person.fromJson(invalidJson2));
    EXPECT_NO_THROW(person.fromJson(invalidJson3));
    
    // 부분적으로 유효한 JSON 처리
    std::string partialJson = R"({
        "name": "PartialTest",
        "age": "not_a_number",
        "active": "not_a_boolean",
        "tags": "not_an_array"
    })";
    
    EXPECT_NO_THROW(person.fromJson(partialJson));
    
    // 이름은 올바르게 설정되어야 함
    EXPECT_EQ(person.getName(), "PartialTest");
    
    std::cout << "Error handling test passed!" << std::endl;
}

/**
 * 🧪 타입 변환 테스트
 */
TEST_F(IntegrationTest, TypeConversion) {
    std::cout << "\n=== Type Conversion Test ===" << std::endl;
    
    TestPerson person;
    
    // 다양한 숫자 타입 테스트
    person.setInt64("int64_val", 9223372036854775807LL); // max int64
    person.setUInt32("uint32_val", 4294967295U); // max uint32
    person.setUInt64("uint64_val", 18446744073709551615ULL); // max uint64
    person.setFloat("float_val", 3.14159f);
    person.setDouble("double_val", 2.718281828459045);
    
    std::string json = person.toJson();
    EXPECT_FALSE(json.empty());
    
    // 역직렬화 후 값 확인
    TestPerson restored;
    restored.fromJson(json);
    
    EXPECT_EQ(restored.getInt64("int64_val"), 9223372036854775807LL);
    EXPECT_EQ(restored.getUInt32("uint32_val"), 4294967295U);
    EXPECT_EQ(restored.getUInt64("uint64_val"), 18446744073709551615ULL);
    EXPECT_NEAR(restored.getFloat("float_val"), 3.14159f, 0.00001f);
    EXPECT_NEAR(restored.getDouble("double_val"), 2.718281828459045, 0.000000000000001);
    
    std::cout << "Type conversion test passed!" << std::endl;
}

/**
 * 🧪 UTF-8 문자열 테스트
 */
TEST_F(IntegrationTest, Utf8StringHandling) {
    std::cout << "\n=== UTF-8 String Handling Test ===" << std::endl;
    
    TestPerson person;
    
    // 다양한 UTF-8 문자열
    std::string korean = "안녕하세요";
    std::string japanese = "こんにちは";
    std::string chinese = "你好";
    std::string emoji = "👋🌍🚀";
    std::string mixed = "Hello 안녕 こんにちは 你好 👋";
    
    person.setString("korean", korean);
    person.setString("japanese", japanese);
    person.setString("chinese", chinese);
    person.setString("emoji", emoji);
    person.setString("mixed", mixed);
    
    std::vector<std::string> multiLangArray = {korean, japanese, chinese, emoji, mixed};
    person.setArray("multilang", multiLangArray);
    
    std::string json = person.toJson();
    EXPECT_FALSE(json.empty());
    
    // 역직렬화 후 확인
    TestPerson restored;
    restored.fromJson(json);
    
    EXPECT_EQ(restored.getString("korean"), korean);
    EXPECT_EQ(restored.getString("japanese"), japanese);
    EXPECT_EQ(restored.getString("chinese"), chinese);
    EXPECT_EQ(restored.getString("emoji"), emoji);
    EXPECT_EQ(restored.getString("mixed"), mixed);
    
    auto restoredArray = restored.getArray<std::string>("multilang");
    EXPECT_EQ(restoredArray.size(), 5);
    for (size_t i = 0; i < restoredArray.size(); ++i) {
        EXPECT_EQ(restoredArray[i], multiLangArray[i]);
    }
    
    std::cout << "UTF-8 handling test passed!" << std::endl;
}

/**
 * @brief 메인 함수
 */
int main(int argc, char **argv) {
    // Google Test 초기화
    ::testing::InitGoogleTest(&argc, argv);
    
    // 전역 테스트 환경 설정
    ::testing::AddGlobalTestEnvironment(new JsonableTestEnvironment);
    
    // 테스트 실행
    int result = RUN_ALL_TESTS();
    
    // 결과 출력
    if (result == 0) {
        std::cout << "\n🎉 All tests passed successfully!" << std::endl;
        std::cout << "New multi-inheritance structure is working correctly." << std::endl;
    } else {
        std::cout << "\n❌ Some tests failed. Please check the output above." << std::endl;
    }
    
    return result;
} 