# Jsonable - C++ JSON 직렬화 라이브러리

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)

**Jsonable**은 C++에서 JSON 직렬화/역직렬화를 쉽게 사용할 수 있도록 하는 가벼운 라이브러리입니다. 다른 언어의 `toJson()`/`fromJson()` 패턴을 따라 **내재적 직렬화(Intrusive Serialization)** 방식을 제공하여, 별도의 JSON 라이브러리 학습 없이 바로 사용할 수 있습니다.

## ✨ 주요 특징

- **🚀 간단한 사용법**: 상속만으로 JSON 직렬화 기능 추가
- **📦 헤더 전용**: 단일 헤더 파일로 쉬운 통합
- **⚡ 고성능**: RapidJSON 기반의 빠른 파싱
- **🔄 양방향 변환**: JSON ↔ C++ 객체 자동 변환
- **🛠️ 매크로 지원**: 반복 코드 최소화
- **💪 풍부한 타입 지원**: 기본 타입, 배열, 중첩 객체, std::optional
- **🎯 직관적 API**: 다른 JSON 라이브러리 학습 불필요

## 🚀 빠른 시작

### 1. 기본 사용법

```cpp
#include "Jsonable.hpp"

class Person : public json::Jsonable {
private:
    std::string name_;
    int age_;
    bool isStudent_;

public:
    Person() : age_(0), isStudent_(false) {}
    Person(const std::string& name, int age, bool isStudent) 
        : name_(name), age_(age), isStudent_(isStudent) {}

    // 매크로로 JSON 직렬화 자동 구현
    JSONABLE_IMPL()

    void fromDocument(const rapidjson::Value& value) override {
        JSON_FIELD_STRING(value, name_, "name");
        JSON_FIELD_INT64(value, age_, "age");
        JSON_FIELD_BOOL(value, isStudent_, "isStudent");
    }

    rapidjson::Value toValue(rapidjson::Document::AllocatorType& allocator) const override {
        rapidjson::Value person(rapidjson::kObjectType);
        JSON_SET_STRING(person, "name", name_, allocator);
        JSON_SET_PRIMITIVE(person, "age", age_, allocator);
        JSON_SET_PRIMITIVE(person, "isStudent", isStudent_, allocator);
        return person;
    }
};

// 사용 예제
int main() {
    // 객체 생성
    Person person("홍길동", 30, true);
    
    // JSON 직렬화 - 한 줄로 끝!
    std::string json = person.toJson();
    
    // JSON 역직렬화 - 한 줄로 끝!
    Person restored;
    restored.fromJson(json);
    
    return 0;
}
```

### 2. 복잡한 중첩 구조

```cpp
class Company : public json::Jsonable {
    std::string name_;
    std::vector<Department> departments_;
    std::vector<Person> employees_;
    
    // ... 구현 생략 ...
};

// 복잡한 JSON도 한 줄로!
Company company;
company.fromJson(complexJsonString);
std::string serialized = company.toJson();
```

## 📖 JSON ↔ C++ 타입 매핑 표준

### ✅ **권장 타입 매핑** (크로스 플랫폼 호환성)

| JSON 타입 | C++ 타입 | 이유 | 예시 |
|-----------|----------|------|------|
| `string` | `std::string` | **필수**, UTF-8 지원 | `"Hello"` → `std::string` |
| `number` | `int64_t` | **권장**, JSON 표준 범위 | `123` → `int64_t` |
| `number` | `double` | **권장**, IEEE 754 호환 | `3.14` → `double` |
| `boolean` | `bool` | **필수**, 표준 불린 | `true` → `bool` |
| `array` | `std::vector<T>` | **권장**, 동적 크기 | `[1,2,3]` → `std::vector<int64_t>` |
| `null` | `std::optional<T>` | **권장**, null 안전성 | `null` → `std::optional<T>{}` |
| `object` | `Jsonable 상속` | **권장**, 중첩 구조 | `{"x":1}` → Custom Class |

### ⚠️ **주의해서 사용할 타입들** (범위/정밀도 제한)

| JSON 타입 | C++ 타입 | 주의사항 | 대안 |
|-----------|----------|----------|------|
| `number` | `uint32_t` | 음수 처리 불가, 32비트 제한 | `int64_t` 사용 권장 |
| `number` | `uint64_t` | 음수 처리 불가 | `int64_t` 사용 권장 |
| `number` | `float` | 정밀도 손실 가능 | `double` 사용 권장 |
| `number` | `int32_t` | 범위 제한 (32비트) | `int64_t` 사용 권장 |

### ❌ **사용하지 말아야 할 타입들** (호환성 문제)

| C++ 타입 | 문제점 | 권장 대안 |
|----------|--------|-----------|
| `short`, `char` | 범위 너무 작음 (16비트/8비트) | `int64_t` |
| `long long` | 플랫폼 의존적 크기 | `int64_t` |
| `size_t`, `ptrdiff_t` | 플랫폼별 크기 차이 | `int64_t` |
| `wchar_t` | 플랫폼별 구현 차이 | `std::string` (UTF-8) |

### 💡 **매핑 예시 코드**

