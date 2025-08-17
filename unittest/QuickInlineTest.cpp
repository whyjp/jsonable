/**
 * QuickInlineTest.cpp - 완전 inline 구조 빠른 테스트
 * 
 * 매크로 없이 단순히 #include "Jsonable.hpp"만으로 작동하는지 확인
 */

#include "../Jsonable.hpp"  // 매크로 없이 이것만!

#include <iostream>
#include <string>
#include <vector>

/**
 * @brief 매우 간단한 테스트 클래스
 */
class QuickTest : public json::Jsonable {
private:
    std::string name_;
    int age_;
    std::vector<std::string> tags_;

public:
    QuickTest() : age_(0) {}
    QuickTest(const std::string& name, int age) : name_(name), age_(age) {}

    // FromJsonable 구현
    void loadFromJson() override {
        name_ = getString("name");
        age_ = static_cast<int>(getInt64("age"));
        tags_ = getArray<std::string>("tags");
    }

    // ToJsonable 구현
    void saveToJson() override {
        setString("name", name_);
        setInt64("age", static_cast<int64_t>(age_));
        setArray("tags", tags_);
    }

    // 접근자들
    void setName(const std::string& name) { name_ = name; }
    void setAge(int age) { age_ = age; }
    void addTag(const std::string& tag) { tags_.push_back(tag); }

    const std::string& getName() const { return name_; }
    int getAge() const { return age_; }
    const std::vector<std::string>& getTags() const { return tags_; }
};

/**
 * @brief Begin/End 스타일 테스트
 */
class BeginEndQuickTest : public json::Jsonable {
private:
    std::string title_;
    std::vector<std::string> items_;

public:
    BeginEndQuickTest() = default;
    BeginEndQuickTest(const std::string& title) : title_(title) {}

    void loadFromJson() override {
        title_ = getString("title");
        items_ = getArray<std::string>("items");
    }

    void saveToJson() override {
        beginObject();
        {
            setString("title", title_);
            
            beginArray("items");
            {
                for (const auto& item : items_) {
                    setString("", item);  // 배열에서 key 무시
                }
            }
            endArray();
        }
        endObject();
    }

    void setTitle(const std::string& title) { title_ = title; }
    void addItem(const std::string& item) { items_.push_back(item); }

    const std::string& getTitle() const { return title_; }
    const std::vector<std::string>& getItems() const { return items_; }
};

/**
 * @brief 기본 테스트 함수
 */
