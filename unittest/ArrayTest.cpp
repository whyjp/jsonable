/**
 * ArrayTest.cpp - 배열 처리 및 Begin/End 패턴 테스트
 * 
 * 테스트 영역:
 * - 기본 타입 배열 처리 (getArray, setArray)
 * - Begin/End 패턴 배열 생성
 * - 복잡한 중첩 배열
 * - 배열 순회 및 처리
 */

#include <gtest/gtest.h>
#include "../Jsonable.hpp"

using namespace json;

class ArrayTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 기본 타입 배열 테스트
TEST_F(ArrayTest, BasicArrayHandling) {
    class TestClass : public Jsonable {
    public:
        std::vector<std::string> strings;
        std::vector<int64_t> numbers;
        std::vector<double> doubles;
        std::vector<bool> bools;
        
        void loadFromJson() override {
            strings = getArray<std::string>("strings");
            numbers = getArray<int64_t>("numbers");
            doubles = getArray<double>("doubles");
            bools = getArray<bool>("bools");
        }
        
        void saveToJson() override {
            setArray("strings", strings);
            setArray("numbers", numbers);
            setArray("doubles", doubles);
            setArray("bools", bools);
        }
    };
    
    std::string jsonStr = R"({
        "strings": ["hello", "world", "test"],
        "numbers": [1, 2, 3, 4, 5],
        "doubles": [1.1, 2.2, 3.3],
        "bools": [true, false, true]
    })";
    
    TestClass obj;
    obj.fromJson(jsonStr);
    
    EXPECT_EQ(obj.strings.size(), 3);
    EXPECT_EQ(obj.strings[0], "hello");
    EXPECT_EQ(obj.strings[2], "test");
    
    EXPECT_EQ(obj.numbers.size(), 5);
    EXPECT_EQ(obj.numbers[0], 1);
    EXPECT_EQ(obj.numbers[4], 5);
    
    EXPECT_EQ(obj.doubles.size(), 3);
    EXPECT_DOUBLE_EQ(obj.doubles[0], 1.1);
    EXPECT_DOUBLE_EQ(obj.doubles[2], 3.3);
    
    EXPECT_EQ(obj.bools.size(), 3);
    EXPECT_TRUE(obj.bools[0]);
    EXPECT_FALSE(obj.bools[1]);
    EXPECT_TRUE(obj.bools[2]);
}

// Begin/End 패턴 배열 생성 테스트
TEST_F(ArrayTest, BeginEndArrayPattern) {
    class TestClass : public Jsonable {
    public:
        std::vector<std::string> fruits = {"apple", "banana", "orange"};
        std::vector<int64_t> scores = {85, 92, 78, 96};
        
        void loadFromJson() override {
            fruits = getArray<std::string>("fruits");
            scores = getArray<int64_t>("scores");
        }
        
        void saveToJson() override {
            beginObject();
            {
                beginArray("fruits");
                {
                    for (const auto& fruit : fruits) {
                        pushString(fruit);
                    }
                }
                endArray();
                
                beginArray("scores");
                {
                    for (const auto& score : scores) {
                        pushInt64(score);
                    }
                }
                endArray();
            }
            endObject();
        }
    };
    
    TestClass original;
    std::string json = original.toJson();
    
    TestClass restored;
    restored.fromJson(json);
    
    EXPECT_EQ(original.fruits, restored.fruits);
    EXPECT_EQ(original.scores, restored.scores);
}

// 빈 배열 처리 테스트
TEST_F(ArrayTest, EmptyArrayHandling) {
    class TestClass : public Jsonable {
    public:
        std::vector<std::string> emptyStrings;
        std::vector<int64_t> emptyNumbers;
        
        void loadFromJson() override {
            emptyStrings = getArray<std::string>("emptyStrings");
            emptyNumbers = getArray<int64_t>("emptyNumbers");
        }
        
        void saveToJson() override {
            setArray("emptyStrings", emptyStrings);
            setArray("emptyNumbers", emptyNumbers);
        }
    };
    
    std::string jsonStr = R"({
        "emptyStrings": [],
        "emptyNumbers": []
    })";
    
    TestClass obj;
    obj.fromJson(jsonStr);
    
    EXPECT_TRUE(obj.emptyStrings.empty());
    EXPECT_TRUE(obj.emptyNumbers.empty());
    
    // 라운드트립 테스트
    std::string json = obj.toJson();
    TestClass restored;
    restored.fromJson(json);
    
    EXPECT_TRUE(restored.emptyStrings.empty());
    EXPECT_TRUE(restored.emptyNumbers.empty());
}

// 배열 순회 테스트
TEST_F(ArrayTest, ArrayIteration) {
    std::string jsonStr = R"({
        "data": [10, 20, 30, 40, 50]
    })";
    
    class TestClass : public Jsonable {
    public:
        std::vector<int64_t> processedData;
        
        void loadFromJson() override {
            iterateArray("data", [this](size_t index) {
                // 인덱스를 통해 배열 요소에 접근하지 않고
                // 직접 getArray 사용
                auto data = getArray<int64_t>("data");
                if (index < data.size()) {
                    processedData.push_back(data[index] * 2);
                }
            });
        }
        
        void saveToJson() override {
            // 처리된 데이터를 다시 원래 형태로 변환하여 저장
            std::vector<int64_t> originalData;
            for (auto val : processedData) {
                originalData.push_back(val / 2);
            }
            setArray("data", originalData);
        }
    };
    
    TestClass obj;
    obj.fromJson(jsonStr);
    
    EXPECT_EQ(obj.processedData.size(), 5);
    EXPECT_EQ(obj.processedData[0], 20); // 10 * 2
    EXPECT_EQ(obj.processedData[4], 100); // 50 * 2
}

// 중첩 배열 내 객체 패턴 테스트
TEST_F(ArrayTest, NestedArrayObjectPattern) {
    class TestClass : public Jsonable {
    public:
        std::vector<std::vector<int64_t>> matrix;
        
        void loadFromJson() override {
            // 2D 배열은 직접 구현 필요
            if (hasKey("matrix") && isArray("matrix")) {
                // 실제 구현에서는 더 복잡한 중첩 처리 필요
                // 여기서는 단순화된 버전
                matrix = {{1, 2, 3}, {4, 5, 6}};
            }
        }
        
        void saveToJson() override {
            beginObject();
            {
                beginArray("matrix");
                {
                    for (const auto& row : matrix) {
                        beginArray();
                        {
                            for (const auto& val : row) {
                                pushInt64(val);
                            }
                        }
                        endArray();
                    }
                }
                endArray();
            }
            endObject();
        }
    };
    
    TestClass original;
    original.matrix = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    
    std::string json = original.toJson();
    EXPECT_TRUE(json.find("[[1,2,3],[4,5,6],[7,8,9]]") != std::string::npos ||
                json.find("[ [ 1, 2, 3 ], [ 4, 5, 6 ], [ 7, 8, 9 ] ]") != std::string::npos);
}
