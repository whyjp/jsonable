#include <gtest/gtest.h>
#include "TestJsonable.hpp"
#include <map>

class JsonableTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 테스트 전 설정
    }

    void TearDown() override {
        // 테스트 후 정리
    }
};

// 기본 Person JSON 직렬화 테스트
TEST_F(JsonableTest, PersonToJsonBasic) {
    test::Person person("홍길동", 30, true);
    person.addHobby("독서");
    person.addHobby("영화");

    std::string json = person.toJson();
    
    // JSON 문자열에 필요한 내용이 포함되어 있는지 확인
    EXPECT_FALSE(json.empty());
    EXPECT_TRUE(json.find("홍길동") != std::string::npos);
    EXPECT_TRUE(json.find("30") != std::string::npos);
    EXPECT_TRUE(json.find("true") != std::string::npos);
    EXPECT_TRUE(json.find("독서") != std::string::npos);
    EXPECT_TRUE(json.find("영화") != std::string::npos);
}

// JSON에서 Person 객체로 역직렬화 테스트
TEST_F(JsonableTest, PersonFromJsonBasic) {
    std::string jsonStr = R"({
        "name": "이순신",
        "age": 45,
        "isStudent": false,
        "hobbies": ["활쏘기", "전술연구"]
    })";

    test::Person person;
    person.fromJson(jsonStr);

    EXPECT_EQ(person.getName(), "이순신");
    EXPECT_EQ(person.getAge(), 45);
    EXPECT_FALSE(person.getIsStudent());
    
    const auto& hobbies = person.getHobbies();
    EXPECT_EQ(hobbies.size(), 2);
    EXPECT_EQ(hobbies[0], "활쏘기");
    EXPECT_EQ(hobbies[1], "전술연구");

}

// 라운드트립 테스트 (JSON -> 객체 -> JSON)
TEST_F(JsonableTest, PersonRoundTrip) {
    // 원본 객체 생성
    test::Person original("박민수", 28, true);
    original.addHobby("게임");
    original.addHobby("음악");

    // 객체 -> JSON -> 새 객체
    std::string json = original.toJson();
    test::Person restored;
    restored.fromJson(json);

    // 원본과 복원된 객체가 같은지 확인
    EXPECT_EQ(original, restored);
}

// Car 클래스 기본 테스트
TEST_F(JsonableTest, CarBasicConversion) {
    test::Car car("현대", "소나타", 2023, 35000.99);
    
    // 객체 -> JSON
    std::string json = car.toJson();
    EXPECT_FALSE(json.empty());
    EXPECT_TRUE(json.find("\"현대\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"소나타\"") != std::string::npos);
    EXPECT_TRUE(json.find("2023") != std::string::npos);
    EXPECT_TRUE(json.find("35000.99") != std::string::npos);

    // JSON -> 새 객체
    test::Car restored;
    restored.fromJson(json);
    
    EXPECT_EQ(car, restored);
}

// 빈 객체 테스트
TEST_F(JsonableTest, EmptyObjects) {
    test::Person emptyPerson;
    std::string json = emptyPerson.toJson();
    
    test::Person restored;
    restored.fromJson(json);
    
    EXPECT_EQ(emptyPerson, restored);
}

// 부분적인 JSON 데이터 테스트 (필드 누락)
TEST_F(JsonableTest, PartialJsonData) {
    std::string partialJson = R"({
        "name": "홍길동",
        "age": 35
    })";

    test::Person person;
    person.fromJson(partialJson);

    EXPECT_EQ(person.getName(), "홍길동");
    EXPECT_EQ(person.getAge(), 35);
    EXPECT_FALSE(person.getIsStudent()); // 기본값
    EXPECT_TRUE(person.getHobbies().empty()); // 빈 배열
}

// 잘못된 JSON 형식 테스트
TEST_F(JsonableTest, InvalidJsonFormat) {
    std::string invalidJson = "{ invalid json }";
    
    test::Person person;
    EXPECT_THROW(person.fromJson(invalidJson), std::runtime_error);
}

// 잘못된 JSON 타입 테스트
TEST_F(JsonableTest, InvalidJsonTypes) {
    std::string wrongTypeJson = R"({
        "name": 123,
        "age": "not_a_number",
        "isStudent": "maybe"
    })";

    test::Person person;
    
    // 예외가 발생하지 않더라도 기본값이나 변환된 값을 확인
    EXPECT_NO_THROW(person.fromJson(wrongTypeJson));
}

// getString 헬퍼 메서드 테스트
TEST_F(JsonableTest, HelperMethodGetString) {
    std::string jsonStr = R"({"text": "hello", "number": 42})";
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    EXPECT_EQ(json::Jsonable::getString(doc, "text"), "hello");
    EXPECT_EQ(json::Jsonable::getString(doc, "missing", "default"), "default");
}