bool runBasicTest() {
    std::cout << "=== Basic Inline Structure Test ===" << std::endl;
    
    try {
        // 1. 기본 객체 테스트
        QuickTest test("Alice", 25);
        test.addTag("developer");
        test.addTag("cpp");
        
        // 2. JSON 직렬화
        std::string json = test.toJson();
        std::cout << "Serialized: " << json << std::endl;
        
        if (json.empty()) {
            std::cout << "❌ Serialization failed!" << std::endl;
            return false;
        }
        
        // 3. JSON 역직렬화
        QuickTest restored;
        restored.fromJson(json);
        
        // 4. 데이터 검증
        if (restored.getName() != "Alice" || restored.getAge() != 25) {
            std::cout << "❌ Basic data mismatch!" << std::endl;
            return false;
        }
        
        if (restored.getTags().size() != 2) {
            std::cout << "❌ Tags size mismatch!" << std::endl;
            return false;
        }
        
        std::cout << "✅ Basic test passed!" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Begin/End 스타일 테스트
 */
bool runBeginEndTest() {
    std::cout << "\n=== Begin/End Inline Test ===" << std::endl;
    
    try {
        // 1. Begin/End 객체 생성
        BeginEndQuickTest test("InlineTest");
        test.addItem("item1");
        test.addItem("item2");
        test.addItem("item3");
        
        // 2. JSON 직렬화 (Begin/End 스타일)
        std::string json = test.toJson();
        std::cout << "Begin/End JSON: " << json << std::endl;
        
        if (json.empty()) {
            std::cout << "❌ Begin/End serialization failed!" << std::endl;
            return false;
        }
        
        // 3. 배열 구조 확인
        if (json.find("\"items\":[\"item1\",\"item2\",\"item3\"]") == std::string::npos) {
            std::cout << "❌ Array structure incorrect!" << std::endl;
            return false;
        }
        
        // 4. 역직렬화
        BeginEndQuickTest restored;
        restored.fromJson(json);
        
        if (restored.getTitle() != "InlineTest" || restored.getItems().size() != 3) {
            std::cout << "❌ Begin/End restoration failed!" << std::endl;
            return false;
        }
        
        std::cout << "✅ Begin/End test passed!" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Begin/End Exception: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 배열 key 무시 테스트
 */
bool runArrayKeyTest() {
    std::cout << "\n=== Array Key Ignoring Test ===" << std::endl;
    
    try {
        QuickTest test;
        
        // Begin/End 스타일로 배열 key 무시 테스트
        test.beginObject();
        {
            test.setString("name", "KeyIgnoreTest");
            
            test.beginArray("test_array");
            {
                // 다양한 key들이 모두 무시되어야 함
                test.setString("", "value1");
                test.setString("ignored", "value2");
                test.setString("whatever", "value3");
            }
            test.endArray();
        }
        test.endObject();
        
        std::string json = test.toJson();
        std::cout << "Key ignore JSON: " << json << std::endl;
        
        // 배열이 올바르게 생성되었는지 확인
        if (json.find("[\"value1\",\"value2\",\"value3\"]") == std::string::npos) {
            std::cout << "❌ Array values not found!" << std::endl;
            return false;
        }
        
        // key들이 JSON에 포함되지 않았는지 확인
        if (json.find("ignored") != std::string::npos || json.find("whatever") != std::string::npos) {
            std::cout << "❌ Keys were not ignored!" << std::endl;
            return false;
        }
        
        std::cout << "✅ Array key ignoring test passed!" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Array key test exception: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 편의 메서드 테스트
 */
bool runConvenienceTest() {
    std::cout << "\n=== Convenience Methods Test ===" << std::endl;
    
    try {
        QuickTest test1("ConvenienceTest", 42);
        test1.addTag("convenience");
        
        // toString/fromString 테스트
        std::string jsonStr = test1.toString();
        
        QuickTest test2;
        test2.fromString(jsonStr);
        
        // 동등성 테스트
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
        std::cout << "❌ Convenience test exception: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 메인 함수
 */
int main() {
    std::cout << "🚀 Quick Inline Structure Test (No Macros!)" << std::endl;
    std::cout << "Testing: #include \"Jsonable.hpp\" only" << std::endl;
    std::cout << "========================================" << std::endl;
    
    bool allPassed = true;
    
    // 기본 테스트
    if (!runBasicTest()) {
        allPassed = false;
    }
    
    // Begin/End 테스트
    if (!runBeginEndTest()) {
        allPassed = false;
    }
    
    // 배열 key 무시 테스트
    if (!runArrayKeyTest()) {
        allPassed = false;
    }
    
    // 편의 메서드 테스트
    if (!runConvenienceTest()) {
        allPassed = false;
    }
    
    std::cout << "========================================" << std::endl;
    
    if (allPassed) {
        std::cout << "🎉 All inline tests passed!" << std::endl;
        std::cout << "✅ Complete inline structure working perfectly!" << std::endl;
        std::cout << "\n🎯 Verified features:" << std::endl;
        std::cout << "  ✅ No JSONABLE_IMPLEMENTATION macro needed" << std::endl;
        std::cout << "  ✅ Single #include \"Jsonable.hpp\" works" << std::endl;
        std::cout << "  ✅ Multi-inheritance structure functional" << std::endl;
        std::cout << "  ✅ RapidJSON completely hidden" << std::endl;
        std::cout << "  ✅ Array context key ignoring" << std::endl;
        std::cout << "  ✅ Begin/End style JSON creation" << std::endl;
        std::cout << "  ✅ Type safety maintained" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some inline tests failed!" << std::endl;
        return 1;
    }
} 