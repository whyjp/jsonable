#pragma once

/**
 * 통일된 API 데모
 * 
 * 배열과 객체에서 동일한 setXX 메서드 사용
 * pushXX는 선택적 편의 메서드
 */

#include "../Jsonable.hpp"

class UnifiedApiDemo : public json::Jsonable {
private:
    std::string name_;
    std::vector<std::string> tags_;
    std::vector<int> scores_;
    
    struct Config {
        std::string key;
        std::string value;
        bool enabled;
    };
    std::vector<Config> configs_;

public:
    UnifiedApiDemo() = default;
    
    void setupDemo() {
        name_ = "Unified API Demo";
        tags_ = {"json", "cpp", "library"};
        scores_ = {95, 87, 92};
        configs_ = {
            {"debug", "true", true},
            {"timeout", "30000", false},
            {"retries", "3", true}
        };
    }

    void loadFromJson() override {
        name_ = getString("name");
        tags_ = getArray<std::string>("tags");
        
        auto int64Scores = getArray<int64_t>("scores");
        scores_.clear();
        for (auto score : int64Scores) {
            scores_.push_back(static_cast<int>(score));
        }
        
        // configs 배열 로딩은 실제로는 더 복잡하지만 스킵
    }

    void saveToJson() override {
        beginObject();
        {
            setString("name", name_);
            
            // ========================================
            // 🎯 방법 1: 통일된 setXX API (권장)
            // ========================================
            
            beginArray("tags");
            {
                for (const auto& tag : tags_) {
                    // ✅ 배열 컨텍스트에서는 key 무시됨
                    setString("", tag);              // 빈 문자열
                    // setString(nullptr, tag);      // nullptr도 가능
                    // setString("ignored", tag);    // 어떤 key든 무시됨
                }
            }
            endArray();
            
            beginArray("scores");
            {
                for (int score : scores_) {
                    // ✅ 배열 컨텍스트에서는 key 무시됨
                    setInt64("", static_cast<int64_t>(score));
                }
            }
            endArray();
            
            // ========================================
            // 🎯 방법 2: pushXX 편의 메서드 (선택적)
            // ========================================
            
            beginArray("tags_alt");
            {
                for (const auto& tag : tags_) {
                    // ✅ pushXX는 배열 전용이지만 setXX와 동일한 결과
                    pushString(tag);
                }
            }
            endArray();
            
            beginArray("scores_alt");
            {
                for (int score : scores_) {
                    // ✅ pushXX는 명시적으로 배열 요소임을 나타냄
                    pushInt64(static_cast<int64_t>(score));
                }
            }
            endArray();
            
            // ========================================
            // 🎯 복잡한 중첩 구조에서의 통일된 API
            // ========================================
            
            beginArray("configs");
            {
                for (const auto& config : configs_) {
                    beginObject();  // 배열 내 객체 (key 없음)
                    {
                        // ✅ 객체 컨텍스트에서는 key 필수
                        setString("key", config.key);
                        setString("value", config.value);
                        setBool("enabled", config.enabled);
                        
                        // 중첩 배열 예제
                        beginArray("metadata");
                        {
                            // ✅ 배열 컨텍스트: key 무시됨
                            setString("", "created");
                            setString("", "2024-01-01");
                            setInt64("", 1704067200);  // timestamp
                        }
                        endArray();
                    }
                    endObject();
                }
            }
            endArray();
            
            // ========================================
            // 🎯 혼합 사용 패턴 - 완전히 유연함
            // ========================================
            
            beginArray("mixed_example");
            {
                // 기본 타입들
                setString("", "string_value");
                setInt64("", 42);
                setBool("", true);
                setDouble("", 3.14);
                
                // 중첩 객체
                beginObject();  // key 없음 (배열 요소)
                {
                    setString("type", "nested_object");
                    setInt64("level", 1);
                    
                    // 더 깊은 중첩 배열
                    beginArray("items");
                    {
                        setString("", "item1");
                        setString("", "item2");
                        
                        // 배열 안의 배열 안의 객체
                        beginObject();
                        {
                            setString("deep", "value");
                        }
                        endObject();
                    }
                    endArray();
                }
                endObject();
                
                // 중첩 배열
                beginArray();  // key 없음 (배열 요소)
                {
                    setString("", "nested_array_item1");
                    setString("", "nested_array_item2");
                }
                endArray();
            }
            endArray();
        }
        endObject();
    }
    
    // 🎯 예상 JSON 구조
    /*
    {
        "name": "Unified API Demo",
        "tags": ["json", "cpp", "library"],
        "scores": [95, 87, 92],
        "tags_alt": ["json", "cpp", "library"],     // pushXX로 생성 (동일)
        "scores_alt": [95, 87, 92],                // pushXX로 생성 (동일)
        "configs": [
            {
                "key": "debug",
                "value": "true", 
                "enabled": true,
                "metadata": ["created", "2024-01-01", 1704067200]
            },
            // ... 더 많은 config 객체들
        ],
        "mixed_example": [
            "string_value",
            42,
            true,
            3.14,
            {
                "type": "nested_object",
                "level": 1,
                "items": [
                    "item1",
                    "item2", 
                    {"deep": "value"}
                ]
            },
            ["nested_array_item1", "nested_array_item2"]
        ]
    }
    */
};

/**
 * 🚀 API 통일의 핵심 원칙:
 * 
 * 1. ✅ **컨텍스트 자동 인식**:
 *    - beginArray() → setXX의 key 매개변수 무시
 *    - beginObject() → setXX의 key 매개변수 사용
 *    - 개발자는 컨텍스트를 신경쓰지 않고 동일한 메서드 사용
 * 
 * 2. ✅ **일관된 메서드명**:
 *    - setString(), setInt64(), setBool() 등
 *    - 객체든 배열이든 동일한 메서드
 *    - 배열 전용 메서드처럼 보이지 않음
 * 
 * 3. ✅ **유연한 key 처리**:
 *    - 배열 컨텍스트: "", nullptr, "anything" 모두 무시됨
 *    - 객체 컨텍스트: key 필수 (nullptr이나 ""면 필드 생성 안됨)
 * 
 * 4. ✅ **선택적 편의 메서드**:
 *    - pushString(), pushInt64() 등은 배열 전용
 *    - 명시적으로 "배열에 추가"를 나타내고 싶을 때 사용
 *    - setXX("", value)와 pushXX(value)는 동일한 결과
 * 
 * 5. ✅ **자연스러운 중첩**:
 *    - begin/end 구조로 명확한 계층 표현
 *    - 임의의 깊이까지 중첩 가능
 *    - 각 레벨에서 동일한 API 사용
 * 
 * 📋 **사용 권장사항**:
 * 
 * - 일관성을 위해 setXX("", value) 방식 권장
 * - pushXX(value)는 "명시적 배열 추가" 의도가 명확할 때만
 * - 객체에서는 항상 의미있는 key 사용
 * - 배열에서는 key를 ""나 nullptr로 통일
 */ 