// getInt64 헬퍼 메서드 테스트
TEST_F(JsonableTest, HelperMethodGetInt64) {
    std::string jsonStr = R"({"number": 12345, "text": "hello"})";
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    EXPECT_EQ(json::Jsonable::getInt64(doc, "number"), 12345);
    EXPECT_EQ(json::Jsonable::getInt64(doc, "missing", 999), 999);
}

// getBool 헬퍼 메서드 테스트
TEST_F(JsonableTest, HelperMethodGetBool) {
    std::string jsonStr = R"({"flag": true, "number": 42})";
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    EXPECT_TRUE(json::Jsonable::getBool(doc, "flag"));
    EXPECT_FALSE(json::Jsonable::getBool(doc, "missing", false));
}

// isArray 헬퍼 메서드 테스트
TEST_F(JsonableTest, HelperMethodIsArray) {
    std::string jsonStr = R"({"array": [1, 2, 3], "object": {}})";
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    EXPECT_TRUE(json::Jsonable::isArray(doc, "array"));
    EXPECT_FALSE(json::Jsonable::isArray(doc, "object"));
    EXPECT_FALSE(json::Jsonable::isArray(doc, "missing"));
}

// isObject 헬퍼 메서드 테스트
TEST_F(JsonableTest, HelperMethodIsObject) {
    std::string jsonStr = R"({"object": {}, "array": [1, 2, 3]})";
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    EXPECT_TRUE(json::Jsonable::isObject(doc, "object"));
    EXPECT_FALSE(json::Jsonable::isObject(doc, "array"));
    EXPECT_FALSE(json::Jsonable::isObject(doc, "missing"));
}

// 복잡한 중첩된 JSON 테스트
TEST_F(JsonableTest, ComplexNestedJson) {
    std::string complexJson = R"({
        "name": "김철수",
        "age": 35,
        "isStudent": false,
        "hobbies": ["축구", "요리", "프로그래밍"]
    })";

    test::Person person;
    person.fromJson(complexJson);

    EXPECT_EQ(person.getName(), "김철수");
    EXPECT_EQ(person.getAge(), 35);
    EXPECT_FALSE(person.getIsStudent());
    
    const auto& hobbies = person.getHobbies();
    EXPECT_EQ(hobbies.size(), 3);
    EXPECT_EQ(hobbies[0], "축구");
    EXPECT_EQ(hobbies[1], "요리");
    EXPECT_EQ(hobbies[2], "프로그래밍");
}

// 특수 문자 처리 테스트
TEST_F(JsonableTest, SpecialCharacters) {
    test::Person person("김\"철수\"", 25, true);
    person.addHobby("읽기 & 쓰기");
    
    std::string json = person.toJson();
    
    test::Person restored;
    restored.fromJson(json);
    
    EXPECT_EQ(restored.getName(), "김\"철수\"");
    EXPECT_EQ(restored.getHobbies()[0], "읽기 & 쓰기");
}

// 큰 숫자 처리 테스트
TEST_F(JsonableTest, LargeNumbers) {
    test::Car car("Tesla", "Model S", 2023, 99999.99);
    
    std::string json = car.toJson();
    test::Car restored;
    restored.fromJson(json);
    
    EXPECT_EQ(restored.getPrice(), 99999.99);
}

// 빈 배열 처리 테스트
TEST_F(JsonableTest, EmptyArray) {
    test::Person person("홍길동", 30, false);
    // hobbies는 빈 상태
    
    std::string json = person.toJson();
    test::Person restored;
    restored.fromJson(json);
    
    EXPECT_TRUE(restored.getHobbies().empty());
}

// 성능 테스트
TEST_F(JsonableTest, PerformanceTest) {
    const int iterations = 1000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        test::Person person("TestPerson" + std::to_string(i), 25 + i % 50, i % 2 == 0);
        person.addHobby("hobby1");
        person.addHobby("hobby2");
        
        std::string json = person.toJson();
        
        test::Person restored;
        restored.fromJson(json);
        
        EXPECT_EQ(person, restored);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 1000번 반복이 1초 이내에 완료되어야 함
    EXPECT_LT(duration.count(), 1000);
}

// ========================================
// 확장된 기능 테스트들
// ========================================

// Float/Double 추출 테스트
TEST_F(JsonableTest, FloatDoubleExtraction) {
    std::string jsonStr = R"({
        "floatVal": 3.14,
        "doubleVal": 2.718281828,
        "intAsFloat": 42,
        "stringAsNumber": "not_a_number"
    })";
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    EXPECT_FLOAT_EQ(json::Jsonable::getFloat(doc, "floatVal"), 3.14f);
    EXPECT_DOUBLE_EQ(json::Jsonable::getDouble(doc, "doubleVal"), 2.718281828);
    EXPECT_FLOAT_EQ(json::Jsonable::getFloat(doc, "intAsFloat"), 42.0f);
    EXPECT_FLOAT_EQ(json::Jsonable::getFloat(doc, "missing", 1.23f), 1.23f);
}

