#pragma once

/**
 * BasicUsageExample.hpp - 기본 사용법 예제
 * 
 * 새로운 다중상속 구조를 사용한 기본적인 JSON 처리 예제
 */

#include "../Jsonable.hpp"

#include <iostream>
#include <vector>
#include <string>

/**
 * @brief 기본 Person 클래스 예제
 * 
 * json::Jsonable을 상속받아 JSON 직렬화/역직렬화 기능 획득
 */
class Person : public json::Jsonable {
private:
    std::string name_;
    int age_;
    std::string email_;
    std::vector<std::string> hobbies_;
    bool isActive_;

public:
    // 생성자들
    Person() : age_(0), isActive_(true) {}
    
    Person(const std::string& name, int age, const std::string& email)
        : name_(name), age_(age), email_(email), isActive_(true) {}

    // ========================================
    // FromJsonable에서 상속받은 필수 구현
    // ========================================
    
    void loadFromJson() override {
        // 기본 필드 로딩
        name_ = getString("name", "Unknown");
        age_ = static_cast<int>(getInt64("age", 0));
        email_ = getString("email", "");
        isActive_ = getBool("isActive", true);
        
        // 배열 필드 로딩
        hobbies_ = getArray<std::string>("hobbies");
    }

    // ========================================
    // ToJsonable에서 상속받은 필수 구현
    // ========================================
    
    void saveToJson() override {
        // 방법 1: 직접 필드 설정 (간단한 구조)
        setString("name", name_);
        setInt64("age", static_cast<int64_t>(age_));
        setString("email", email_);
        setBool("isActive", isActive_);
        setArray("hobbies", hobbies_);
    }

    // ========================================
    // Getter/Setter 메서드들
    // ========================================
    
    void setName(const std::string& name) { name_ = name; }
    void setAge(int age) { age_ = age; }
    void setEmail(const std::string& email) { email_ = email; }
    void setActive(bool active) { isActive_ = active; }
    void addHobby(const std::string& hobby) { hobbies_.push_back(hobby); }
    void clearHobbies() { hobbies_.clear(); }
    
    const std::string& getName() const { return name_; }
    int getAge() const { return age_; }
    const std::string& getEmail() const { return email_; }
    bool isActive() const { return isActive_; }
    const std::vector<std::string>& getHobbies() const { return hobbies_; }

    // ========================================
    // 편의 메서드들 (Jsonable에서 제공)
    // ========================================
    
