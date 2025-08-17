#pragma once

/**
 * Begin/End 구조로 중첩 처리 예제
 * 
 * nested XXX 인터페이스 없이도 복잡한 중첩 구조를 쉽게 처리
 */

#include "../Jsonable.hpp"

class NestedStructureExample : public json::Jsonable {
private:
    std::string title_;
    
    struct Department {
        std::string name;
        std::vector<std::string> members;
        struct Budget {
            double allocated;
            double spent;
            std::vector<std::string> categories;
        } budget;
    };
    
    std::vector<Department> departments_;
    
public:
    NestedStructureExample(const std::string& title) : title_(title) {}
    
    void addDepartment(const std::string& name, const std::vector<std::string>& members,
                      double allocated, double spent, const std::vector<std::string>& categories) {
        Department dept;
        dept.name = name;
        dept.members = members;
        dept.budget.allocated = allocated;
        dept.budget.spent = spent;
        dept.budget.categories = categories;
        departments_.push_back(dept);
    }
    
    void saveToJson() override {
        beginObject();  // 루트 객체
        {
            setString("title", title_);
            
            beginArray("departments");  // 부서 배열
            {
                for (const auto& dept : departments_) {
                    beginObject();  // 부서 객체
                    {
                        setString("name", dept.name);
                        
                        beginArray("members");  // 멤버 배열
                        {
                            for (const auto& member : dept.members) {
                                setString("", member);  // 배열 요소
                            }
                        }
                        endArray();
                        
                        beginObject("budget");  // 예산 객체 (중첩)
                        {
                            setDouble("allocated", dept.budget.allocated);
                            setDouble("spent", dept.budget.spent);
                            
                            beginArray("categories");  // 카테고리 배열 (깊은 중첩)
                            {
                                for (const auto& category : dept.budget.categories) {
                                    setString("", category);  // 배열 요소
                                }
                            }
                            endArray();
                        }
                        endObject();  // 예산 객체 종료
                    }
                    endObject();  // 부서 객체 종료
                }
            }
            endArray();  // 부서 배열 종료
        }
        endObject();  // 루트 객체 종료
    }
    
    void loadFromJson() override {
        title_ = getString("title");
        
        departments_.clear();
        
        // 🎯 Begin/End 구조 없이는 이런 식으로 읽어야 함
        // 하지만 읽기는 기존 방식이 더 편할 수 있음
        if (hasKey("departments") && isArray("departments")) {
            iterateArray("departments", [this](size_t deptIndex) {
                Department dept;
                
                // 부서 이름과 멤버들은 기존 방식으로
                std::string deptKey = "departments/" + std::to_string(deptIndex);
                // dept.name = getNestedString(deptKey + "/name");  // ❌ 제거된 nested API
                
                // 대신 임시 JSON 객체를 만들어서 처리하거나
                // 또는 직접 RapidJSON 순회 (하지만 이는 내부 구현에서 처리)
                
                departments_.push_back(dept);
            });
        }
    }
};

/**
 * 🎯 실제 생성되는 JSON 구조:
 * 
 * {
 *   "title": "Company Structure",
 *   "departments": [
 *     {
 *       "name": "Engineering",
 *       "members": ["Alice", "Bob", "Charlie"],
 *       "budget": {
 *         "allocated": 100000.0,
 *         "spent": 75000.0,
 *         "categories": ["Hardware", "Software", "Training"]
 *       }
 *     },
 *     {
 *       "name": "Marketing", 
 *       "members": ["David", "Eve"],
 *       "budget": {
 *         "allocated": 50000.0,
 *         "spent": 30000.0,
 *         "categories": ["Advertising", "Events"]
 *       }
 *     }
 *   ]
 * }
 */

/**
 * ✅ Begin/End 구조의 장점:
 * 
 * 1. **직관적 중첩**: 코드 구조가 JSON 구조와 일치
 * 2. **타입 안전**: 컴파일 타임에 구조 검증
 * 3. **가독성**: 중첩 레벨이 코드 들여쓰기로 명확
 * 4. **유지보수**: 구조 변경이 쉬움
 * 5. **path 불필요**: "a/b/c" 같은 문자열 경로 불필요
 * 
 * ❌ nested XXX 인터페이스의 문제점:
 * 
 * 1. **문자열 경로**: "departments/0/budget/allocated" 같은 복잡한 문자열
 * 2. **런타임 오류**: 경로 오타 시 런타임에만 발견
 * 3. **타입 불안전**: 경로의 타입을 컴파일 타임에 알 수 없음
 * 4. **성능 오버헤드**: 매번 경로 파싱 필요
 * 5. **가독성 저하**: 복잡한 경로 문자열
 */ 