// Unsigned Integer 추출 테스트
TEST_F(JsonableTest, UnsignedIntegerExtraction) {
    std::string jsonStr = R"({
        "uint32Val": 4294967290,
        "uint64Val": 9223372036854775807,
        "negativeVal": -1,
        "floatVal": 3.14
    })";
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    EXPECT_EQ(json::Jsonable::getUInt32(doc, "uint32Val"), 4294967290u);
    EXPECT_EQ(json::Jsonable::getUInt64(doc, "uint64Val"), 9223372036854775807ull);
    EXPECT_EQ(json::Jsonable::getUInt32(doc, "negativeVal"), 0u); // 음수는 0으로 처리
    EXPECT_EQ(json::Jsonable::getUInt32(doc, "missing", 999u), 999u);
}

// Optional 추출 테스트
TEST_F(JsonableTest, OptionalExtraction) {
    std::string jsonStr = R"({
        "existingString": "hello",
        "existingInt": 42,
        "existingFloat": 3.14,
        "existingBool": true
    })";
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    // 존재하는 값들
    auto optStr = json::Jsonable::getOptionalString(doc, "existingString");
    EXPECT_TRUE(optStr.has_value());
    EXPECT_EQ(optStr.value(), "hello");
    
    auto optInt = json::Jsonable::getOptionalInt64(doc, "existingInt");
    EXPECT_TRUE(optInt.has_value());
    EXPECT_EQ(optInt.value(), 42);
    
    auto optFloat = json::Jsonable::getOptionalFloat(doc, "existingFloat");
    EXPECT_TRUE(optFloat.has_value());
    EXPECT_FLOAT_EQ(optFloat.value(), 3.14f);
    
    auto optBool = json::Jsonable::getOptionalBool(doc, "existingBool");
    EXPECT_TRUE(optBool.has_value());
    EXPECT_TRUE(optBool.value());
    
    // 존재하지 않는 값들
    auto missingStr = json::Jsonable::getOptionalString(doc, "missing");
    EXPECT_FALSE(missingStr.has_value());
    
    auto missingInt = json::Jsonable::getOptionalInt64(doc, "missing");
    EXPECT_FALSE(missingInt.has_value());
}

// 타입 변환 유연성 테스트
TEST_F(JsonableTest, TypeConversionFlexibility) {
    std::string jsonStr = R"({
        "intAsString": "123",
        "floatAsInt": 3,
        "boolAsInt": 1,
        "stringAsBool": "true"
    })";
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    // 암시적 변환이 잘 작동하는지 확인
    EXPECT_EQ(json::Jsonable::getInt64(doc, "intAsString"), 0); // 문자열은 숫자로 변환되지 않음
    EXPECT_FLOAT_EQ(json::Jsonable::getFloat(doc, "floatAsInt"), 3.0f);
    EXPECT_EQ(json::Jsonable::getInt64(doc, "boolAsInt"), 1);
}

// 경계값 테스트
TEST_F(JsonableTest, BoundaryValueTests) {
    std::string jsonStr = R"({
        "maxFloat": 3.4028235e+38,
        "minFloat": -3.4028235e+38,
        "maxDouble": 1.7976931348623157e+308,
        "minDouble": -1.7976931348623157e+308
    })";
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    // 큰 값들이 올바르게 처리되는지 확인
    EXPECT_GT(json::Jsonable::getFloat(doc, "maxFloat"), 3.0e+38f);
    EXPECT_LT(json::Jsonable::getFloat(doc, "minFloat"), -3.0e+38f);
    EXPECT_GT(json::Jsonable::getDouble(doc, "maxDouble"), 1.0e+308);
    EXPECT_LT(json::Jsonable::getDouble(doc, "minDouble"), -1.0e+308);
}

// 배열 반복자 함수 테스트
TEST_F(JsonableTest, IterationFunctorArrayTest) {
    std::string jsonStr = R"({
        "numbers": [1, 2, 3, 4, 5],
        "strings": ["apple", "banana", "cherry"],
        "mixed": [1, "two", 3.0, true]
    })";
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    // 숫자 배열 반복
    std::vector<int> numbers;
    json::Jsonable::iterateArray(doc, "numbers", [&numbers](const rapidjson::Value& val) {
        if (val.IsInt()) {
            numbers.push_back(val.GetInt());
        }
    });
    EXPECT_EQ(numbers.size(), 5);
    EXPECT_EQ(numbers[0], 1);
    EXPECT_EQ(numbers[4], 5);
    
    // 문자열 배열 반복
    std::vector<std::string> strings;
    json::Jsonable::iterateArray(doc, "strings", [&strings](const rapidjson::Value& val) {
        if (val.IsString()) {
            strings.push_back(val.GetString());
        }
    });
    EXPECT_EQ(strings.size(), 3);
    EXPECT_EQ(strings[0], "apple");
    EXPECT_EQ(strings[2], "cherry");
}

