#pragma once

#include "../Jsonable.hpp"
#include <string>
#include <vector>
#include <cmath>

namespace test {

// 테스트용 Person 클래스
class Person : public json::Jsonable {
private:
    std::string name_;
    int age_;
    bool isStudent_;
    std::vector<std::string> hobbies_;

public:
    Person() : age_(0), isStudent_(false) {}
    Person(const std::string& name, int age, bool isStudent = false) 
        : name_(name), age_(age), isStudent_(isStudent) {}

    // Getters
    const std::string& getName() const { return name_; }
    int getAge() const { return age_; }
    bool getIsStudent() const { return isStudent_; }
    const std::vector<std::string>& getHobbies() const { return hobbies_; }

    // Setters
    void setName(const std::string& name) { name_ = name; }
    void setAge(int age) { age_ = age; }
    void setIsStudent(bool isStudent) { isStudent_ = isStudent; }
    void addHobby(const std::string& hobby) { hobbies_.push_back(hobby); }
    void setHobbies(const std::vector<std::string>& hobbies) { hobbies_ = hobbies; }

    // Jsonable 인터페이스 구현
    void fromJson(const std::string& jsonStr) override {
        auto doc = parseJson(jsonStr);
        fromDocument(doc);
    }

    std::string toJson() const override {
        rapidjson::Document doc;
        doc.SetObject();
        auto value = toValue(doc.GetAllocator());
        doc.CopyFrom(value, doc.GetAllocator());
        return valueToString(doc);
    }

    void fromDocument(const rapidjson::Value& value) override {
        if (!value.IsObject()) {
            throw std::runtime_error("Expected JSON object for Person");
        }

        name_ = getString(value, "name");
        age_ = static_cast<int>(getInt64(value, "age"));
        isStudent_ = getBool(value, "isStudent");

        hobbies_.clear();
        if (isArray(value, "hobbies")) {
            const auto& hobbiesArray = value["hobbies"];
            for (const auto& hobby : hobbiesArray.GetArray()) {
                if (hobby.IsString()) {
                    hobbies_.push_back(hobby.GetString());
                }
            }
        }
    }

    rapidjson::Value toValue(rapidjson::Document::AllocatorType& allocator) const override {
        rapidjson::Value person(rapidjson::kObjectType);
        
        // name
        rapidjson::Value nameValue;
        nameValue.SetString(name_.c_str(), static_cast<rapidjson::SizeType>(name_.length()), allocator);
        person.AddMember("name", nameValue, allocator);
        
        // age
        person.AddMember("age", age_, allocator);
        
        // isStudent
        person.AddMember("isStudent", isStudent_, allocator);
        
        // hobbies
        rapidjson::Value hobbiesArray(rapidjson::kArrayType);
        for (const auto& hobby : hobbies_) {
            rapidjson::Value hobbyValue;
            hobbyValue.SetString(hobby.c_str(), static_cast<rapidjson::SizeType>(hobby.length()), allocator);
            hobbiesArray.PushBack(hobbyValue, allocator);
        }
        person.AddMember("hobbies", hobbiesArray, allocator);
        
        return person;
    }

    // 비교 연산자 (테스트용)
    bool operator==(const Person& other) const {
        return name_ == other.name_ && 
               age_ == other.age_ && 
               isStudent_ == other.isStudent_ &&
               hobbies_ == other.hobbies_;
    }
};

// 테스트용 Car 클래스 (더 간단한 예제)
class Car : public json::Jsonable {
private:
    std::string brand_;
    std::string model_;
    int year_;
    double price_;

public:
    Car() : year_(0), price_(0.0) {}
    Car(const std::string& brand, const std::string& model, int year, double price)
        : brand_(brand), model_(model), year_(year), price_(price) {}

    // Getters
    const std::string& getBrand() const { return brand_; }
    const std::string& getModel() const { return model_; }
    int getYear() const { return year_; }
    double getPrice() const { return price_; }

    // Jsonable 인터페이스 구현
    void fromJson(const std::string& jsonStr) override {
        auto doc = parseJson(jsonStr);
        fromDocument(doc);
    }

