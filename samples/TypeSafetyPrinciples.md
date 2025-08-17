# Jsonable 라이브러리 타입 안전성 설계 원칙

## 🎯 핵심 철학

**"강타입 언어의 장점을 살리는 JSON 라이브러리"**

JSON의 동적 특성을 C++의 강타입 시스템에 안전하게 매핑하여, 런타임 오류를 최소화하고 컴파일 타임에 타입 검증을 수행합니다.

## ✅ 지원하는 타입 안전 패턴

### 1. 단일 타입 배열만 지원

```cpp
// ✅ 좋은 설계 - 타입 안전
std::vector<std::string> names = {"Alice", "Bob", "Charlie"};
jsonObj.setArray("names", names);

std::vector<int> scores = {100, 95, 88};
jsonObj.setArray("scores", scores);

// ✅ 읽기도 타입 안전
auto names = jsonObj.getArray<std::string>("names");
auto scores = jsonObj.getArray<int>("scores");
```

### 2. 컴파일 타임 타입 검증

```cpp
// ✅ 지원되는 기본 JSON 타입들
jsonObj.setField("name", std::string("Alice"));     // string
jsonObj.setField("age", int(25));                   // int
jsonObj.setField("height", double(175.5));          // double
jsonObj.setField("active", bool(true));             // bool

// ❌ 컴파일 에러 - 사용자 정의 타입
struct CustomType { int x; };
CustomType custom;
// jsonObj.setField("custom", custom);  // static_assert 실패!
```

### 3. 명시적 타입 변환

```cpp
// ✅ 명시적 타입 지정으로 의도 명확화
int age = jsonObj.getField<int>("age");
double height = jsonObj.getField<double>("height");
std::string name = jsonObj.getField<std::string>("name");
```

## ❌ 지원하지 않는 패턴 (타입 안전성 위배)

### 1. Mixed Type 배열

```cpp
// ❌ 나쁜 설계 - 타입 안전성 위배
// JSON: ["Alice", 25, true, 175.5]
// 이런 배열은 지원하지 않음!

// 왜 나쁜가?
// 1. 컴파일 타임에 타입 검증 불가
// 2. 런타임에 타입 오류 발생 가능
// 3. 코드 가독성 및 유지보수성 저하
```

### 2. 암시적 타입 변환

```cpp
// ❌ 피해야 할 패턴
auto value = jsonObj.getValue("someKey");  // 타입이 뭔지 모름
if (value.isString()) { /* ... */ }       // 런타임 타입 체크 필요
```

### 3. std::any, std::variant 사용

```cpp
// ❌ 지원하지 않는 패턴
std::vector<std::any> mixedArray;         // 타입 안전성 없음
std::vector<std::variant<int, string>> variants;  // 복잡성 증가
```

## 🔧 타입 안전성 구현 메커니즘

### 1. 컴파일 타임 타입 검증

```cpp
template<typename T>
constexpr bool is_json_primitive_v = std::disjunction_v<
    std::is_same<T, std::string>,
    std::is_same<T, int>,
    std::is_same<T, int64_t>,
    std::is_same<T, double>,
    std::is_same<T, float>,
    std::is_same<T, bool>,
    std::is_same<T, uint32_t>,
    std::is_same<T, uint64_t>
>;

template<typename T>
void setField(const char* key, const T& value) {
    static_assert(is_json_primitive_v<T>, 
                 "Only JSON primitive types are supported for type safety");
    // ...
}
```

### 2. 런타임 타입 호환성 체크

```cpp
template<typename T>
T getField(const char* key) const {
    if constexpr (std::is_same_v<T, std::string>) {
        return getString(key);  // JSON이 string이 아니면 기본값 반환
    }
    else if constexpr (std::is_same_v<T, int>) {
        return static_cast<int>(getInt64(key));  // 안전한 변환
    }
    // ...
}
```

### 3. 명확한 오류 메시지

```cpp
static_assert(is_json_primitive_v<T>, 
             "Array elements must be JSON primitive types only. "
             "Mixed type arrays are not supported for type safety.");
```

## 📋 설계 결정 이유

### 1. **타입 안전성 > 편의성**
- 런타임 오류보다 컴파일 타임 오류가 훨씬 안전
- 명시적 타입 지정으로 코드 의도 명확화

### 2. **C++ 철학 준수**
- 강타입 언어의 장점 활용
- "런타임에 알 수 있는 것을 컴파일 타임에 검증"

### 3. **유지보수성**
- 타입이 명확하면 코드 이해가 쉬움
- 리팩토링 시 타입 오류를 컴파일러가 잡아줌

### 4. **성능**
- 런타임 타입 체크 최소화
- 템플릿 특수화로 최적화된 코드 생성

## 🎯 사용 권장 패턴

### 1. 구조화된 데이터

```cpp
class Person : public json::Jsonable {
    std::string name_;
    int age_;
    std::vector<std::string> hobbies_;  // 동일 타입 배열
    
    void saveToJson() override {
        beginObject();
        {
            setString("name", name_);
            setInt64("age", static_cast<int64_t>(age_));
            
            beginArray("hobbies");
            {
                for (const auto& hobby : hobbies_) {
                    pushString(hobby);  // 배열 요소 추가, 타입 명확
                }
            }
            endArray();
        }
        endObject();
    }
};
```

### 2. 타입별 분리된 배열

```cpp
// ✅ 좋은 설계 - 타입별로 분리
void saveToJson() override {
    beginObject();
    {
        // 문자열 배열
        beginArray("names");
        for (const auto& name : names_) pushString(name);
        endArray();
        
        // 숫자 배열  
        beginArray("scores");
        for (auto score : scores_) pushInt64(score);
        endArray();
        
        // 불린 배열
        beginArray("flags");
        for (auto flag : flags_) pushBool(flag);
        endArray();
    }
    endObject();
}
```

## 🚀 결론

Jsonable 라이브러리는 **"타입 안전성을 타협하지 않는 JSON 처리"**를 목표로 설계되었습니다.

- ✅ **컴파일 타임 검증**: 런타임 오류 방지
- ✅ **명시적 타입 지정**: 코드 의도 명확화  
- ✅ **단일 타입 배열**: 타입 안전성 보장
- ✅ **C++ 철학 준수**: 강타입 언어의 장점 활용

이러한 원칙을 통해 안전하고 유지보수 가능한 JSON 처리 코드를 작성할 수 있습니다. 