#pragma once

/**
 * 다중상속 구조 사용 예제
 * 
 * JsonableImpl ← FromJsonable ← 
 *                             ↘
 *                              Jsonable (다중상속)
 *                             ↗
 * JsonableImpl ← ToJsonable   ← 
 */

#define JSONABLE_IMPLEMENTATION
#include "../Jsonable.hpp"

/**
 * @brief 간단한 사용자 클래스 예제
 * 
 * 사용자는 json::Jsonable만 상속받으면 됨
 * - ToJsonable과 FromJsonable 기능을 모두 자동으로 획득
 * - JsonableImpl의 모든 헬퍼 함수들도 사용 가능
 */
class User : public json::Jsonable {
private:
    std::string username_;
    std::string email_;
    int age_;
    bool isActive_;
    std::vector<std::string> permissions_;

public:
    // 생성자들
    User() = default;
    User(const std::string& username, const std::string& email, int age) 
        : username_(username), email_(email), age_(age), isActive_(true) {}
    
    // JSON 문자열로부터 생성 (Jsonable에서 제공)
    User(const std::string& jsonStr) : json::Jsonable(jsonStr) {}

    // ========================================
    // FromJsonable에서 상속받은 순수 가상 함수 구현
    // ========================================
    
    void loadFromJson() override {
        // 📖 기본 필드 로딩
        username_ = getString("username");
        email_ = getString("email");
        age_ = static_cast<int>(getInt64("age"));
        isActive_ = getBool("isActive", true);  // 기본값 true
        
        // 📖 배열 필드 로딩
        permissions_ = getArray<std::string>("permissions");
        
        // 🔍 고급 로딩 유틸리티 사용 (FromJsonable에서 제공)
        // loadField("age", age_, [](int age) { return age >= 0 && age <= 150; });
        // loadArrayField("permissions", permissions_, 10);  // 최대 10개
    }

    // ========================================
    // ToJsonable에서 상속받은 순수 가상 함수 구현
    // ========================================
    
    void saveToJson() override {
        // 💾 기본 필드 저장
        setString("username", username_);
        setString("email", email_);
        setInt64("age", static_cast<int64_t>(age_));
        setBool("isActive", isActive_);
        
        // 💾 배열 필드 저장
        setArray("permissions", permissions_);
        
        // 🎯 고급 직렬화 유틸리티 사용 (ToJsonable에서 제공)
        // saveFieldIf("email", email_, !email_.empty());  // 조건부 저장
        // saveArrayField("permissions", permissions_, 
        //               [](const std::string& perm) { return !perm.empty(); }); // 필터링
    }

    // 편의 메서드들
    void addPermission(const std::string& permission) {
        permissions_.push_back(permission);
    }
    
    bool hasPermission(const std::string& permission) const {
        return std::find(permissions_.begin(), permissions_.end(), permission) 
               != permissions_.end();
    }
    
    // Getter들
    const std::string& getUsername() const { return username_; }
    const std::string& getEmail() const { return email_; }
    int getAge() const { return age_; }
    bool isActive() const { return isActive_; }
    const std::vector<std::string>& getPermissions() const { return permissions_; }
};

/**
 * @brief 복잡한 중첩 구조 예제 - Begin/End 스타일 사용
 */
class Organization : public json::Jsonable {
private:
    std::string name_;
    std::string address_;
    std::vector<User> members_;
    struct Department {
        std::string name;
        std::string manager;
        std::vector<std::string> projects;
    };
    std::vector<Department> departments_;

public:
    Organization() = default;
    Organization(const std::string& name) : name_(name) {}

    void addMember(const User& user) {
        members_.push_back(user);
    }
    
    void addDepartment(const std::string& name, const std::string& manager, 
                      const std::vector<std::string>& projects) {
        Department dept;
        dept.name = name;
        dept.manager = manager;
        dept.projects = projects;
        departments_.push_back(dept);
    }

    // ========================================
    // FromJsonable 구현 - 복잡한 중첩 구조 읽기
    // ========================================
    
