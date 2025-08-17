# Jsonable - 타입 안전한 C++ JSON 라이브러리

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![RapidJSON](https://img.shields.io/badge/RapidJSON-Hidden-green.svg)](https://rapidjson.org/)

**완전한 RapidJSON 의존성 숨김과 다중상속 기반의 깔끔한 JSON 처리 라이브러리**

## 🎯 핵심 특징

### ✅ **완벽한 의존성 숨김 (SOLID DIP 준수)**
- 🔒 **RapidJSON 100% 숨김**: 사용자는 RapidJSON 헤더를 볼 필요 없음
- 📦 **단일 헤더**: `Jsonable.hpp` 하나만 include
- 🛡️ **PIMPL 패턴**: 구현 세부사항 완전 은닉

### ✅ **명확한 다중상속 구조**
```cpp
JsonableBase (기본 구현)
   ↑ virtual    ↑ virtual  
   |            |
FromJsonable   ToJsonable
(역직렬화)     (직렬화)
   ↑            ↑
   |            |
   └────────────┴──→ Jsonable (사용자 인터페이스)
```

### ✅ **타입 안전성 보장**
- 🧪 **컴파일 타임 검증**: `static_assert`로 타입 오류 방지
- 🚫 **Mixed Type 배열 금지**: 타입 안전성 위배 패턴 제거
- 📊 **단일 타입 배열만 지원**: `std::vector<T>` 형태만 허용

### ✅ **통일된 API**
- 🎯 **컨텍스트 자동 인식**: 배열/객체에서 동일한 `setXX` 메서드 사용
- 🔄 **Begin/End 스타일**: 직관적인 중첩 구조 생성
- 🛡️ **Null 안전성**: `nullptr`, 빈 문자열 key 안전하게 처리

## 🚀 빠른 시작

### 1. 기본 사용법

```cpp
#define JSONABLE_IMPLEMENTATION  // 한 번만 정의
#include "Jsonable.hpp"

class Person : public json::Jsonable {
private:
    std::string name_;
    int age_;
    std::vector<std::string> hobbies_;

public:
    // FromJsonable에서 상속받은 순수 가상 함수
    void loadFromJson() override {
        name_ = getString("name");
        age_ = static_cast<int>(getInt64("age"));
        hobbies_ = getArray<std::string>("hobbies");
    }

    // ToJsonable에서 상속받은 순수 가상 함수
    void saveToJson() override {
        setString("name", name_);
        setInt64("age", static_cast<int64_t>(age_));
        setArray("hobbies", hobbies_);
    }

    // Getter/Setter들
    void setName(const std::string& name) { name_ = name; }
    void setAge(int age) { age_ = age; }
    void addHobby(const std::string& hobby) { hobbies_.push_back(hobby); }
    
    const std::string& getName() const { return name_; }
    int getAge() const { return age_; }
    const std::vector<std::string>& getHobbies() const { return hobbies_; }
};

// 사용 예시
int main() {
    // JSON 문자열에서 객체 생성
    Person person;
    person.fromJson(R"({"name":"Alice","age":25,"hobbies":["reading","coding"]})");
    
    std::cout << "Name: " << person.getName() << std::endl;
    std::cout << "Age: " << person.getAge() << std::endl;
    
    // 객체에서 JSON 문자열 생성
    person.setName("Bob");
    person.setAge(30);
    person.addHobby("gaming");
    
    std::string json = person.toJson();
    std::cout << "JSON: " << json << std::endl;
    
    return 0;
}
```

### 2. Begin/End 스타일 (복잡한 중첩 구조)

```cpp
class Company : public json::Jsonable {
private:
    std::string name_;
    std::vector<Person> employees_;
    std::vector<std::string> departments_;

public:
    void saveToJson() override {
        beginObject();  // 루트 객체 시작
        {
            setString("name", name_);
            
            // 부서 배열
            beginArray("departments");
            {
                for (const auto& dept : departments_) {
                    setString("", dept);  // 배열에서 key 무시됨
                }
            }
            endArray();
            
            // 직원 객체 배열
            beginArray("employees");
            {
                for (const auto& emp : employees_) {
                    beginObject();  // 직원 객체 시작
                    {
                        setString("name", emp.getName());
                        setInt64("age", static_cast<int64_t>(emp.getAge()));
                        
                        beginArray("hobbies");
                        {
                            for (const auto& hobby : emp.getHobbies()) {
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

    void loadFromJson() override {
        name_ = getString("name");
        departments_ = getArray<std::string>("departments");
        
        // 직원 배열은 수동 로딩 (복잡한 중첩 객체)
        employees_.clear();
        if (hasKey("employees") && isArray("employees")) {
            iterateArray("employees", [this](size_t index) {
                // 실제 구현에서는 중첩 객체 접근 방법 필요
                Person emp;
                // emp 로딩 로직...
                employees_.push_back(emp);
            });
        }
    }
};
```

## 📚 고급 기능

### 🎯 메타프로그래밍 지원

```cpp
class Config : public json::Jsonable {
    void saveToJson() override {
        // 자동 타입 판별
        setField("debug", true);           // bool
        setField("timeout", 30000);        // int
        setField("version", 1.2);          // double
        setField("name", std::string("MyApp")); // string
    }
    
    void loadFromJson() override {
        bool debug = getField<bool>("debug");
        int timeout = getField<int>("timeout");
        double version = getField<double>("version");
        std::string name = getField<std::string>("name");
    }
};
```

### 🔍 조건부 필드 처리

```cpp
class User : public json::Jsonable {
    void saveToJson() override {
        setString("username", username_);
        
        // 조건부 저장 (ToJsonable에서 제공)
        saveFieldIf("email", email_, !email_.empty());
        saveFieldIf("age", age_, age_ > 0);
        
        // 필터링된 배열 저장
        saveArrayField("permissions", permissions_, 
                      [](const std::string& perm) { 
                          return !perm.empty(); 
                      });
    }
    
    void loadFromJson() override {
        username_ = getString("username");
        
        // 안전한 로딩 (FromJsonable에서 제공)
        loadField("age", age_, [](int age) { 
            return age >= 0 && age <= 150; 
        });
        
        loadArrayField("permissions", permissions_, 10); // 최대 10개
    }
};
```

### 🛡️ Optional 타입 지원

```cpp
void loadFromJson() override {
    name_ = getString("name", "Unknown");  // 기본값
    
    // Optional 접근
    auto optAge = getOptionalInt64("age");
    if (optAge.has_value()) {
        age_ = static_cast<int>(optAge.value());
    }
    
    // 안전한 배열 접근
    if (hasKey("hobbies") && isArray("hobbies")) {
        hobbies_ = getArray<std::string>("hobbies");
    }
}
```

## 🏗️ 아키텍처

### 📋 클래스 역할 분리

| 클래스 | 역할 | 제공 기능 |
|--------|------|-----------|
| `JsonableBase` | 기본 JSON 조작 | `getString()`, `setString()`, `beginObject()` 등 |
| `FromJsonable` | 역직렬화 책임 | `fromJson()`, `loadFromJson()`, `loadField()` 등 |
| `ToJsonable` | 직렬화 책임 | `toJson()`, `saveToJson()`, `saveFieldIf()` 등 |
| `Jsonable` | 통합 인터페이스 | 모든 기능 + 편의 메서드들 |

### 🔧 타입 안전성 메커니즘

```cpp
// 컴파일 타임 타입 검증
template<typename T>
void setArray(const char* key, const std::vector<T>& values) {
    static_assert(is_json_primitive_v<T>, 
                 "Array elements must be JSON primitive types only");
    // ...
}

// 지원되는 기본 타입들
constexpr bool is_json_primitive_v<T> = std::disjunction_v<
    std::is_same<T, std::string>,
    std::is_same<T, int>,
    std::is_same<T, int64_t>,
    std::is_same<T, double>,
    std::is_same<T, float>,
    std::is_same<T, bool>,
    std::is_same<T, uint32_t>,
    std::is_same<T, uint64_t>
>;
```

## 🧪 테스트

### 단위 테스트 실행

```bash
cd unittest
./run_tests_nopause.bat  # Windows
```

### 지원되는 테스트들

- ✅ **기본 타입 직렬화/역직렬화**
- ✅ **배열 처리 (동일 타입만)**
- ✅ **중첩 구조 처리**
- ✅ **Begin/End 스타일 테스트**
- ✅ **타입 안전성 검증**
- ✅ **Null key 처리 안전성**
- ✅ **다중상속 구조 테스트**

## 📁 프로젝트 구조

```
jsonable/
├── 📄 Jsonable.hpp              # 🌟 메인 사용자 인터페이스
├── 📄 ToJsonable.hpp            # 📤 JSON 직렬화 책임
├── 📄 FromJsonable.hpp          # 📥 JSON 역직렬화 책임
├── 📄 JsonableBase.hpp          # 🔧 기본 JSON 조작
├── 📄 JsonableImpl.inl          # ⚙️ RapidJSON 구현부 (숨김)
├── 📁 samples/                  # 💡 예제 및 문서
│   ├── InheritanceExample.hpp   # 다중상속 사용 예제
│   ├── ArrayContextTest.hpp     # 배열 컨텍스트 테스트
│   ├── UnifiedApiDemo.hpp       # 통일된 API 데모
│   └── ...
├── 📁 unittest/                 # 🧪 단위 테스트
└── 📄 README.md                 # 📖 이 문서
```

## ⚡ 성능 특징

- 🚀 **Zero-Copy**: 가능한 한 복사 최소화
- 📦 **Header-Only**: 별도 라이브러리 빌드 불필요
- 🧠 **메모리 효율**: Virtual 상속으로 다이아몬드 문제 해결
- ⚡ **컴파일 타임 최적화**: Template 특수화 활용

## 🔒 보안 및 안전성

- 🛡️ **메모리 안전**: RAII 패턴과 스마트 포인터 사용
- 🚫 **Buffer Overflow 방지**: RapidJSON의 안전한 파싱
- 🔍 **타입 검증**: 컴파일 타임 + 런타임 이중 검증
- ⚠️ **예외 안전**: 강한 예외 보장 제공

## 🎯 설계 원칙

### SOLID 원칙 준수

1. **SRP (Single Responsibility)**: 각 클래스가 하나의 책임만 담당
2. **OCP (Open/Closed)**: 확장에는 열려있고 수정에는 닫혀있음
3. **LSP (Liskov Substitution)**: 파생 클래스는 기반 클래스를 완전히 대체 가능
4. **ISP (Interface Segregation)**: 필요한 인터페이스만 노출
5. **DIP (Dependency Inversion)**: RapidJSON 의존성을 완전히 숨김

### 타입 안전성 우선

- ❌ **Mixed Type 배열 금지**: `["string", 123, true]` 같은 배열 불허
- ✅ **단일 타입 배열**: `["a", "b", "c"]` 또는 `[1, 2, 3]`만 허용
- 🧪 **컴파일 타임 검증**: 런타임 오류를 컴파일 타임에 방지

## 🤝 기여하기

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 라이센스

이 프로젝트는 MIT 라이센스 하에 배포됩니다. 자세한 내용은 [LICENSE](LICENSE) 파일을 참조하세요.

## 🙏 감사의 말

- [RapidJSON](https://rapidjson.org/) - 빠르고 안정적인 JSON 파싱 라이브러리
- [GoogleTest](https://github.com/google/googletest) - 포괄적인 C++ 테스트 프레임워크

---

**Jsonable**로 타입 안전하고 깔끔한 JSON 처리를 경험해보세요! 🚀 