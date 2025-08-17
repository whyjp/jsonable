#pragma once

/**
 * MultiInheritanceTest.hpp - 다중상속 구조 테스트
 * 
 * 새로운 다중상속 구조 (JsonableBase ← ToJsonable + FromJsonable ← Jsonable)를 테스트
 */

#include "../Jsonable.hpp"

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <iostream>

/**
 * @brief 기본 테스트용 클래스
 */
class TestPerson : public json::Jsonable {
private:
    std::string name_;
    int age_;
    bool active_;
    std::vector<std::string> tags_;

public:
    TestPerson() : age_(0), active_(false) {}
    TestPerson(const std::string& name, int age, bool active = true)
        : name_(name), age_(age), active_(active) {}
    
    // Jsonable의 JSON 문자열 생성자 상속
    using Jsonable::Jsonable;

    // FromJsonable 구현
    void loadFromJson() override {
        name_ = getString("name");
        age_ = static_cast<int>(getInt64("age"));
        active_ = getBool("active", true);
        tags_ = getArray<std::string>("tags");
    }

    // ToJsonable 구현
    void saveToJson() override {
        setString("name", name_);
        setInt64("age", static_cast<int64_t>(age_));
        setBool("active", active_);
        setArray("tags", tags_);
    }

    // 테스트용 접근자들
    void setName(const std::string& name) { name_ = name; }
    void setAge(int age) { age_ = age; }
    void setActive(bool active) { active_ = active; }
    void addTag(const std::string& tag) { tags_.push_back(tag); }
    void clearTags() { tags_.clear(); }

    const std::string& getName() const { return name_; }
    int getAge() const { return age_; }
    bool isActive() const { return active_; }
    const std::vector<std::string>& getTags() const { return tags_; }
};

/**
 * @brief Begin/End 스타일 테스트용 클래스
 */
class TestCompany : public json::Jsonable {
private:
    std::string name_;
    std::vector<TestPerson> employees_;
    std::vector<std::string> departments_;

public:
    TestCompany() = default;
    TestCompany(const std::string& name) : name_(name) {}

    void loadFromJson() override {
        name_ = getString("name");
        departments_ = getArray<std::string>("departments");
        
        // 직원은 단순화 (실제로는 더 복잡한 중첩 객체 처리 필요)
        employees_.clear();
    }

    void saveToJson() override {
        beginObject();
        {
            setString("name", name_);
            
            beginArray("departments");
            {
                for (const auto& dept : departments_) {
                    setString("", dept);
                }
            }
            endArray();
            
            beginArray("employees");
            {
                for (const auto& emp : employees_) {
                    beginObject();
                    {
                        setString("name", emp.getName());
                        setInt64("age", static_cast<int64_t>(emp.getAge()));
                        setBool("active", emp.isActive());
                        
                        beginArray("tags");
                        {
                            for (const auto& tag : emp.getTags()) {
                                setString("", tag);
                            }
                        }
                        endArray();
                    }
                    endObject();
                }
            }
            endArray();
        }
        endObject();
    }

    void setName(const std::string& name) { name_ = name; }
    void addEmployee(const TestPerson& person) { employees_.push_back(person); }
    void addDepartment(const std::string& dept) { departments_.push_back(dept); }

    const std::string& getName() const { return name_; }
    const std::vector<TestPerson>& getEmployees() const { return employees_; }
    const std::vector<std::string>& getDepartments() const { return departments_; }
};

/**
 * @brief 다중상속 구조 테스트 클래스
 */
class MultiInheritanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 테스트 설정
    }

    void TearDown() override {
        // 테스트 정리
    }
};

/**
 * 🧪 기본 다중상속 기능 테스트
 */
TEST_F(MultiInheritanceTest, BasicInheritanceStructure) {
    TestPerson person("Alice", 25, true);
    person.addTag("developer");
    person.addTag("team-lead");

    // ToJsonable 기능 테스트
    std::string json = person.toJson();
    EXPECT_FALSE(json.empty());
    EXPECT_NE(json.find("Alice"), std::string::npos);
    EXPECT_NE(json.find("25"), std::string::npos);
    EXPECT_NE(json.find("developer"), std::string::npos);

    // FromJsonable 기능 테스트
    TestPerson restored;
    restored.fromJson(json);
    
    EXPECT_EQ(restored.getName(), "Alice");
    EXPECT_EQ(restored.getAge(), 25);
    EXPECT_TRUE(restored.isActive());
    EXPECT_EQ(restored.getTags().size(), 2);
    EXPECT_EQ(restored.getTags()[0], "developer");
    EXPECT_EQ(restored.getTags()[1], "team-lead");
}

/**
 * 🧪 JSON 문자열 생성자 테스트
 */
