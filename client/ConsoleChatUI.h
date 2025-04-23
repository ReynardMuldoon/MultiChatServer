// ConsoleChatUI.h
#pragma once

#include <string>
#include <vector>
#include <windows.h>
#include <mutex>

class ConsoleChatUI {
public:
    ConsoleChatUI();

    void Init();                        // 콘솔 초기화
    void PrintMessage(const std::string& msg);  // 메시지 출력
    void RefreshInputLine();           // 입력 라인 갱신

    void SetInput(const std::string& input);    // 입력 문자열 설정
    std::string GetInput();                    // 현재 입력 문자열 반환

    void Lock();                        // 출력 동기화 잠금
    void Unlock();                      // 출력 동기화 해제

private:
    HANDLE hConsole;                   // 콘솔 핸들
    COORD inputPos;                    // 입력 라인의 시작 위치
    int screenHeight;                  // 콘솔 창 높이
    int screenWidth;                   // 콘솔 창 너비

    std::string currentInput;          // 현재 입력 중인 문자열
    std::vector<std::string> messages; // 출력된 메시지 저장
    std::mutex consoleMutex;           // 동기화를 위한 뮤텍스

    void MoveCursorToInputLine();      // 입력 라인으로 커서 이동
    void ClearInputLine();             // 입력 라인 클리어
    void UpdateScreenSize();            // 콘솔 창 크기 업데이트 
};