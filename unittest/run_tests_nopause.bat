@echo off
setlocal EnableDelayedExpansion
cd /d "%~dp0"

echo === Jsonable Unit Test Execution (Visual Studio 2022) ===

:: Visual Studio 2022 찾기
set "VS_INSTALL_PATH="
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community" (
    set "VS_INSTALL_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community"
    echo Visual Studio 2022 found: !VS_INSTALL_PATH!
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional" (
    set "VS_INSTALL_PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional"
    echo Visual Studio 2022 found: !VS_INSTALL_PATH!
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise" (
    set "VS_INSTALL_PATH=C:\Program Files\Microsoft Visual Studio\2022\Enterprise"
    echo Visual Studio 2022 found: !VS_INSTALL_PATH!
) else (
    echo Visual Studio 2022를 찾을 수 없습니다. 수동으로 경로를 설정해주세요.
    goto error
)

:: MSVC 환경 설정
echo Setting up MSVC environment...
call "!VS_INSTALL_PATH!\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 (
    echo MSVC 환경 설정 실패
    goto error
)

echo Test directory: %CD%

:: 빌드 디렉토리가 없으면 생성
if not exist "build" (
    echo Creating build directory: %CD%\build
    mkdir build
)

cd build

:: 환경 확인
echo.
echo === Environment Check ===
echo CMake path:
where cmake
echo MSBuild path:
where MSBuild

:: vcpkg 확인
if exist "%VCPKG_ROOT%" (
    echo.
    echo vcpkg found. Using vcpkg toolchain.
    set CMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
) else (
    echo.
    echo vcpkg not found. Using system libraries.
    set CMAKE_TOOLCHAIN_FILE=
)

:: CMake 구성
echo CMake configuration...
if defined CMAKE_TOOLCHAIN_FILE (
    cmake .. -DCMAKE_TOOLCHAIN_FILE="%CMAKE_TOOLCHAIN_FILE%"
) else (
    cmake ..
)

if errorlevel 1 (
    echo.
    echo CMake configuration failed.
    echo.
    echo Possible solutions:
    echo 1. Ensure GoogleTest is installed
    echo 2. For vcpkg users: vcpkg install gtest
    echo 3. Set GoogleTest path manually
    goto error
)

:: 빌드
echo.
echo Building...
MSBuild jsonable_unittest.vcxproj /p:Configuration=Debug /p:Platform=x64

if errorlevel 1 (
    echo Build failed.
    goto error
)

:: 테스트 실행
echo.
echo Running tests...
echo === Test Results ===
echo Executable: Debug\jsonable_unittest.exe
Debug\jsonable_unittest.exe

if errorlevel 1 (
    echo.
    echo Tests failed.
    goto error
)

echo.
echo === All tests passed ===

:: CTest 실행 (선택사항)
echo.
echo === CTest execution ===
ctest --output-on-failure

if errorlevel 1 (
    echo CTest failed.
    goto error
)

echo.
echo Test execution completed
goto end

:error
echo.
echo Test execution failed with error code %errorlevel%
exit /b %errorlevel%

:end
echo.
exit /b 0 