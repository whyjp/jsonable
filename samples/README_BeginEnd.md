# Jsonable - Begin/End 스타일 API

## 🎯 개요

SOLID 원칙 기반의 Jsonable 라이브러리에 새로운 **Begin/End 스타일 API**가 추가되었습니다. 이 API는 구조적이고 직관적인 JSON 생성을 제공합니다.

## ✨ Begin/End 스타일의 장점

1. **🏗️ 직관적 구조**: JSON 구조와 코드 구조가 완벽히 일치
2. **🔒 타입 안전성**: 컨텍스트별 자동 타입 검증
3. **📖 가독성 향상**: 중첩 구조가 코드에서 명확히 보임
4. **📚 스택 기반**: 자동 컨텍스트 관리로 실수 방지
5. **🔐 완전 은닉**: RapidJSON 의존성 완전히 숨김

## 🚀 기본 사용법

### 1. 간단한 객체 생성

```cpp
class Person : public json::Jsonable {
    void saveToJson() override {
        beginObject(); // 루트 객체 시작
        {
            setString("name", name_);
            setInt64("age", static_cast<int64_t>(age_));
            setBool("active", active_);
        }
        endObject(); // 루트 객체 종료
    }
};
```

**생성되는 JSON:**
```json
{
    "name": "김개발",
    "age": 28,
    "active": true
}
```

### 2. 배열 생성

```cpp
void saveToJson() override {
    beginObject();
    {
        setString("title", title_);
        
        beginArray("hobbies"); // 배열 시작
        {
            for (const auto& hobby : hobbies_) {
                pushString(hobby);  // 배열 요소 추가
            }
        }
        endArray(); // 배열 종료
    }
    endObject();
}
```

**생성되는 JSON:**
```json
{
    "title": "개발자",
    "hobbies": ["프로그래밍", "독서", "게임"]
}
```

### 3. 복잡한 중첩 구조

```cpp
void saveToJson() override {
    beginObject(); // 루트 객체
    {
        setString("company", name_);
        
        // 직원 배열
        beginArray("employees");
        {
            for (const auto& employee : employees_) {
                beginObject(); // 새 직원 객체 시작
                {
                    setString("name", employee.getName());
                    setInt64("age", static_cast<int64_t>(employee.getAge()));
                    
                    beginArray("skills"); // 기술 배열
                    {
                        for (const auto& skill : employee.getSkills()) {
                            pushString(skill);  // 배열 요소 추가
                        }
                    }
                    endArray();
                }
                endObject(); // 직원 객체 종료
            }
        }
        endArray();
        
        // 메타데이터 객체
        beginObject("metadata");
        {
            setInt64("employeeCount", static_cast<int64_t>(employees_.size()));
            setString("lastUpdated", getCurrentTimestamp());
        }
        endObject();
    }
    endObject(); // 루트 객체 종료
}
```

**생성되는 JSON:**
```json
{
    "company": "테크 이노베이션",
    "employees": [
        {
            "name": "김개발",
            "age": 28,
            "skills": ["C++", "Python", "JavaScript"]
        },
        {
            "name": "이디자인",
            "age": 26,
            "skills": ["Photoshop", "Figma", "UI/UX"]
        }
    ],
    "metadata": {
        "employeeCount": 2,
        "lastUpdated": "2024-01-15T10:30:00Z"
    }
}
```

### 4. 2차원 배열 구조

```cpp
void saveToJson() override {
    beginObject();
    {
        setString("title", "매트릭스 데이터");
        
        // 2차원 배열
        beginArray("matrix");
        {
            for (const auto& row : matrix_) {
                beginArray(); // 새로운 행 배열 시작 (배열 내에서 key 없음)
                {
                    for (int value : row) {
                        pushInt64(static_cast<int64_t>(value));  // 배열 요소 추가
                    }
                }
                endArray(); // 행 배열 종료
            }
        }
        endArray();
    }
    endObject();
}
```

**생성되는 JSON:**
```json
{
    "title": "매트릭스 데이터",
    "matrix": [
        [1, 2, 3],
        [4, 5, 6],
        [7, 8, 9]
    ]
}
```

