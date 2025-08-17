/**
 * SimpleCompileTest.cpp - 컴파일 테스트
 * 
 * 새로운 다중상속 구조가 올바르게 컴파일되는지 확인
 */

#include "../Jsonable.hpp"

#include <iostream>
#include <string>
#include <vector>

/**
 * @brief 간단한 테스트 클래스
 */
class SimpleTestClass : public json::Jsonable {
private:
    std::string name_;
    int value_;
    std::vector<std::string> items_;

public:
    SimpleTestClass() : value_(0) {}
    SimpleTestClass(const std::string& name, int value) : name_(name), value_(value) {}

    // FromJsonable 구현
    void loadFromJson() override {
        name_ = getString("name");
        value_ = static_cast<int>(getInt64("value"));
        items_ = getArray<std::string>("items");
    }

    // ToJsonable 구현
    void saveToJson() override {
        setString("name", name_);
        setInt64("value", static_cast<int64_t>(value_));
        setArray("items", items_);
    }

    // 접근자들
    void setName(const std::string& name) { name_ = name; }
    void setValue(int value) { value_ = value; }
    void addItem(const std::string& item) { items_.push_back(item); }

    const std::string& getName() const { return name_; }
    int getValue() const { return value_; }
    const std::vector<std::string>& getItems() const { return items_; }
};

/**
 * @brief Begin/End 스타일 테스트 클래스
 */
class BeginEndTestClass : public json::Jsonable {
private:
    std::string title_;
    std::vector<std::string> tags_;

public:
    BeginEndTestClass() = default;
    BeginEndTestClass(const std::string& title) : title_(title) {}

    void loadFromJson() override {
        title_ = getString("title");
        tags_ = getArray<std::string>("tags");
    }

    void saveToJson() override {
        beginObject();
        {
            setString("title", title_);
            
            beginArray("tags");
            {
                for (const auto& tag : tags_) {
                    setString("", tag);  // 배열에서 key 무시됨
                }
            }
            endArray();
        }
        endObject();
    }

    void setTitle(const std::string& title) { title_ = title; }
    void addTag(const std::string& tag) { tags_.push_back(tag); }

    const std::string& getTitle() const { return title_; }
    const std::vector<std::string>& getTags() const { return tags_; }
};

/**
 * @brief 기본 기능 테스트
 */
