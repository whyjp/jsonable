#pragma once

/**
 * JsonableBase.hpp - 상속 계층의 최하위 레벨
 * 
 * 역할: RapidJSON 의존성 캡슐화 및 기본 JSON 조작 제공
 */

#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <cstdint>
#include <memory>

namespace json {

/**
 * @brief 상속 계층의 최하위 - RapidJSON 구현 캡슐화
 * 
 * 책임:
 * - RapidJSON document 관리
 * - 기본 타입 읽기/쓰기 (getString, setString 등)
 * - Begin/End 스택 관리
 * - 배열/객체 존재 확인
 * - RapidJSON 의존성 100% 숨김
 */
class JsonableImpl {
private:
    class Impl;  // RapidJSON 의존성 숨김용 PIMPL
    std::unique_ptr<Impl> pImpl_;

protected:
    // 파생 클래스에서만 생성/소멸 가능
    JsonableImpl();
    virtual ~JsonableImpl();
    
    // 복사/이동 (PIMPL 패턴)
    JsonableImpl(const JsonableImpl& other);
    JsonableImpl(JsonableImpl&& other) noexcept;
    JsonableImpl& operator=(const JsonableImpl& other);
    JsonableImpl& operator=(JsonableImpl&& other) noexcept;

public:
    // ========================================
    // 기본 타입 읽기 (RapidJSON 완전 숨김)
    // ========================================
    
    std::string getString(const char* key, const std::string& defaultValue = "") const;
    int64_t getInt64(const char* key, int64_t defaultValue = 0) const;
    double getDouble(const char* key, double defaultValue = 0.0) const;
    float getFloat(const char* key, float defaultValue = 0.0f) const;
    bool getBool(const char* key, bool defaultValue = false) const;
    uint32_t getUInt32(const char* key, uint32_t defaultValue = 0) const;
    uint64_t getUInt64(const char* key, uint64_t defaultValue = 0) const;
    
    // ========================================
    // 기본 타입 쓰기 (컨텍스트 자동 인식)
    // ========================================
    
    void setString(const char* key, const std::string& value);
    void setInt64(const char* key, int64_t value);
    void setDouble(const char* key, double value);
    void setFloat(const char* key, float value);
    void setBool(const char* key, bool value);
    void setUInt32(const char* key, uint32_t value);
    void setUInt64(const char* key, uint64_t value);
    
    // ========================================
    // 배열 처리 (타입 안전성 보장)
    // ========================================
    
    template<typename T>
    std::vector<T> getArray(const char* key) const;
    
    template<typename T>
    void setArray(const char* key, const std::vector<T>& values);
    
    // ========================================
    // 객체/배열 존재 확인
    // ========================================
    
    bool hasKey(const char* key) const;
    bool isArray(const char* key) const;
    bool isObject(const char* key) const;
    
    // ========================================
    // Iteration 함수들
    // ========================================
    
    void iterateArray(const char* key, std::function<void(size_t index)> processor) const;
    void iterateObject(const char* key, std::function<void(const std::string& key)> processor) const;
    
    // ========================================
    // Begin/End 스타일 구조적 JSON 생성
    // ========================================
    
    void beginObject(const char* key = nullptr);
    void endObject();
    void beginArray(const char* key = nullptr);
    void endArray();
    
    // 배열 요소 추가 편의 메서드들
    void pushString(const std::string& value);
    void pushInt64(int64_t value);
    void pushDouble(double value);
    void pushBool(bool value);
    void pushObject();
    void pushArray();
    
    // ========================================
    // 메타프로그래밍 기반 자동 처리
    // ========================================
    
    template<typename T>
    void setField(const char* key, const T& value);
    
    template<typename T>
    T getField(const char* key) const;
    
    // ========================================
    // Optional getter (안전한 접근)
    // ========================================
    
    std::optional<std::string> getOptionalString(const char* key) const;
    std::optional<int64_t> getOptionalInt64(const char* key) const;
    std::optional<double> getOptionalDouble(const char* key) const;
    std::optional<bool> getOptionalBool(const char* key) const;

private:
    // 내부 구현 접근
    Impl& getImpl();
    const Impl& getImpl() const;
};

} // namespace json

// 구현부는 조건부 포함
#ifdef JSONABLE_IMPLEMENTATION
#include "JsonableBaseImpl.inl"
#endif 