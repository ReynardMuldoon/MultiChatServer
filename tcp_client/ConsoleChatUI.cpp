// ConsoleChatUI.cpp
#include "ConsoleChatUI.h"
#include <iostream>
#include <conio.h>

ConsoleChatUI::ConsoleChatUI() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    UpdateScreenSize();
    inputPos = { 0, static_cast<SHORT>(screenHeight - 1) };
}

void ConsoleChatUI::Init() {
    system("cls");
    MoveCursorToInputLine();
}

void ConsoleChatUI::PrintMessage(const std::string& msg) {
    Lock();

    // 콘솔 크기를 실시간 반영하기 위해 매번 업데이트 
    UpdateScreenSize();
    // 입력 중에도 메시지가 오면 출력해야 하므로 입력 내용을 백업 후 출력
    std::string savedInput = currentInput;

    // 메시지를 저장하고 화면에 보이는 메시지 개수를 유지하기 위해 오래된 메시지 제거
    messages.push_back(msg);
    if (messages.size() >= screenHeight - 1) {
        messages.erase(messages.begin());
    }

    // 전체 출력 라인을 다시 그려 메시지를 일괄적으로 갱신
    for (int i = 0; i < screenHeight - 1; ++i) {
        COORD pos = { 0, static_cast<SHORT>(i) };
        SetConsoleCursorPosition(hConsole, pos);
        DWORD written;
        std::string line = (i < messages.size()) ? messages[i] : "";
        line.resize(screenWidth, ' '); // 기존 줄의 잔여 문자 제거
        WriteConsoleA(hConsole, line.c_str(), screenWidth, &written, nullptr);
    }

    // 백업했던 입력을 복구하여 사용자 입력 경험 유지
    SetInput(savedInput);
    RefreshInputLine();
    Unlock();
}

void ConsoleChatUI::RefreshInputLine() {
    ClearInputLine(); // 기존 입력 줄의 잔여 문자를 지우기 위함
    MoveCursorToInputLine();
    DWORD written;
    std::string printString = "> " + currentInput;  // 입력을 위한 "> " 문자열 함께 출력 
    WriteConsoleA(hConsole, printString.c_str(), printString.size(), &written, nullptr);
}

void ConsoleChatUI::SetInput(const std::string& input) {
    currentInput = input;
}

std::string ConsoleChatUI::GetInput() {
    return currentInput;
}

void ConsoleChatUI::MoveCursorToInputLine() {
    SetConsoleCursorPosition(hConsole, inputPos);
}

void ConsoleChatUI::ClearInputLine() {
    // 커서를 입력 라인으로 이동시키고 해당 라인을 공백으로 덮어 기존 입력 제거
    COORD pos = inputPos;
    SetConsoleCursorPosition(hConsole, pos);
    DWORD written;
    std::string blank(screenWidth, ' ');
    WriteConsoleA(hConsole, blank.c_str(), screenWidth, &written, nullptr);
    SetConsoleCursorPosition(hConsole, pos);
}

void ConsoleChatUI::Lock() {
    consoleMutex.lock();
}

void ConsoleChatUI::Unlock() {
    consoleMutex.unlock();
}

// 콘솔 화면 크기를 업데이트하는 함수
void ConsoleChatUI::UpdateScreenSize() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    screenHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    screenWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    inputPos = { 0, static_cast<SHORT>(screenHeight - 1) };
}

// 사용자 키보드 입력을 추적하고 표시하는 함수
void ConsoleChatUI::ProcessInputChar(char ch) {
    if (ch == '\b') {
        if (!currentInput.empty()) {
            currentInput.pop_back();
        }
    }
    else if (isprint(static_cast<unsigned char>(ch))) {
        currentInput.push_back(ch);
    }
    RefreshInputLine();
}

// 입력 완료 시 currentInput을 반환하고 초기화
std::string ConsoleChatUI::ConsumeInput() {
    std::string result = currentInput;
    currentInput.clear();
    RefreshInputLine();
    return result;
}