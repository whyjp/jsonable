#!/bin/bash

# Jsonable 라이브러리 단위 테스트 빌드 및 실행 스크립트
# Linux/macOS 빌드 자동화

set -e  # 오류 발생 시 스크립트 중단

# 색상 출력 정의
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Jsonable 단위 테스트 빌드 시작${NC}"

# 현재 스크립트 위치 확인
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo -e "${YELLOW}작업 디렉토리: $SCRIPT_DIR${NC}"

# build 폴더 리셋
if [ -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}기존 build 폴더 제거 중...${NC}"
    rm -rf "$BUILD_DIR"
fi

# build 폴더 생성
echo -e "${YELLOW}build 폴더 생성 중...${NC}"
mkdir -p "$BUILD_DIR"

# CMake 설정 및 빌드
echo -e "${BLUE}CMake 설정 중...${NC}"
cd "$BUILD_DIR"

# CMake 설정 (Release 모드)
cmake .. -DCMAKE_BUILD_TYPE=Release

# 빌드 실행
echo -e "${BLUE}빌드 실행 중...${NC}"
cmake --build . --config Release

# 빌드 성공 확인
if [ ! -f "jsonable_unittest" ]; then
    echo -e "${RED}빌드 실패: 실행 파일을 찾을 수 없습니다${NC}"
    exit 1
fi

echo -e "${GREEN}빌드 성공!${NC}"

# 테스트 실행
echo -e "${BLUE}단위 테스트 실행 중...${NC}"
echo "=================================================="

# 테스트 실행 (상세 출력)
./jsonable_unittest --gtest_color=yes

# 테스트 결과 확인
TEST_RESULT=$?

echo "=================================================="

if [ $TEST_RESULT -eq 0 ]; then
    echo -e "${GREEN}모든 테스트 통과!${NC}"
else
    echo -e "${RED}테스트 실패! (exit code: $TEST_RESULT)${NC}"
    exit 1
fi

echo -e "${GREEN}Jsonable 단위 테스트 완료${NC}"