// 객체 반복자 테스트
TEST_F(JsonableTest, ObjectIterationTest) {
    std::string jsonStr = R"({
        "person": {
            "name": "John",
            "age": 30,
            "city": "Seoul"
        },
        "config": {
            "debug": true,
            "timeout": 5000
        }
    })";
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    // 객체 멤버 반복
    std::map<std::string, std::string> personData;
    json::Jsonable::iterateObject(doc, "person", [&personData](const char* key, const rapidjson::Value& val) {
        if (val.IsString()) {
            personData[key] = val.GetString();
        } else if (val.IsInt()) {
            personData[key] = std::to_string(val.GetInt());
        }
    });
    
    EXPECT_EQ(personData["name"], "John");
    EXPECT_EQ(personData["age"], "30");
    EXPECT_EQ(personData["city"], "Seoul");
    
    // 루트 레벨 객체 반복
    std::vector<std::string> rootKeys;
    json::Jsonable::iterateObject(doc, [&rootKeys](const char* key, const rapidjson::Value& val) {
        rootKeys.push_back(key);
    });
    EXPECT_EQ(rootKeys.size(), 2);
    EXPECT_TRUE(std::find(rootKeys.begin(), rootKeys.end(), "person") != rootKeys.end());
    EXPECT_TRUE(std::find(rootKeys.begin(), rootKeys.end(), "config") != rootKeys.end());
}

// 중첩 경로 탐색 테스트
TEST_F(JsonableTest, NestedPathNavigationTest) {
    std::string jsonStr = R"({
        "company": {
            "name": "TechCorp",
            "departments": {
                "engineering": {
                    "head": "Alice",
                    "budget": 1000000
                },
                "sales": {
                    "head": "Bob",
                    "budget": 500000
                }
            }
        }
    })";
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    // 깊은 경로 탐색
    EXPECT_EQ(json::Jsonable::getNestedString(doc, "company.name"), "TechCorp");
    EXPECT_EQ(json::Jsonable::getNestedString(doc, "company.departments.engineering.head"), "Alice");
    EXPECT_EQ(json::Jsonable::getNestedInt64(doc, "company.departments.engineering.budget"), 1000000);
    EXPECT_EQ(json::Jsonable::getNestedString(doc, "company.departments.sales.head"), "Bob");
    
    // 존재하지 않는 경로
    EXPECT_EQ(json::Jsonable::getNestedString(doc, "company.departments.marketing.head", "Unknown"), "Unknown");
    EXPECT_FALSE(json::Jsonable::hasNestedPath(doc, "company.departments.marketing"));
    EXPECT_TRUE(json::Jsonable::hasNestedPath(doc, "company.departments.engineering"));
}

// 중첩 배열 반복 테스트
TEST_F(JsonableTest, NestedArrayIterationTest) {
    std::string jsonStr = R"({
        "data": {
            "categories": {
                "fruits": ["apple", "banana", "orange"],
                "colors": ["red", "green", "blue"]
            }
        }
    })";
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    // 중첩된 배열 반복
    std::vector<std::string> fruits;
    json::Jsonable::iterateNestedArray(doc, "data.categories.fruits", [&fruits](const rapidjson::Value& val) {
        if (val.IsString()) {
            fruits.push_back(val.GetString());
        }
    });
    
    EXPECT_EQ(fruits.size(), 3);
    EXPECT_EQ(fruits[0], "apple");
    EXPECT_EQ(fruits[1], "banana");
    EXPECT_EQ(fruits[2], "orange");
}

// 함수 기반 배열 추출 테스트
TEST_F(JsonableTest, FunctorBasedArrayExtractionTest) {
    std::string jsonStr = R"({
        "stringArray": ["hello", "world", "test"],
        "intArray": [1, 2, 3, 4, 5],
        "doubleArray": [1.1, 2.2, 3.3]
    })";
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    // 문자열 배열 추출
    auto strings = json::Jsonable::getStringArray(doc, "stringArray");
    EXPECT_EQ(strings.size(), 3);
    EXPECT_EQ(strings[0], "hello");
    EXPECT_EQ(strings[2], "test");
    
    // 정수 배열 추출
    auto ints = json::Jsonable::getInt64Array(doc, "intArray");
    EXPECT_EQ(ints.size(), 5);
    EXPECT_EQ(ints[0], 1);
    EXPECT_EQ(ints[4], 5);
    
    // 실수 배열 추출
    auto doubles = json::Jsonable::getDoubleArray(doc, "doubleArray");
    EXPECT_EQ(doubles.size(), 3);
    EXPECT_DOUBLE_EQ(doubles[0], 1.1);
    EXPECT_DOUBLE_EQ(doubles[2], 3.3);
}