    void loadFromJson() override {
        name_ = getString("name");
        address_ = getString("address");
        
        // 💡 멤버 배열 로딩 (중첩 객체)
        members_.clear();
        if (hasKey("members") && isArray("members")) {
            iterateArray("members", [this](size_t index) {
                // 각 멤버를 User 객체로 로딩
                // 실제 구현에서는 중첩 객체 접근 방법 필요
                User user;
                // user.fromJson(...); // 중첩 객체 처리
                members_.push_back(user);
            });
        }
        
        // 💡 부서 배열 로딩
        departments_.clear();
        // 비슷한 방식으로 부서 정보 로딩...
    }

    // ========================================
    // ToJsonable 구현 - Begin/End 스타일로 중첩 구조 생성
    // ========================================
    
    void saveToJson() override {
        beginObject();  // 루트 객체 시작
        {
            setString("name", name_);
            setString("address", address_);
            
            // 🎯 멤버 배열 생성
            beginArray("members");
            {
                for (const auto& member : members_) {
                    beginObject();  // 멤버 객체 시작
                    {
                        setString("username", member.getUsername());
                        setString("email", member.getEmail());
                        setInt64("age", static_cast<int64_t>(member.getAge()));
                        setBool("isActive", member.isActive());
                        
                        beginArray("permissions");
                        {
                            for (const auto& perm : member.getPermissions()) {
                                setString("", perm);  // 배열 요소 (key 무시됨)
                            }
                        }
                        endArray();
                    }
                    endObject();  // 멤버 객체 종료
                }
            }
            endArray();  // 멤버 배열 종료
            
            // 🎯 부서 배열 생성
            beginArray("departments");
            {
                for (const auto& dept : departments_) {
                    beginObject();  // 부서 객체 시작
                    {
                        setString("name", dept.name);
                        setString("manager", dept.manager);
                        
                        beginArray("projects");
                        {
                            for (const auto& project : dept.projects) {
                                setString("", project);  // 배열 요소
                            }
                        }
                        endArray();
                    }
                    endObject();  // 부서 객체 종료
                }
            }
            endArray();  // 부서 배열 종료
        }
        endObject();  // 루트 객체 종료
    }

    // Getter들
    const std::string& getName() const { return name_; }
    const std::vector<User>& getMembers() const { return members_; }
    const std::vector<Department>& getDepartments() const { return departments_; }
};

/**
 * 🎯 생성되는 JSON 구조:
 * 
 * {
 *   "name": "Tech Corp",
 *   "address": "123 Tech Street",
 *   "members": [
 *     {
 *       "username": "alice",
 *       "email": "alice@example.com",
 *       "age": 25,
 *       "isActive": true,
 *       "permissions": ["read", "write", "admin"]
 *     }
 *   ],
 *   "departments": [
 *     {
 *       "name": "Engineering",
 *       "manager": "Bob Smith",
 *       "projects": ["Project A", "Project B"]
 *     }
 *   ]
 * }
 */

/**
 * 🚀 다중상속 구조의 장점:
 * 
 * 1. ✅ **명확한 책임 분리**:
 *    - JsonableImpl: 기본 JSON 조작
 *    - FromJsonable: 역직렬화 책임
 *    - ToJsonable: 직렬화 책임
 *    - Jsonable: 통합 인터페이스
 * 
 * 2. ✅ **단순한 사용자 인터페이스**:
 *    - 사용자는 json::Jsonable만 상속
 *    - loadFromJson()과 saveToJson()만 구현
 * 
 * 3. ✅ **확장성**:
 *    - 각 레벨에서 고유한 기능 제공
 *    - FromJsonable: loadField(), loadArrayField()
 *    - ToJsonable: saveFieldIf(), saveArrayField()
 * 
 * 4. ✅ **virtual 상속으로 다이아몬드 문제 해결**:
 *    - JsonableImpl이 한 번만 상속됨
 *    - 메모리 효율성과 명확성 확보
 * 
 * 5. ✅ **타입 안전성 유지**:
 *    - 컴파일 타임 타입 검증
 *    - is_jsonable_v, is_json_primitive_v 지원
 */ 