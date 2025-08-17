# Jsonable 단위 테스트 CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(JsonableUnitTest)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Windows 특정 설정
if(WIN32)
    # Visual Studio 멀티프로세서 컴파일 활성화
    add_compile_options(/MP)
    
    # Windows SDK 및 런타임 설정
    set(CMAKE_SYSTEM_VERSION 10.0)
    
    # 정적 런타임 라이브러리 사용 (선택사항)
    # set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# FetchContent 모듈 포함
include(FetchContent)

# GoogleTest 찾기 (vcpkg 또는 수동 설치)
find_package(GTest CONFIG QUIET)

if(NOT GTest_FOUND)
    message(STATUS "GTest CONFIG not found, trying Module mode...")
    find_package(GTest MODULE QUIET)
    
    if(NOT GTest_FOUND)
        message(STATUS "GTest를 find_package로 찾을 수 없습니다. FetchContent로 다운로드합니다...")
        
        # GoogleTest를 FetchContent로 다운로드
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        release-1.12.1
        )
        
        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        
        FetchContent_MakeAvailable(googletest)
        
        # FetchContent로 가져온 경우 타겟 별칭 설정
        add_library(GTest::GTest ALIAS gtest)
        add_library(GTest::Main ALIAS gtest_main)
        
        message(STATUS "GoogleTest가 FetchContent로 성공적으로 다운로드되었습니다.")
        set(GTest_FOUND TRUE)
    endif()
endif()

# Threads 라이브러리 찾기
find_package(Threads REQUIRED)

# 테스트 실행 파일 생성
add_executable(jsonable_unittest
    JsonableTest.cpp
    # TestJsonable.hpp는 헤더 전용이므로 소스에 포함하지 않음
)

# 헤더 포함 디렉토리 설정
target_include_directories(jsonable_unittest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/rapidjson/include
)

# GoogleTest 포함 경로 추가
if(TARGET GTest::gtest)
    # 모던 CMake 타겟 사용
    target_link_libraries(jsonable_unittest 
        GTest::gtest 
        GTest::gtest_main
        Threads::Threads
    )
elseif(TARGET GTest::GTest)
    # FetchContent 별칭 사용
    target_link_libraries(jsonable_unittest 
        GTest::GTest 
        GTest::Main
        Threads::Threads
    )
else()
    # 레거시 변수 사용
    target_include_directories(jsonable_unittest PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(jsonable_unittest 
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES}
        Threads::Threads
    )
endif()

# 컴파일 옵션 설정
target_compile_features(jsonable_unittest PRIVATE cxx_std_17)

# Windows 특정 설정
if(WIN32)
    target_compile_definitions(jsonable_unittest PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
    )
    
    # MSVC 특정 설정
    if(MSVC)
        target_compile_options(jsonable_unittest PRIVATE
            /W4                 # 경고 레벨 4
            /WX-                # 경고를 오류로 처리하지 않음
            /wd4100             # 'identifier' : unreferenced formal parameter
            /wd4996             # deprecated 함수 경고 무시
            /permissive-        # 표준 준수 모드
            /utf-8              # UTF-8 인코딩 사용
        )
        
        # Debug 설정
        target_compile_options(jsonable_unittest PRIVATE
            $<$<CONFIG:Debug>:/MDd>     # 동적 런타임 라이브러리 (Debug)
            $<$<CONFIG:Debug>:/Od>      # 최적화 비활성화
            $<$<CONFIG:Debug>:/Zi>      # 디버그 정보 생성
        )
        
        # Release 설정
        target_compile_options(jsonable_unittest PRIVATE
            $<$<CONFIG:Release>:/MD>    # 동적 런타임 라이브러리 (Release)
            $<$<CONFIG:Release>:/O2>    # 최적화 활성화
        )
    endif()
else()
    # Linux/macOS 설정
    target_compile_options(jsonable_unittest PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
    )
endif()

# 테스트 활성화
enable_testing()

# 테스트 추가
add_test(NAME JsonableUnitTest COMMAND jsonable_unittest)

# GoogleTest 자동 검색을 위한 설정 (CMake 3.10 이상)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.10.0")
    include(GoogleTest)
    gtest_discover_tests(jsonable_unittest
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

# Visual Studio에서 디버그 작업 디렉토리 설정
if(WIN32)
    set_property(TARGET jsonable_unittest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif() 