// 함수 기반 배열 생성 테스트
TEST_F(JsonableTest, ArrayCreationWithFunctorTest) {
    rapidjson::Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();
    
    // 문자열 배열 생성
    std::vector<std::string> strings = {"hello", "world", "test"};
    std::function<rapidjson::Value(const std::string&)> stringConverter = 
        [&allocator](const std::string& str) {
            rapidjson::Value val;
            val.SetString(str.c_str(), static_cast<rapidjson::SizeType>(str.length()), allocator);
            return val;
        };
    auto stringArray = json::Jsonable::createArray(strings, allocator, stringConverter);
    
    doc.AddMember("strings", stringArray, allocator);
    
    // 정수 배열 생성
    std::vector<int> ints = {1, 2, 3, 4, 5};
    std::function<rapidjson::Value(const int&)> intConverter = 
        [](const int& i) {
            rapidjson::Value val(i);
            return val;
        };
    auto intArray = json::Jsonable::createArray(ints, allocator, intConverter);
    
    doc.AddMember("numbers", intArray, allocator);
    
    // 생성된 배열 검증
    EXPECT_TRUE(doc["strings"].IsArray());
    EXPECT_EQ(doc["strings"].GetArray().Size(), 3u);
    EXPECT_EQ(std::string(doc["strings"][0].GetString()), "hello");
    
    EXPECT_TRUE(doc["numbers"].IsArray());
    EXPECT_EQ(doc["numbers"].GetArray().Size(), 5u);
    EXPECT_EQ(doc["numbers"][0].GetInt(), 1);
}

// Department 기본 직렬화 테스트
TEST_F(JsonableTest, DepartmentBasicSerializationTest) {
    test::Department dept("Engineering", "Alice Smith", 15, 1500000.0);
    dept.addProject("Project Alpha");
    dept.addProject("Project Beta");
    
    // 직렬화
    std::string json = dept.toJson();
    EXPECT_FALSE(json.empty());
    
    // 역직렬화
    test::Department restored;
    restored.fromJson(json);
    
    // 검증
    EXPECT_EQ(dept, restored);
    EXPECT_EQ(restored.getName(), "Engineering");
    EXPECT_EQ(restored.getManager(), "Alice Smith");
    EXPECT_EQ(restored.getEmployeeCount(), 15);
    EXPECT_DOUBLE_EQ(restored.getBudget(), 1500000.0);
    EXPECT_EQ(restored.getProjects().size(), 2);
}

// Company 복잡한 중첩 직렬화 테스트
TEST_F(JsonableTest, CompanyComplexNestedSerializationTest) {
    test::Company company("TechCorp Inc.", "123 Tech Street, Seoul");
    
    // 부서 추가
    test::Department engDept("Engineering", "Alice", 20, 2000000.0);
    engDept.addProject("AI Platform");
    engDept.addProject("Cloud Migration");
    company.addDepartment(engDept);
    
    test::Department salesDept("Sales", "Bob", 10, 800000.0);
    salesDept.addProject("Q4 Campaign");
    company.addDepartment(salesDept);
    
    // 직원 추가
    test::Person emp1("John Doe", 30, false);
    emp1.addHobby("Programming");
    emp1.addHobby("Gaming");
    company.addEmployee(emp1);
    
    test::Person emp2("Jane Smith", 28, false);
    emp2.addHobby("Design");
    company.addEmployee(emp2);
    
    // 직렬화
    std::string json = company.toJson();
    EXPECT_FALSE(json.empty());
    
    // 역직렬화
    test::Company restored;
    restored.fromJson(json);
    
    // 검증
    EXPECT_EQ(company, restored);
    EXPECT_EQ(restored.getName(), "TechCorp Inc.");
    EXPECT_EQ(restored.getDepartments().size(), 2);
    EXPECT_EQ(restored.getEmployees().size(), 2);
    EXPECT_EQ(restored.getDepartments()[0].getProjects().size(), 2);
}