    std::string toJson() const override {
        rapidjson::Document doc;
        doc.SetObject();
        auto value = toValue(doc.GetAllocator());
        doc.CopyFrom(value, doc.GetAllocator());
        return valueToString(doc);
    }

    void fromDocument(const rapidjson::Value& value) override {
        if (!value.IsObject()) {
            throw std::runtime_error("Expected JSON object for Car");
        }

        brand_ = getString(value, "brand");
        model_ = getString(value, "model");
        year_ = static_cast<int>(getInt64(value, "year"));
        
        // price는 double이므로 별도 처리
        if (value.HasMember("price") && value["price"].IsNumber()) {
            price_ = value["price"].GetDouble();
        } else {
            price_ = 0.0;
        }
    }

    rapidjson::Value toValue(rapidjson::Document::AllocatorType& allocator) const override {
        rapidjson::Value car(rapidjson::kObjectType);
        
        rapidjson::Value brandValue;
        brandValue.SetString(brand_.c_str(), static_cast<rapidjson::SizeType>(brand_.length()), allocator);
        car.AddMember("brand", brandValue, allocator);
        
        rapidjson::Value modelValue;
        modelValue.SetString(model_.c_str(), static_cast<rapidjson::SizeType>(model_.length()), allocator);
        car.AddMember("model", modelValue, allocator);
        
        car.AddMember("year", year_, allocator);
        car.AddMember("price", price_, allocator);
        
        return car;
    }

    // 비교 연산자 (테스트용)
    bool operator==(const Car& other) const {
        return brand_ == other.brand_ && 
               model_ == other.model_ && 
               year_ == other.year_ && 
               std::abs(price_ - other.price_) < 0.01; // 부동소수점 비교
    }
};

// 복잡한 중첩 구조 테스트용 - 부서 클래스
class Department : public json::Jsonable {
private:
    std::string name_;
    std::string manager_;
    int employeeCount_;
    double budget_;
    std::vector<std::string> projects_;

public:
    Department() : employeeCount_(0), budget_(0.0) {}
    Department(const std::string& name, const std::string& manager, int count, double budget)
        : name_(name), manager_(manager), employeeCount_(count), budget_(budget) {}

    // Getters
    const std::string& getName() const { return name_; }
    const std::string& getManager() const { return manager_; }
    int getEmployeeCount() const { return employeeCount_; }
    double getBudget() const { return budget_; }
    const std::vector<std::string>& getProjects() const { return projects_; }

    // Setters
    void addProject(const std::string& project) { projects_.push_back(project); }

    // Jsonable 인터페이스 구현
    void fromJson(const std::string& jsonStr) override {
        auto doc = parseJson(jsonStr);
        fromDocument(doc);
    }

    std::string toJson() const override {
        rapidjson::Document doc;
        doc.SetObject();
        auto value = toValue(doc.GetAllocator());
        doc.CopyFrom(value, doc.GetAllocator());
        return valueToString(doc);
    }

    void fromDocument(const rapidjson::Value& value) override {
        if (!value.IsObject()) {
            throw std::runtime_error("Expected JSON object for Department");
        }

        name_ = getString(value, "name");
        manager_ = getString(value, "manager");
        employeeCount_ = static_cast<int>(getInt64(value, "employeeCount"));
        budget_ = getDouble(value, "budget");

        projects_.clear();
        if (isArray(value, "projects")) {
            const auto& projectsArray = value["projects"];
            for (const auto& project : projectsArray.GetArray()) {
                if (project.IsString()) {
                    projects_.push_back(project.GetString());
                }
            }
        }
    }