```cpp
class DataModel : public json::Jsonable {
private:
    // ✅ 권장하는 안전한 타입들
    std::string name_;                    // JSON string
    int64_t id_;                         // JSON number (정수)
    double price_;                       // JSON number (소수)
    bool active_;                        // JSON boolean
    std::vector<std::string> tags_;      // JSON array
    std::optional<std::string> note_;    // JSON null 허용

public:
    JSONABLE_IMPL()
    
    void fromDocument(const rapidjson::Value& value) override {
        JSON_FIELD_STRING(value, name_, "name");     // 문자열 매핑
        JSON_FIELD_INT64(value, id_, "id");          // 정수 매핑  
        JSON_FIELD_DOUBLE(value, price_, "price");   // 실수 매핑
        JSON_FIELD_BOOL(value, active_, "active");   // 불린 매핑
        
        // 배열 매핑
        tags_.clear();
        if (isArray(value, "tags")) {
            const auto& arr = value["tags"];
            for (const auto& item : arr.GetArray()) {
                if (item.IsString()) {
                    tags_.push_back(item.GetString());
                }
            }
        }
        
        // 선택적 필드 매핑 (null 안전)
        note_ = getOptionalString(value, "note");
    }
    
    rapidjson::Value toValue(rapidjson::Document::AllocatorType& allocator) const override {
        rapidjson::Value obj(rapidjson::kObjectType);
        JSON_SET_STRING(obj, "name", name_, allocator);
        JSON_SET_PRIMITIVE(obj, "id", id_, allocator);
        JSON_SET_PRIMITIVE(obj, "price", price_, allocator);
        JSON_SET_PRIMITIVE(obj, "active", active_, allocator);
        // ... 배열 및 선택적 필드 설정
        return obj;
    }
};
```

### 🎯 **타입 선택 가이드라인**

1. **기본 원칙**: JSON 표준을 벗어나지 않는 타입 선택
2. **호환성 우선**: 플랫폼 독립적인 고정 크기 타입 사용
3. **안전성 중시**: null 허용 필드는 `std::optional<T>` 사용
4. **성능 고려**: 불필요한 타입 변환 최소화
5. **유지보수성**: 명확하고 예측 가능한 타입 매핑

## 🛠️ 고급 기능

### 배열/객체 반복자 (Functor Injection)

```cpp
// 배열 반복
json::Jsonable::iterateArray(doc, "items", [](const rapidjson::Value& item) {
    // 각 배열 요소 처리
});

// 객체 반복  
json::Jsonable::iterateObject(doc, "config", [](const char* key, const rapidjson::Value& val) {
    // 각 객체 속성 처리
});
```

### 중첩 경로 탐색

```cpp
// 깊은 경로 접근
std::string value = json::Jsonable::getNestedString(doc, "company.department.manager");
bool exists = json::Jsonable::hasNestedPath(doc, "settings.debug.enabled");
```

### 비침입적(Non-intrusive) 방식

```cpp
class SimpleData {
    std::string title_;
    int count_;
public:
    // 상속 없이도 JSON 직렬화 가능
    static SimpleData fromJson(const std::string& json) { /* ... */ }
    std::string toJson() const { /* ... */ }
};
```

## 💻 설치

### 1. 헤더 전용 라이브러리

```bash
# 단순히 헤더 파일만 복사
cp Jsonable.hpp /your/project/include/
```

### 2. CMake 통합

```cmake
# CMakeLists.txt에 추가
target_include_directories(your_target PRIVATE path/to/jsonable)

# RapidJSON 의존성 추가
target_include_directories(your_target PRIVATE path/to/rapidjson/include)
```

### 3. 의존성

- **RapidJSON**: JSON 파싱 엔진
- **C++17**: std::optional 등 모던 C++ 기능

## 🧪 테스트

```bash
# Windows (Visual Studio 2022)
cd unittest
run_tests_nopause.bat

# Linux/macOS
cd unittest && mkdir build && cd build
cmake .. && make && ./jsonable_unittest
```

**테스트 통계:**
- ✅ **39개 단위 테스트** 모두 통과
- ✅ **867줄 테스트 코드**로 검증된 안정성
- ✅ 복잡한 중첩 구조, 성능, 에러 처리 모두 테스트

## 🆚 다른 라이브러리와 비교

| 특징 | Jsonable | nlohmann/json | RapidJSON | Boost.JSON |
|------|----------|---------------|-----------|------------|
| 헤더 전용 | ✅ | ✅ | ✅ | ❌ |
| 학습 곡선 | 매우 낮음 | 중간 | 높음 | 높음 |
| 내재적 직렬화 | ✅ | ❌ | ❌ | ❌ |
| 성능 | 높음 | 중간 | 매우 높음 | 높음 |
| 사용 편의성 | 매우 높음 | 높음 | 낮음 | 중간 |

## 🎯 사용 사례

- **설정 파일 로딩**: 애플리케이션 설정을 JSON으로 저장/로딩
- **API 통신**: REST API의 JSON 페이로드 직렬화
- **데이터 교환**: 서비스 간 데이터 교환 포맷
- **게임 데이터**: 게임 상태, 설정, 저장 데이터
- **로그 구조화**: 구조화된 로그 데이터 JSON 변환

## 📚 문서

더 자세한 사용법과 예제는 `unittest/` 폴더의 테스트 코드를 참고하세요:

- `TestJsonable.hpp`: 다양한 사용 패턴 예제
- `JsonableTest.cpp`: 상세한 기능 테스트
- 39개 테스트 케이스로 모든 기능 학습 가능

## 🤝 기여

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 라이선스

이 프로젝트는 MIT 라이선스 하에 배포됩니다. 자세한 내용은 [LICENSE](LICENSE) 파일을 참고하세요.

## 👨‍💻 저자

- **whyjp** - Initial work

## 🙏 감사 인사

- [RapidJSON](https://rapidjson.org/) - 고성능 JSON 파싱 라이브러리
- [GoogleTest](https://github.com/google/googletest) - 단위 테스트 프레임워크 