// 복잡한 JSON 문자열에서 Company 역직렬화 테스트
TEST_F(JsonableTest, ComplexJsonToCompanyDeserializationTest) {
    std::string complexJson = R"({
        "name": "GlobalTech Corp",
        "address": "456 Innovation Ave, Busan",
        "departments": [
            {
                "name": "Research",
                "manager": "Dr. Kim",
                "employeeCount": 25,
                "budget": 3000000.0,
                "projects": ["Quantum Computing", "AI Ethics", "Blockchain Study"]
            },
            {
                "name": "Marketing",
                "manager": "Ms. Park",
                "employeeCount": 8,
                "budget": 600000.0,
                "projects": ["Brand Refresh", "Social Media Campaign"]
            }
        ],
        "employees": [
            {
                "name": "Alex Johnson",
                "age": 32,
                "isStudent": false,
                "hobbies": ["Reading", "Hiking", "Photography"]
            },
            {
                "name": "Sarah Connor",
                "age": 29,
                "isStudent": false,
                "hobbies": ["Martial Arts", "Technology"]
            }
        ]
    })";
    
    test::Company company;
    company.fromJson(complexJson);
    
    // 회사 정보 검증
    EXPECT_EQ(company.getName(), "GlobalTech Corp");
    EXPECT_EQ(company.getAddress(), "456 Innovation Ave, Busan");
    
    // 부서 검증
    const auto& departments = company.getDepartments();
    EXPECT_EQ(departments.size(), 2);
    EXPECT_EQ(departments[0].getName(), "Research");
    EXPECT_EQ(departments[0].getProjects().size(), 3);
    EXPECT_EQ(departments[1].getName(), "Marketing");
    EXPECT_EQ(departments[1].getProjects().size(), 2);
    
    // 직원 검증
    const auto& employees = company.getEmployees();
    EXPECT_EQ(employees.size(), 2);
    EXPECT_EQ(employees[0].getName(), "Alex Johnson");
    EXPECT_EQ(employees[0].getHobbies().size(), 3);
    EXPECT_EQ(employees[1].getName(), "Sarah Connor");
    EXPECT_EQ(employees[1].getHobbies().size(), 2);
}

// 빈 컬렉션이 있는 복잡한 구조 테스트
TEST_F(JsonableTest, ComplexStructureWithEmptyCollectionsTest) {
    test::Company company("StartupCorp", "789 Startup Blvd");
    
    // 빈 부서 추가
    test::Department emptyDept("NewDept", "TBD", 0, 0.0);
    company.addDepartment(emptyDept);
    
    // 취미가 없는 직원 추가
    test::Person employee("New Hire", 25, false);
    company.addEmployee(employee);
    
    // 라운드트립 테스트
    std::string json = company.toJson();
    test::Company restored;
    restored.fromJson(json);
    
    EXPECT_EQ(company, restored);
    EXPECT_EQ(restored.getDepartments().size(), 1);
    EXPECT_EQ(restored.getDepartments()[0].getProjects().size(), 0);
    EXPECT_EQ(restored.getEmployees().size(), 1);
    EXPECT_EQ(restored.getEmployees()[0].getHobbies().size(), 0);
}

// 깊은 중첩 스트레스 테스트
TEST_F(JsonableTest, DeepNestingStressTest) {
    test::Company company("MegaCorp", "Enterprise Plaza");
    
    // 여러 부서와 직원으로 스트레스 테스트
    for (int i = 0; i < 3; ++i) {
        test::Department dept("Dept " + std::to_string(i), "Manager " + std::to_string(i), 
                             10 + i, 100000.0 * (i + 1));
        
        for (int j = 0; j < 2; ++j) {
            dept.addProject("Project " + std::to_string(i) + "-" + std::to_string(j));
        }
        
        company.addDepartment(dept);
    }
    
    for (int i = 0; i < 5; ++i) {
        test::Person emp("Employee " + std::to_string(i), 25 + i, i % 2 == 0);
        for (int j = 0; j < 4; ++j) {
            emp.addHobby("Hobby " + std::to_string(i) + "-" + std::to_string(j));
        }
        company.addEmployee(emp);
    }
    
    // 라운드트립 테스트
    std::string json = company.toJson();
    test::Company restored;
    restored.fromJson(json);
    
    EXPECT_EQ(company, restored);
    EXPECT_EQ(restored.getDepartments().size(), 3);
    EXPECT_EQ(restored.getEmployees().size(), 5);
    
    // 첫 번째 직원의 취미 수 확인
    const auto& firstEmp = restored.getEmployees()[0];
    EXPECT_EQ(firstEmp.getHobbies().size(), 4);
    EXPECT_EQ(firstEmp.getHobbies()[0], "Hobby 0-0");
}

// ========================================
// 간단한 사용법 테스트 (내재적 직렬화 패턴)
// ========================================