TEST_F(MultiInheritanceTest, JsonStringConstructor) {
    std::string jsonData = R"({
        "name": "Bob",
        "age": 30,
        "active": false,
        "tags": ["manager", "senior"]
    })";

    // 편의 생성자 테스트 (생성 후 명시적 fromJson 호출)
    TestPerson person(jsonData);
    person.fromJson(jsonData);  // 명시적 호출 필요
    
    EXPECT_EQ(person.getName(), "Bob");
    EXPECT_EQ(person.getAge(), 30);
    EXPECT_FALSE(person.isActive());
    EXPECT_EQ(person.getTags().size(), 2);
    EXPECT_EQ(person.getTags()[0], "manager");
    EXPECT_EQ(person.getTags()[1], "senior");
}

/**
 * 🧪 객체 비교 기능 테스트
 */
TEST_F(MultiInheritanceTest, ObjectEquality) {
    TestPerson person1("Charlie", 35, true);
    person1.addTag("architect");
    
    TestPerson person2("Charlie", 35, true);
    person2.addTag("architect");

    // equals 연산자 테스트
    EXPECT_TRUE(person1 == person2);
    
    // 연산자 오버로딩 테스트
    EXPECT_TRUE(person1 == person2);
    EXPECT_FALSE(person1 != person2);

    // 데이터 변경 후 테스트
    person2.setAge(36);
    EXPECT_FALSE(person1 == person2);
    EXPECT_FALSE(person1 == person2);
    EXPECT_TRUE(person1 != person2);
}

/**
 * 🧪 깊은 복사 기능 테스트
 */
TEST_F(MultiInheritanceTest, DeepCopy) {
    TestPerson original("Dave", 28, true);
    original.addTag("fullstack");
    original.addTag("react");

    // 깊은 복사 테스트
    TestPerson copy = original.deepCopy<TestPerson>();
    
    EXPECT_EQ(copy.getName(), original.getName());
    EXPECT_EQ(copy.getAge(), original.getAge());
    EXPECT_EQ(copy.isActive(), original.isActive());
    EXPECT_EQ(copy.getTags(), original.getTags());
    
    // 원본 수정이 복사본에 영향 없음을 확인
    original.setName("David");
    original.addTag("nodejs");
    
    EXPECT_NE(copy.getName(), original.getName());
    EXPECT_NE(copy.getTags().size(), original.getTags().size());
}

/**
 * 🧪 Begin/End 스타일 테스트
 */
TEST_F(MultiInheritanceTest, BeginEndStyle) {
    TestCompany company("Tech Corp");
    company.addDepartment("Engineering");
    company.addDepartment("Marketing");
    
    TestPerson emp1("Alice", 25, true);
    emp1.addTag("frontend");
    emp1.addTag("react");
    
    TestPerson emp2("Bob", 30, true);
    emp2.addTag("backend");
    emp2.addTag("nodejs");
    
    company.addEmployee(emp1);
    company.addEmployee(emp2);

    // JSON 생성 테스트
    std::string json = company.toJson();
    EXPECT_FALSE(json.empty());
    
    // 구조 검증
    EXPECT_NE(json.find("Tech Corp"), std::string::npos);
    EXPECT_NE(json.find("Engineering"), std::string::npos);
    EXPECT_NE(json.find("Marketing"), std::string::npos);
    EXPECT_NE(json.find("Alice"), std::string::npos);
    EXPECT_NE(json.find("Bob"), std::string::npos);
    EXPECT_NE(json.find("frontend"), std::string::npos);
    EXPECT_NE(json.find("backend"), std::string::npos);

    std::cout << "Generated Company JSON:" << std::endl;
    std::cout << json << std::endl;
}

/**
 * 🧪 타입 안전성 테스트
 */
TEST_F(MultiInheritanceTest, TypeSafety) {
    TestPerson person;
    
    // TestPerson의 실제 필드들로 타입 안전성 테스트
    person.setName("TypeSafetyTest");
    person.setAge(30);
    person.setActive(true);
    person.addTag("safety");
    person.addTag("test");
    
    std::string json = person.toJson();
    EXPECT_FALSE(json.empty());
    
    // 역직렬화 테스트
    TestPerson restored;
    restored.fromJson(json);
    
    // 실제 TestPerson 필드들 검증
    EXPECT_EQ(restored.getName(), "TypeSafetyTest");
    EXPECT_EQ(restored.getAge(), 30);
    EXPECT_TRUE(restored.isActive());
    
    const auto& tags = restored.getTags();
    EXPECT_EQ(tags.size(), 2);
    EXPECT_EQ(tags[0], "safety");
    EXPECT_EQ(tags[1], "test");
    
    // 타입 안전성 검증: 원본과 복원된 객체가 동일해야 함
    EXPECT_EQ(person, restored);
    
    // 추가 JSON 라운드트립으로 타입 안전성 확인
    std::string json2 = restored.toJson();
    EXPECT_EQ(json, json2);  // 동일한 JSON이 생성되어야 함
}

/**
 * 🧪 배열 컨텍스트 key 무시 테스트
 */