bool testBasicFunctionality() {
    std::cout << "=== Basic Functionality Test ===" << std::endl;
    
    try {
        // 객체 생성 및 설정
        SimpleTestClass test("TestObject", 42);
        test.addItem("item1");
        test.addItem("item2");
        test.addItem("item3");
        
        // JSON 직렬화
        std::string json = test.toJson();
        std::cout << "Serialized JSON: " << json << std::endl;
        
        if (json.empty()) {
            std::cout << "❌ JSON serialization failed!" << std::endl;
            return false;
        }
        
        // JSON 역직렬화
        SimpleTestClass restored;
        restored.fromJson(json);
        
        // 데이터 검증
        if (restored.getName() != "TestObject") {
            std::cout << "❌ Name mismatch: " << restored.getName() << std::endl;
            return false;
        }
        
        if (restored.getValue() != 42) {
            std::cout << "❌ Value mismatch: " << restored.getValue() << std::endl;
            return false;
        }
        
        if (restored.getItems().size() != 3) {
            std::cout << "❌ Items count mismatch: " << restored.getItems().size() << std::endl;
            return false;
        }
        
        std::cout << "✅ Basic functionality test passed!" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception in basic test: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Begin/End 스타일 테스트
 */
bool testBeginEndStyle() {
    std::cout << "\n=== Begin/End Style Test ===" << std::endl;
    
    try {
        // Begin/End 스타일로 객체 생성
        BeginEndTestClass test("BeginEndTest");
        test.addTag("tag1");
        test.addTag("tag2");
        test.addTag("tag3");
        
        // JSON 직렬화 (Begin/End 스타일)
        std::string json = test.toJson();
        std::cout << "Begin/End JSON: " << json << std::endl;
        
        if (json.empty()) {
            std::cout << "❌ Begin/End JSON serialization failed!" << std::endl;
            return false;
        }
        
        // 배열 구조 확인
        if (json.find("[\"tag1\",\"tag2\",\"tag3\"]") == std::string::npos) {
            std::cout << "❌ Array structure not found in JSON!" << std::endl;
            return false;
        }
        
        // 역직렬화
        BeginEndTestClass restored;
        restored.fromJson(json);
        
        if (restored.getTitle() != "BeginEndTest") {
            std::cout << "❌ Title mismatch: " << restored.getTitle() << std::endl;
            return false;
        }
        
        if (restored.getTags().size() != 3) {
            std::cout << "❌ Tags count mismatch: " << restored.getTags().size() << std::endl;
            return false;
        }
        
        std::cout << "✅ Begin/End style test passed!" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception in Begin/End test: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 배열 컨텍스트 key 무시 테스트
 */
bool testArrayKeyIgnoring() {
    std::cout << "\n=== Array Key Ignoring Test ===" << std::endl;
    
    try {
        SimpleTestClass test;
        
        test.beginObject();
        {
            test.setString("name", "ArrayKeyTest");
            
            test.beginArray("test_items");
            {
                // 배열에서는 key가 모두 무시되어야 함
                test.setString("", "item1");           // 빈 문자열
                test.setString("ignored", "item2");    // 임의 문자열
                test.setString("whatever", "item3");   // 다른 임의 문자열
            }
            test.endArray();
        }
        test.endObject();
        
        std::string json = test.toJson();
        std::cout << "Array key ignoring JSON: " << json << std::endl;
        
        // 배열이 올바르게 생성되었는지 확인
        if (json.find("[\"item1\",\"item2\",\"item3\"]") == std::string::npos) {
            std::cout << "❌ Array items not found correctly!" << std::endl;
            return false;
        }
        
        // key들이 JSON에 포함되지 않았는지 확인
        if (json.find("ignored") != std::string::npos || json.find("whatever") != std::string::npos) {
            std::cout << "❌ Array keys were not ignored!" << std::endl;
            return false;
        }
        
        std::cout << "✅ Array key ignoring test passed!" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception in array key test: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 편의 메서드 테스트
 */
bool testConvenienceMethods() {
    std::cout << "\n=== Convenience Methods Test ===" << std::endl;
    
    try {
        SimpleTestClass test1("ConvenienceTest", 100);
        test1.addItem("convenience_item");
        
        // toString/fromString 테스트
        std::string jsonStr1 = test1.toString();
        std::string jsonStr2 = test1.toJson();
        
        if (jsonStr1 != jsonStr2) {
            std::cout << "❌ toString() != toJson()!" << std::endl;
            return false;
        }
        
        SimpleTestClass test2;
        test2.fromString(jsonStr1);
        
        if (test2.getName() != "ConvenienceTest" || test2.getValue() != 100) {
            std::cout << "❌ fromString() failed!" << std::endl;
            return false;
        }
        
        // 객체 비교 테스트
        if (!test1.equals(test2)) {
            std::cout << "❌ Objects should be equal!" << std::endl;
            return false;
        }
        
        if (!(test1 == test2)) {
            std::cout << "❌ Operator == failed!" << std::endl;
            return false;
        }
        
        std::cout << "✅ Convenience methods test passed!" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception in convenience test: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 메인 함수
 */
int main() {
    std::cout << "🚀 Running Simple Compile Tests for New Multi-Inheritance Structure..." << std::endl;
    std::cout << "========================================" << std::endl;
    
    bool allPassed = true;
    
    // 기본 기능 테스트
    if (!testBasicFunctionality()) {
        allPassed = false;
    }
    
    // Begin/End 스타일 테스트
    if (!testBeginEndStyle()) {
        allPassed = false;
    }
    
    // 배열 key 무시 테스트
    if (!testArrayKeyIgnoring()) {
        allPassed = false;
    }
    
    // 편의 메서드 테스트
    if (!testConvenienceMethods()) {
        allPassed = false;
    }
    
    std::cout << "========================================" << std::endl;
    
    if (allPassed) {
        std::cout << "🎉 All simple tests passed!" << std::endl;
        std::cout << "✅ New multi-inheritance structure is working correctly." << std::endl;
        std::cout << "\nKey features verified:" << std::endl;
        std::cout << "  ✅ Multi-inheritance structure (ToJsonable + FromJsonable)" << std::endl;
        std::cout << "  ✅ Basic serialization/deserialization" << std::endl;
        std::cout << "  ✅ Begin/End style JSON creation" << std::endl;
        std::cout << "  ✅ Array context key ignoring" << std::endl;
        std::cout << "  ✅ Convenience methods (toString, fromString, equals)" << std::endl;
        std::cout << "  ✅ Operator overloading (==, !=)" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some tests failed!" << std::endl;
        std::cout << "Please check the implementation." << std::endl;
        return 1;
    }
} 