// 매크로를 사용한 간단한 클래스 테스트
TEST_F(JsonableTest, SimpleProductMacroUsageTest) {
    test::SimpleProduct product("Laptop", 12345, 999.99, true);
    
    // 직렬화 - 다른 JSON 라이브러리 학습 없이 바로 사용!
    std::string json = product.toJson();
    EXPECT_FALSE(json.empty());
    
    // 역직렬화 - 마찬가지로 바로 사용!
    test::SimpleProduct restored;
    restored.fromJson(json);
    
    // 검증
    EXPECT_EQ(product, restored);
    EXPECT_EQ(restored.getName(), "Laptop");
    EXPECT_EQ(restored.getId(), 12345);
    EXPECT_DOUBLE_EQ(restored.getPrice(), 999.99);
    EXPECT_TRUE(restored.isAvailable());
}

// JSON 문자열에서 직접 파싱 테스트
TEST_F(JsonableTest, SimpleProductFromJsonStringTest) {
    std::string jsonInput = R"({
        "name": "Smartphone",
        "id": 67890,
        "price": 599.50,
        "available": false
    })";
    
    test::SimpleProduct product;
    product.fromJson(jsonInput); // 한 줄로 끝!
    
    EXPECT_EQ(product.getName(), "Smartphone");
    EXPECT_EQ(product.getId(), 67890);
    EXPECT_DOUBLE_EQ(product.getPrice(), 599.50);
    EXPECT_FALSE(product.isAvailable());
}

// 비침입적(Non-intrusive) 방식 테스트
TEST_F(JsonableTest, NonIntrusiveDataTest) {
    test::NonIntrusiveData data("Sample Title", 42);
    
    // 상속 없이도 JSON 직렬화 가능
    std::string json = data.toJson();
    EXPECT_FALSE(json.empty());
    
    // 정적 메서드로 역직렬화
    auto restored = test::NonIntrusiveData::fromJson(json);
    
    EXPECT_EQ(data, restored);
    EXPECT_EQ(restored.getTitle(), "Sample Title");
    EXPECT_EQ(restored.getCount(), 42);
}

// 사용 패턴 비교 테스트
TEST_F(JsonableTest, UsagePatternComparisonTest) {
    // 1. 내재적(Intrusive) 방식 - 상속 사용
    test::SimpleProduct intrusiveProduct("Gaming Mouse", 111, 79.99, true);
    std::string intrusiveJson = intrusiveProduct.toJson();
    
    test::SimpleProduct restoredIntrusive;
    restoredIntrusive.fromJson(intrusiveJson);
    
    // 2. 비침입적(Non-intrusive) 방식 - 상속 없음
    test::NonIntrusiveData nonIntrusiveData("Mouse Data", 111);
    std::string nonIntrusiveJson = nonIntrusiveData.toJson();
    
    auto restoredNonIntrusive = test::NonIntrusiveData::fromJson(nonIntrusiveJson);
    
    // 둘 다 성공적으로 동작
    EXPECT_EQ(intrusiveProduct, restoredIntrusive);
    EXPECT_EQ(nonIntrusiveData, restoredNonIntrusive);
    
    // JSON 결과도 올바름
    EXPECT_TRUE(intrusiveJson.find("Gaming Mouse") != std::string::npos);
    EXPECT_TRUE(nonIntrusiveJson.find("Mouse Data") != std::string::npos);
}

// 실제 사용 시나리오 - 설정 파일 로딩
TEST_F(JsonableTest, ConfigurationLoadingScenarioTest) {
    // 애플리케이션 설정 같은 실제 사용 사례
    std::string configJson = R"({
        "name": "MyApp Config",
        "id": 1,
        "price": 0.0,
        "available": true
    })";
    
    test::SimpleProduct config;
    
    // JSON 설정 파일을 한 줄로 로딩
    config.fromJson(configJson);
    
    // 설정값 사용
    if (config.isAvailable()) {
        EXPECT_EQ(config.getName(), "MyApp Config");
        EXPECT_EQ(config.getId(), 1);
    }
    
    // 설정 변경 후 저장
    config.setPrice(29.99);
    config.setAvailable(false);
    
    std::string savedConfig = config.toJson();
    EXPECT_TRUE(savedConfig.find("29.99") != std::string::npos);
    EXPECT_TRUE(savedConfig.find("false") != std::string::npos);
}