## 📚 API 참조

### 구조 관리

| 메서드 | 설명 |
|--------|------|
| `beginObject(const char* key = nullptr)` | 객체 시작 (key가 없으면 루트) |
| `endObject()` | 현재 객체 종료 |
| `beginArray(const char* key = nullptr)` | 배열 시작 (key가 없으면 루트) |
| `endArray()` | 현재 배열 종료 |

### 객체 내 값 설정

| 메서드 | 설명 |
|--------|------|
| `setString(key, value)` | 현재 객체에 문자열 필드 추가 |
| `setInt64(key, value)` | 현재 객체에 정수 필드 추가 |
| `setDouble(key, value)` | 현재 객체에 실수 필드 추가 |
| `setBool(key, value)` | 현재 객체에 불린 필드 추가 |

### 배열 내 값 추가

| 메서드 | 설명 |
|--------|------|
| `pushString(value)` | 현재 배열에 문자열 요소 추가 |
| `pushInt64(value)` | 현재 배열에 정수 요소 추가 |
| `pushDouble(value)` | 현재 배열에 실수 요소 추가 |
| `pushBool(value)` | 현재 배열에 불린 요소 추가 |

### 중첩 구조 생성

| 메서드 | 설명 |
|--------|------|
| `beginObject()` | 배열 내에 새 객체 시작 |
| `beginArray()` | 배열 내에 새 배열 시작 |

## 🔄 기존 방식과 비교

### 기존 방식 (개별 필드 설정)
```cpp
void saveToJson() override {
    setString("name", name_);
    setInt64("age", age_);
    setArray("hobbies", hobbies_);
}
```

### Begin/End 스타일 (구조적 접근)
```cpp
void saveToJson() override {
    beginObject();
    {
        setString("name", name_);
        setInt64("age", age_);
        
        beginArray("hobbies");
        {
            for (const auto& hobby : hobbies_) {
                pushString(hobby);  // 배열 요소 추가
            }
        }
        endArray();
    }
    endObject();
}
```

## 🎯 언제 사용하면 좋을까?

- ✅ **복잡한 중첩 구조**: 다층 객체/배열 구조
- ✅ **동적 배열**: 런타임에 결정되는 배열 크기
- ✅ **구조 시각화**: JSON 구조를 코드에서 명확히 보고 싶을 때
- ✅ **타입 안전성**: 컨텍스트별 타입 검증이 필요할 때

## 🔧 SOLID 원칙 준수

- **SRP**: 각 API가 단일 책임(구조 생성)만 담당
- **OCP**: 새로운 타입 추가 시 기존 코드 변경 없음
- **LSP**: 모든 Begin/End 호출이 동일한 인터페이스 사용
- **ISP**: 필요한 메서드만 노출
- **DIP**: RapidJSON 의존성 완전히 숨김

## 🚀 시작하기

1. `Jsonable.hpp`만 include
2. `json::Jsonable` 상속
3. `saveToJson()`에서 Begin/End API 사용
4. 구조적이고 직관적인 JSON 생성 완료!

```cpp
#include "Jsonable.hpp"

class MyClass : public json::Jsonable {
    void saveToJson() override {
        beginObject();
        {
            setString("field1", value1_);
            setInt64("field2", value2_);
            
            beginArray("items");
            {
                for (const auto& item : items_) {
                    pushString(item);  // 배열 요소 추가
                }
            }
            endArray();
        }
        endObject();
    }
};
```

**✨ 핵심 개선사항:**
- 🚫 **복잡한 nested 헬퍼 제거**: `saveNestedObject()` 등 불필요한 함수 제거
- ⚡ **성능 향상**: 함수 포인터 오버헤드 제거로 직접적인 처리
- 📖 **단순화된 API**: 두 가지 명확한 방식만 제공 (직접 설정 vs Begin/End)

Begin/End 스타일로 더 직관적이고 성능 좋은 JSON 처리를 경험해보세요! 🎉 