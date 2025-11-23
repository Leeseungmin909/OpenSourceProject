#!/bin/bash
# test_myshell.sh

echo "=== MyShell 테스트 시작 ==="

# 테스트 파일 준비
echo "apple" > test_fruits.txt
echo "banana" >> test_fruits.txt
echo "cherry" >> test_fruits.txt

echo ""
echo "1. cp 테스트"
echo "cp test_fruits.txt copied.txt" | ./myshell
echo "복사된 파일 확인:"
cat copied.txt

echo ""
echo "2. grep 테스트"
echo "grep 'banana' test_fruits.txt" | ./myshell

echo ""
echo "3. mv 테스트"
echo "mv copied.txt moved.txt" | ./myshell
ls -l moved.txt

echo ""
echo "4. ln 테스트"
echo "ln test_fruits.txt linked.txt" | ./myshell
ls -li test_fruits.txt linked.txt

echo ""
echo "5. 파이프 테스트"
echo "cat test_fruits.txt | grep 'apple'" | ./myshell

echo ""
echo "6. 출력 재지향 테스트"
echo "ls > file_list.txt" | ./myshell
echo "생성된 파일 목록:"
cat file_list.txt

echo ""
echo "7. 입력 재지향 테스트"
echo "cat < test_fruits.txt" | ./myshell

echo ""
echo "=== 테스트 종료 ==="
echo "정리..."
rm -f test_fruits.txt copied.txt moved.txt linked.txt file_list.txt