    rapidjson::Value toValue(rapidjson::Document::AllocatorType& allocator) const override {
        rapidjson::Value dept(rapidjson::kObjectType);
        
        rapidjson::Value nameValue;
        nameValue.SetString(name_.c_str(), static_cast<rapidjson::SizeType>(name_.length()), allocator);
        dept.AddMember("name", nameValue, allocator);
        
        rapidjson::Value managerValue;
        managerValue.SetString(manager_.c_str(), static_cast<rapidjson::SizeType>(manager_.length()), allocator);
        dept.AddMember("manager", managerValue, allocator);
        
        dept.AddMember("employeeCount", employeeCount_, allocator);
        dept.AddMember("budget", budget_, allocator);
        
        rapidjson::Value projectsArray(rapidjson::kArrayType);
        for (const auto& project : projects_) {
            rapidjson::Value projectValue;
            projectValue.SetString(project.c_str(), static_cast<rapidjson::SizeType>(project.length()), allocator);
            projectsArray.PushBack(projectValue, allocator);
        }
        dept.AddMember("projects", projectsArray, allocator);
        
        return dept;
    }

    bool operator==(const Department& other) const {
        return name_ == other.name_ && 
               manager_ == other.manager_ && 
               employeeCount_ == other.employeeCount_ && 
               std::abs(budget_ - other.budget_) < 0.01 &&
               projects_ == other.projects_;
    }
};

// 최상위 복잡한 구조 - 회사 클래스
class Company : public json::Jsonable {
private:
    std::string name_;
    std::string address_;
    std::vector<Department> departments_;
    std::vector<Person> employees_;

public:
    Company() {}
    Company(const std::string& name, const std::string& address)
        : name_(name), address_(address) {}

    // Getters
    const std::string& getName() const { return name_; }
    const std::string& getAddress() const { return address_; }
    const std::vector<Department>& getDepartments() const { return departments_; }
    const std::vector<Person>& getEmployees() const { return employees_; }

    // Setters
    void setName(const std::string& name) { name_ = name; }
    void setAddress(const std::string& address) { address_ = address; }
    void addDepartment(const Department& dept) { departments_.push_back(dept); }
    void addEmployee(const Person& emp) { employees_.push_back(emp); }

    // Jsonable 인터페이스 구현
    void fromJson(const std::string& jsonStr) override {
        auto doc = parseJson(jsonStr);
        fromDocument(doc);
    }

    std::string toJson() const override {
        rapidjson::Document doc;
        doc.SetObject();
        auto value = toValue(doc.GetAllocator());
        doc.CopyFrom(value, doc.GetAllocator());
        return valueToString(doc);
    }

    void fromDocument(const rapidjson::Value& value) override {
        if (!value.IsObject()) {
            throw std::runtime_error("Expected JSON object for Company");
        }

        name_ = getString(value, "name");
        address_ = getString(value, "address");

        // 부서 배열 처리
        departments_.clear();
        if (isArray(value, "departments")) {
            const auto& deptsArray = value["departments"];
            for (const auto& deptValue : deptsArray.GetArray()) {
                Department dept;
                dept.fromDocument(deptValue);
                departments_.push_back(dept);
            }
        }

        // 직원 배열 처리
        employees_.clear();
        if (isArray(value, "employees")) {
            const auto& empsArray = value["employees"];
            for (const auto& empValue : empsArray.GetArray()) {
                Person emp;
                emp.fromDocument(empValue);
                employees_.push_back(emp);
            }
        }
    }

    rapidjson::Value toValue(rapidjson::Document::AllocatorType& allocator) const override {
        rapidjson::Value company(rapidjson::kObjectType);
        
        rapidjson::Value nameValue;
        nameValue.SetString(name_.c_str(), static_cast<rapidjson::SizeType>(name_.length()), allocator);
        company.AddMember("name", nameValue, allocator);
        
        rapidjson::Value addressValue;
        addressValue.SetString(address_.c_str(), static_cast<rapidjson::SizeType>(address_.length()), allocator);
        company.AddMember("address", addressValue, allocator);
        
        // 부서 배열
        rapidjson::Value deptsArray(rapidjson::kArrayType);
        for (const auto& dept : departments_) {
            deptsArray.PushBack(dept.toValue(allocator), allocator);
        }
        company.AddMember("departments", deptsArray, allocator);
        
        // 직원 배열
        rapidjson::Value empsArray(rapidjson::kArrayType);
        for (const auto& emp : employees_) {
            empsArray.PushBack(emp.toValue(allocator), allocator);
        }
        company.AddMember("employees", empsArray, allocator);
        
        return company;
    }