// 메타프로그래밍 기반 자동화 테스트
TEST_F(JsonableTest, MetaProgrammingAutomationTest) {
    using namespace test;
    
    // 테스트 데이터 생성
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    std::vector<std::string> tags = {"test", "meta", "programming"};
    std::vector<SimpleProduct> products = {
        SimpleProduct("Product A", 1, 10.0, true),
        SimpleProduct("Product B", 2, 20.0, false)
    };
    
    MetaProgrammingExample example("Meta Example", numbers, tags, products);
    
    // 직렬화
    std::string json = example.toJson();
    
    // JSON 내용 검증
    EXPECT_TRUE(json.find("Meta Example") != std::string::npos);
    EXPECT_TRUE(json.find("[1,2,3,4,5]") != std::string::npos);
    EXPECT_TRUE(json.find("test") != std::string::npos);
    EXPECT_TRUE(json.find("Product A") != std::string::npos);
    
    // 역직렬화
    MetaProgrammingExample restored;
    restored.fromJson(json);
    
    // 자동 타입 변환 검증
    EXPECT_EQ(restored.getTitle(), "Meta Example");
    EXPECT_EQ(restored.getNumbers().size(), 5);
    EXPECT_EQ(restored.getTags().size(), 3);
    EXPECT_EQ(restored.getProducts().size(), 2);
    
    // 구체적인 값 검증
    EXPECT_EQ(restored.getNumbers()[0], 1);
    EXPECT_EQ(restored.getNumbers()[4], 5);
    EXPECT_EQ(restored.getTags()[0], "test");
    EXPECT_EQ(restored.getTags()[2], "programming");
    EXPECT_EQ(restored.getProducts()[0].getName(), "Product A");
    EXPECT_EQ(restored.getProducts()[1].getId(), 2);
    
    // 완전한 동등성 검증
    EXPECT_EQ(example, restored);
}

// 자동 배열 처리 테스트
TEST_F(JsonableTest, AutoArrayProcessingTest) {
    std::string jsonStr = R"({
        "integers": [10, 20, 30],
        "strings": ["hello", "world", "auto"],
        "doubles": [1.1, 2.2, 3.3]
    })";
    
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    // 메타프로그래밍 기반 자동 배열 추출
    auto integers = json::Jsonable::getArray<int>(doc, "integers");
    auto strings = json::Jsonable::getArray<std::string>(doc, "strings");
    auto doubles = json::Jsonable::getArray<double>(doc, "doubles");
    
    // 검증
    ASSERT_EQ(integers.size(), 3);
    EXPECT_EQ(integers[0], 10);
    EXPECT_EQ(integers[2], 30);
    
    ASSERT_EQ(strings.size(), 3);
    EXPECT_EQ(strings[0], "hello");
    EXPECT_EQ(strings[2], "auto");
    
    ASSERT_EQ(doubles.size(), 3);
    EXPECT_DOUBLE_EQ(doubles[0], 1.1);
    EXPECT_DOUBLE_EQ(doubles[2], 3.3);
    
    // 자동 배열 생성 테스트
    rapidjson::Document newDoc;
    newDoc.SetObject();
    auto& allocator = newDoc.GetAllocator();
    
    auto intArray = json::Jsonable::createArray(integers, allocator);
    auto stringArray = json::Jsonable::createArray(strings, allocator);
    auto doubleArray = json::Jsonable::createArray(doubles, allocator);
    
    newDoc.AddMember("integers", intArray, allocator);
    newDoc.AddMember("strings", stringArray, allocator);
    newDoc.AddMember("doubles", doubleArray, allocator);
    
    std::string result = json::Jsonable::valueToString(newDoc);
    EXPECT_TRUE(result.find("[10,20,30]") != std::string::npos);
    EXPECT_TRUE(result.find("hello") != std::string::npos);
    EXPECT_TRUE(result.find("1.1") != std::string::npos);
}

// 자동 필드 설정 테스트
TEST_F(JsonableTest, AutoFieldSettingTest) {
    std::string jsonStr = R"({
        "name": "AutoField",
        "age": 25,
        "height": 175.5,
        "active": true
    })";
    
    auto doc = json::Jsonable::parseJson(jsonStr);
    
    // 메타프로그래밍 기반 자동 필드 설정
    std::string name;
    int age;
    double height;
    bool active;
    
    json::Jsonable::setField(doc, "name", name);
    json::Jsonable::setField(doc, "age", age);
    json::Jsonable::setField(doc, "height", height);
    json::Jsonable::setField(doc, "active", active);
    
    // 검증
    EXPECT_EQ(name, "AutoField");
    EXPECT_EQ(age, 25);
    EXPECT_DOUBLE_EQ(height, 175.5);
    EXPECT_TRUE(active);
    
    // 자동 필드 추가 테스트
    rapidjson::Document newDoc;
    newDoc.SetObject();
    auto& allocator = newDoc.GetAllocator();
    
    json::Jsonable::addField(newDoc, "name", name, allocator);
    json::Jsonable::addField(newDoc, "age", age, allocator);
    json::Jsonable::addField(newDoc, "height", height, allocator);
    json::Jsonable::addField(newDoc, "active", active, allocator);
    
    std::string result = json::Jsonable::valueToString(newDoc);
    EXPECT_TRUE(result.find("AutoField") != std::string::npos);
    EXPECT_TRUE(result.find("25") != std::string::npos);
    EXPECT_TRUE(result.find("175.5") != std::string::npos);
    EXPECT_TRUE(result.find("true") != std::string::npos);
} 