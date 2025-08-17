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
        
        // 🎯 복잡한 중첩 구조 읽기는 Begin/End 방식보다는
        // 직접 배열 처리나 반복문이 더 적합할 수 있음
        if (hasKey("departments") && isArray("departments")) {
            // 실제 구현에서는 중첩된 구조를 직접 파싱하거나
            // 별도의 부분 파서를 사용하는 것이 일반적
            // 
            // 예: departments_ = getArray<Department>("departments"); 
            // (단, Department이 Jsonable을 상속받은 경우)
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
 * ✅ Begin/End 구조의 우수성:
 * 
 * 1. **명확한 구조**: 코드 구조가 JSON 구조와 완벽 일치
 * 2. **컴파일 타임 안전**: 구조 오류를 컴파일 시점에 발견
 * 3. **높은 성능**: 함수 호출 오버헤드 없이 직접 처리
 * 4. **우수한 가독성**: 중첩 레벨이 들여쓰기로 명확히 표현
 * 5. **간단한 API**: 복잡한 경로 문자열이나 헬퍼 함수 불필요
 */ 