    bool operator==(const Company& other) const {
        return name_ == other.name_ && 
               address_ == other.address_ && 
               departments_ == other.departments_ &&
               employees_ == other.employees_;
    }
};

// ========================================
// 매크로를 사용한 간단한 사용법 예제
// ========================================

// 간단한 클래스 - 매크로로 완전 자동화
class SimpleProduct : public json::Jsonable {
private:
    std::string name_;
    int id_;
    double price_;
    bool available_;

public:
    SimpleProduct() : id_(0), price_(0.0), available_(false) {}
    SimpleProduct(const std::string& name, int id, double price, bool available)
        : name_(name), id_(id), price_(price), available_(available) {}

    // Getters
    const std::string& getName() const { return name_; }
    int getId() const { return id_; }
    double getPrice() const { return price_; }
    bool isAvailable() const { return available_; }

    // Setters  
    void setName(const std::string& name) { name_ = name; }
    void setId(int id) { id_ = id; }
    void setPrice(double price) { price_ = price; }
    void setAvailable(bool available) { available_ = available; }

    // 매크로로 직렬화 구현 자동 생성 (실험적)
    // JSONABLE_AUTO_IMPL(SimpleProduct, 
    //     string, name, "name",
    //     int, id, "id", 
    //     double, price, "price",
    //     bool, available, "available"
    // )

    // 현재는 수동 구현 (위 매크로는 복잡하므로 일단 주석)
    void fromDocument(const rapidjson::Value& value) override {
        if (!value.IsObject()) {
            throw std::runtime_error("Expected JSON object for SimpleProduct");
        }

        JSON_FIELD_STRING(value, name_, "name");
        if (value.HasMember("id") && value["id"].IsInt()) {
            id_ = value["id"].GetInt();
        }
        JSON_FIELD_DOUBLE(value, price_, "price");
        JSON_FIELD_BOOL(value, available_, "available");
    }

    rapidjson::Value toValue(rapidjson::Document::AllocatorType& allocator) const override {
        rapidjson::Value product(rapidjson::kObjectType);
        
        JSON_SET_STRING(product, "name", name_, allocator);
        JSON_SET_PRIMITIVE(product, "id", id_, allocator);
        JSON_SET_PRIMITIVE(product, "price", price_, allocator);
        JSON_SET_PRIMITIVE(product, "available", available_, allocator);
        
        return product;
    }

public:
    // 기본 구현은 매크로로
    JSONABLE_IMPL()

    // 비교 연산자
    bool operator==(const SimpleProduct& other) const {
        return name_ == other.name_ && 
               id_ == other.id_ && 
               std::abs(price_ - other.price_) < 0.01 &&
               available_ == other.available_;
    }
};

// 상속 없이도 사용할 수 있는 비침입적(Non-intrusive) 버전 예제
class NonIntrusiveData {
private:
    std::string title_;
    int count_;

public:
    NonIntrusiveData() : count_(0) {}
    NonIntrusiveData(const std::string& title, int count) : title_(title), count_(count) {}

    const std::string& getTitle() const { return title_; }
    int getCount() const { return count_; }
    void setTitle(const std::string& title) { title_ = title; }
    void setCount(int count) { count_ = count; }

    // JSON 직렬화를 위한 정적 메서드들 (상속 없이)
    static NonIntrusiveData fromJson(const std::string& jsonStr) {
        auto doc = json::Jsonable::parseJson(jsonStr);
        NonIntrusiveData data;
        data.title_ = json::Jsonable::getString(doc, "title");
        data.count_ = static_cast<int>(json::Jsonable::getInt64(doc, "count"));
        return data;
    }

    std::string toJson() const {
        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        rapidjson::Value titleVal(title_.c_str(), allocator);
        doc.AddMember("title", titleVal, allocator);
        doc.AddMember("count", count_, allocator);

        return json::Jsonable::valueToString(doc);
    }

    bool operator==(const NonIntrusiveData& other) const {
        return title_ == other.title_ && count_ == other.count_;
    }
};

} // namespace test 