    void print() const {
        std::cout << "Person: " << name_ << " (age: " << age_ << ")" << std::endl;
        std::cout << "Email: " << email_ << std::endl;
        std::cout << "Active: " << (isActive_ ? "Yes" : "No") << std::endl;
        std::cout << "Hobbies: ";
        for (size_t i = 0; i < hobbies_.size(); ++i) {
            std::cout << hobbies_[i];
            if (i < hobbies_.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
};

/**
 * @brief Begin/End 스타일을 사용하는 복잡한 구조 예제
 */
class Company : public json::Jsonable {
private:
    std::string companyName_;
    std::string address_;
    std::vector<Person> employees_;
    std::vector<std::string> departments_;
    
    struct ContactInfo {
        std::string phone;
        std::string fax;
        std::string website;
    } contact_;

public:
    Company() = default;
    Company(const std::string& name) : companyName_(name) {}

    // ========================================
    // FromJsonable 구현 - 복잡한 구조 로딩
    // ========================================
    
    void loadFromJson() override {
        companyName_ = getString("name");
        address_ = getString("address");
        departments_ = getArray<std::string>("departments");
        
        // 연락처 정보 로딩
        contact_.phone = getString("contact.phone", "");
        contact_.fax = getString("contact.fax", "");
        contact_.website = getString("contact.website", "");
        
        // 직원 배열은 간단한 경우만 처리 (실제로는 더 복잡)
        employees_.clear();
        // 실제 구현에서는 iterateArray와 중첩 객체 처리 필요
    }

    // ========================================
    // ToJsonable 구현 - Begin/End 스타일
    // ========================================
    
    void saveToJson() override {
        beginObject();  // 루트 객체 시작
        {
            // 기본 회사 정보
            setString("name", companyName_);
            setString("address", address_);
            
            // 부서 배열
            beginArray("departments");
            {
                for (const auto& dept : departments_) {
                    setString("", dept);  // 배열에서 key 무시됨
                }
            }
            endArray();
            
            // 연락처 중첩 객체
            beginObject("contact");
            {
                setString("phone", contact_.phone);
                setString("fax", contact_.fax);
                setString("website", contact_.website);
            }
            endObject();
            
            // 직원 객체 배열
            beginArray("employees");
            {
                for (const auto& employee : employees_) {
                    beginObject();  // 직원 객체 시작
                    {
                        setString("name", employee.getName());
                        setInt64("age", static_cast<int64_t>(employee.getAge()));
                        setString("email", employee.getEmail());
                        setBool("isActive", employee.isActive());
                        
                        // 취미 배열
                        beginArray("hobbies");
                        {
                            for (const auto& hobby : employee.getHobbies()) {
                                setString("", hobby);  // key 무시됨
                            }
                        }
                        endArray();
                    }
                    endObject();  // 직원 객체 종료
                }
            }
            endArray();
        }
        endObject();  // 루트 객체 종료
    }

    // ========================================
    // 비즈니스 로직 메서드들
    // ========================================
    
    void setCompanyName(const std::string& name) { companyName_ = name; }
    void setAddress(const std::string& address) { address_ = address; }
    void setContactPhone(const std::string& phone) { contact_.phone = phone; }
    void setContactWebsite(const std::string& website) { contact_.website = website; }
    
    void addEmployee(const Person& person) {
        employees_.push_back(person);
    }
    
    void addDepartment(const std::string& dept) {
        departments_.push_back(dept);
    }
    
    const std::string& getCompanyName() const { return companyName_; }
    const std::vector<Person>& getEmployees() const { return employees_; }
    const std::vector<std::string>& getDepartments() const { return departments_; }
    
    void print() const {
        std::cout << "\n=== Company: " << companyName_ << " ===" << std::endl;
        std::cout << "Address: " << address_ << std::endl;
        std::cout << "Phone: " << contact_.phone << std::endl;
        std::cout << "Website: " << contact_.website << std::endl;
        
        std::cout << "\nDepartments:" << std::endl;
        for (const auto& dept : departments_) {
            std::cout << "  - " << dept << std::endl;
        }
        
        std::cout << "\nEmployees (" << employees_.size() << "):" << std::endl;
        for (const auto& emp : employees_) {
            std::cout << "  ";
            emp.print();
        }
    }
};

/**
 * 🧪 사용 예제 및 테스트 함수들
 */
namespace BasicUsageExamples {

    /**
     * @brief 기본 Person 사용 예제
     */
    void testBasicPersonUsage() {
        std::cout << "\n=== Basic Person Usage Test ===" << std::endl;
        
        // 1. 객체 생성 및 데이터 설정
        Person person("Alice Johnson", 28, "alice@example.com");
        person.addHobby("reading");
        person.addHobby("coding");
        person.addHobby("hiking");
        
        std::cout << "Original Person:" << std::endl;
        person.print();
        
        // 2. JSON 직렬화
        std::string jsonStr = person.toJson();
        std::cout << "\nSerialized JSON:" << std::endl;
        std::cout << jsonStr << std::endl;
        
        // 3. JSON 역직렬화
        Person restoredPerson;
        restoredPerson.fromJson(jsonStr);
        
        std::cout << "\nRestored Person:" << std::endl;
        restoredPerson.print();
        
        // 4. 동등성 확인 (Jsonable에서 제공)
        bool isEqual = person.equals(restoredPerson);
        std::cout << "\nObjects are equal: " << (isEqual ? "Yes" : "No") << std::endl;
    }

    /**
     * @brief JSON 문자열에서 직접 생성 예제
     */
    void testJsonStringCreation() {
        std::cout << "\n=== JSON String Creation Test ===" << std::endl;
        
        std::string jsonData = R"({
            "name": "Bob Smith",
            "age": 35,
            "email": "bob@company.com",
            "isActive": true,
            "hobbies": ["photography", "traveling", "cooking"]
        })";
        
        // JSON 문자열에서 직접 생성 (편의 생성자 사용)
        Person person(jsonData);
        
        std::cout << "Person created from JSON:" << std::endl;
        person.print();
        
        // 데이터 수정
        person.setAge(36);
        person.addHobby("gardening");
        
        std::cout << "\nAfter modifications:" << std::endl;
        person.print();
        
        std::cout << "\nUpdated JSON:" << std::endl;
        std::cout << person.toJson() << std::endl;
    }

    /**
     * @brief 복잡한 Company 구조 예제
     */
    void testComplexCompanyStructure() {
        std::cout << "\n=== Complex Company Structure Test ===" << std::endl;
        
        // 회사 생성
        Company company("Tech Innovations Inc.");
        company.setAddress("123 Innovation Drive, Tech City");
        company.setContactPhone("+1-555-TECH");
        company.setContactWebsite("https://techinnovations.com");
        
        // 부서 추가
        company.addDepartment("Engineering");
        company.addDepartment("Marketing");
        company.addDepartment("Sales");
        
        // 직원 추가
        Person emp1("Alice Johnson", 28, "alice@tech.com");
        emp1.addHobby("coding");
        emp1.addHobby("reading");
        company.addEmployee(emp1);
        
        Person emp2("Bob Smith", 32, "bob@tech.com");
        emp2.addHobby("photography");
        emp2.addHobby("traveling");
        company.addEmployee(emp2);
        
        // 회사 정보 출력
        company.print();
        
        // JSON 직렬화
        std::string companyJson = company.toJson();
        std::cout << "\nCompany JSON (first 200 chars):" << std::endl;
        std::cout << companyJson.substr(0, 200) << "..." << std::endl;
        
        // 파일 크기 정보
        std::cout << "\nJSON size: " << companyJson.length() << " characters" << std::endl;
    }

    /**
     * @brief 모든 예제 실행
     */
    void runAllExamples() {
        std::cout << "🚀 Running Basic Usage Examples..." << std::endl;
        
        testBasicPersonUsage();
        testJsonStringCreation();
        testComplexCompanyStructure();
        
        std::cout << "\n✅ All basic usage examples completed!" << std::endl;
    }

} // namespace BasicUsageExamples

/**
 * 🎯 예상 JSON 출력 예시:
 * 
 * Person JSON:
 * {
 *   "name": "Alice Johnson",
 *   "age": 28,
 *   "email": "alice@example.com",
 *   "isActive": true,
 *   "hobbies": ["reading", "coding", "hiking"]
 * }
 * 
 * Company JSON:
 * {
 *   "name": "Tech Innovations Inc.",
 *   "address": "123 Innovation Drive, Tech City",
 *   "departments": ["Engineering", "Marketing", "Sales"],
 *   "contact": {
 *     "phone": "+1-555-TECH",
 *     "fax": "",
 *     "website": "https://techinnovations.com"
 *   },
 *   "employees": [
 *     {
 *       "name": "Alice Johnson",
 *       "age": 28,
 *       "email": "alice@tech.com",
 *       "isActive": true,
 *       "hobbies": ["coding", "reading"]
 *     },
 *     // ... 더 많은 직원들
 *   ]
 * }
 */ 