TEST_F(MultiInheritanceTest, ArrayContextKeyIgnoring) {
    TestPerson person;
    
    person.beginObject();
    {
        person.setString("name", "ArrayTest");
        
        person.beginArray("items");
        {
            // 배열에서는 key가 모두 무시되어야 함
            person.setString("", "item1");           // 빈 문자열
            person.setString("ignored", "item2");    // 임의 문자열
            person.setString("whatever", "item3");   // 다른 임의 문자열
        }
        person.endArray();
    }
    person.endObject();
    
    std::string json = person.toJson();
    std::cout << "Array context test JSON:" << std::endl;
    std::cout << json << std::endl;
    
    // JSON에 배열이 올바르게 생성되었는지 확인
    EXPECT_NE(json.find("[\"item1\",\"item2\",\"item3\"]"), std::string::npos);
    
    // key들이 JSON에 포함되지 않았는지 확인
    EXPECT_EQ(json.find("ignored"), std::string::npos);
    EXPECT_EQ(json.find("whatever"), std::string::npos);
}

/**
 * 🧪 null key 안전성 테스트
 */
TEST_F(MultiInheritanceTest, NullKeySafety) {
    TestPerson person;
    
    person.beginObject();
    {
        person.setString("valid", "valid_value");
        
        // null key와 빈 key는 안전하게 무시되어야 함
        person.setString("", "empty_key_value");      // 빈 문자열 (무시됨)
        person.setString(nullptr, "null_key_value");  // nullptr (무시됨)
        
        person.setString("another", "another_value");
        
        person.beginArray("test_array");
        {
            // 배열에서는 모든 key가 무시됨 (안전)
            person.setString("", "array_item1");
            person.setString(nullptr, "array_item2");
            person.setString("ignored", "array_item3");
        }
        person.endArray();
    }
    person.endObject();
    
    std::string json = person.toJson();
    std::cout << "Null key safety test JSON:" << std::endl;
    std::cout << json << std::endl;
    
    // 유효한 필드들은 포함되어야 함
    EXPECT_NE(json.find("valid"), std::string::npos);
    EXPECT_NE(json.find("valid_value"), std::string::npos);
    EXPECT_NE(json.find("another"), std::string::npos);
    EXPECT_NE(json.find("another_value"), std::string::npos);
    
    // 배열 요소들은 포함되어야 함
    EXPECT_NE(json.find("array_item1"), std::string::npos);
    EXPECT_NE(json.find("array_item2"), std::string::npos);
    EXPECT_NE(json.find("array_item3"), std::string::npos);
    
    // null/empty key로 인한 필드들은 포함되지 않아야 함
    EXPECT_EQ(json.find("empty_key_value"), std::string::npos);
    EXPECT_EQ(json.find("null_key_value"), std::string::npos);
}

/**
 * 🧪 편의 메서드 테스트
 */
TEST_F(MultiInheritanceTest, ConvenienceMethods) {
    TestPerson person("ConvenienceTest", 42, true);
    person.addTag("test");
    
    // toJson 메서드 테스트
    std::string jsonStr1 = person.toJson();
    std::string jsonStr2 = person.toJson();
    EXPECT_EQ(jsonStr1, jsonStr2);
    
    // fromJson 메서드 테스트
    TestPerson restored;
    restored.fromJson(jsonStr1);
    
    EXPECT_EQ(restored.getName(), "ConvenienceTest");
    EXPECT_EQ(restored.getAge(), 42);
    EXPECT_TRUE(restored.isActive());
    EXPECT_EQ(restored.getTags().size(), 1);
    EXPECT_EQ(restored.getTags()[0], "test");
}

/**
 * 🧪 성능 및 안정성 테스트
 */
TEST_F(MultiInheritanceTest, PerformanceAndStability) {
    const int TEST_COUNT = 100;
    
    std::vector<TestPerson> persons;
    persons.reserve(TEST_COUNT);
    
    // 대량 객체 생성 및 직렬화
    for (int i = 0; i < TEST_COUNT; ++i) {
        TestPerson person("Person" + std::to_string(i), 20 + i, i % 2 == 0);
        person.addTag("tag" + std::to_string(i));
        person.addTag("category" + std::to_string(i % 5));
        persons.push_back(person);
    }
    
    // 모든 객체 직렬화
    std::vector<std::string> jsonStrings;
    jsonStrings.reserve(TEST_COUNT);
    
    for (const auto& person : persons) {
        std::string json = person.toJson();
        EXPECT_FALSE(json.empty());
        jsonStrings.push_back(json);
    }
    
    // 모든 객체 역직렬화
    std::vector<TestPerson> restored;
    restored.reserve(TEST_COUNT);
    
    for (const auto& json : jsonStrings) {
        TestPerson person;
        person.fromJson(json);
        restored.push_back(person);
    }
    
    // 데이터 무결성 확인
    EXPECT_EQ(restored.size(), TEST_COUNT);
    
    for (int i = 0; i < TEST_COUNT; ++i) {
        EXPECT_EQ(persons[i].getName(), restored[i].getName());
        EXPECT_EQ(persons[i].getAge(), restored[i].getAge());
        EXPECT_EQ(persons[i].isActive(), restored[i].isActive());
        EXPECT_EQ(persons[i].getTags(), restored[i].getTags());
    }
    
    std::cout << "Performance test completed: " << TEST_COUNT 
              << " objects serialized and deserialized successfully." << std::endl;
} 