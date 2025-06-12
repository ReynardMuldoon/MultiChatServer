# TCP Multi-Client Chat Program

## 개요
- Windows 환경(CMake + Visual Studio)에서 Winsock API를 사용하여
- 멀티 클라이언트 채팅 서버 및 클라이언트 프로그램을 구현했습니다.
- 콘솔(UI) 경험 개선을 위해 WinAPI를 활용해 입력/출력 영역을 분리하였습니다.
- 다양한 설계구조를 사용하여 구현해보고자 합니다.

### 멀티 쓰레딩 서버, SELECT 서버, WSAPOLL 서버, IOCP 서버를 구현하였습니다.
### 클라이언트의 경우 위 4가지 서버에 모두 사용 가능한 하나의 클라이언트 코드만 사용합니다.

## 프로젝트 계층 구조 
### TCP_MultiChat/
### ├── CMakeLists.txt
### ├── tcp_server/
### │   └── CMakeLists.txt
### │   └── main.cpp
### │   └── TCPServer.h
### │   └── TCPServer.cpp
### │   └── multithread/
### │       └── multithreadServer.h
### │       └── multithreadServer.cpp
### │   └── select/
### │       └── selectServer.h
### │       └── selectServer.cpp
### │   └── poll/
### │       └── pollServer.h
### │       └── pollServer.cpp
### │   └── multithread/
### │       └── iocpServer.h
### │       └── iocpServer.cpp
### ├── tcp_client/
### │   └── CMakeLists.txt
### │   └── ConsoleChatUI.h
### │   └── ConsoleChatUI.cpp
### │   └── main.cpp
### │   └── TCPClient.h
### │   └── TCPClient.cpp


## 빌드 방법
### 1. Visual Studio에서 Open Folder 기능으로 루트 폴더(TCP_MultiChat)를 엽니다.

### 2. CMakeLists.txt를 자동 인식하여 빌드 구성이 생성됩니다.

### 3. 원하는 Target(server/client)을 선택하여 빌드 및 실행합니다.


## 실행 방법
### 1. 서버 실행
[서버 프로그램 실행]
서버는 포트 12345를 열어 클라이언트 접속을 대기합니다.

### 2. 클라이언트 실행
[클라이언트 프로그램 실행]
기본 설정은 127.0.0.1:12345로 서버에 접속합니다.

### 3. 채팅
서버에 여러 클라이언트가 접속할 수 있습니다.
클라이언트는 콘솔 하단 입력줄에 메시지를 입력하고 Enter로 전송합니다.
상대방의 메시지가 오면 입력 도중에도 자연스럽게 출력됩니다.
서버에 보내는 첫 메시지는 자신의 닉네임이 됩니다.


# 주요 기술 
## 0. 공통 
- CMake: 프로젝트 빌드 관리
- WinAPI: 콘솔 출력 영역/입력 영역 제어
  - 입력 추적: _kbhit, _getch를 통한 실시간 타이핑 관리
  - 사용자 경험(UX) 개선: 입력 중에도 수신 메시지를 자연스럽게 출력
- 네트워크 통신: Winsock2 TCP/IP Socket API

## 1. 멀티 스레딩 버전 
- thread: HandleClient 함수를 실행.
- 멀티스레드: 클라이언트마다 하나의 쓰레드를 할당하여 통신 처리.

## 2. Select
- 목표: 하나의 프로세스(쓰레드)로 여러 클라이언트에 서비스 제공.
- fd_set: 감시할 소켓들을 담는 구조체(FD_SET, FD_ZERO, FD_ISSET 등으로 조작)
- select: 하나 이상의 소켓의 상태를 확인. 각 소켓에 대해 읽기, 쓰기 또는 오류 상태에 대한 정보를 요청 가능.
-  

## 3. Poll
- WSAPoll: 

## 4. IOCP (+ Overlapped I/O + Thread Pool)
- CreateIoCompletionPort: IOCP HANDLE을 설정하고 해당 HANDLE에 소켓들을 등록.
- GetQueuedCompletionStatus: IOCP HANDLE에 등록된 소켓들에 이벤트가 발생했을 때, 이를 읽어들임.
- 쓰레드 풀을 생성해서 GetQueuedCompletionStatus에서 


## 주요 기능
- 다수 클라이언트의 동시 접속 및 채팅 지원
- 서버 연결 종료 및 감지 처리
- 입력 중 메시지 수신 시 자연스러운 출력 및 입력 복원
- 콘솔 크기 변경에 동적 대응 (출력 시 콘솔 크기 재확인) 


## 한계 및 주의사항
- 콘솔 창 크기 변경을 출력 시에만 감지합니다. (실시간 감지 미지원)
- TCP 연결 기반으로, UDP 기반의 비연결성 통신은 지원하지 않습니다.


## 앞으로 개선할 수 있는 점
- GUI 기반 채팅 클라이언트 (예: WinForms, Qt 등) 제작
- 서버 관리 기능 추가 (Kick, Ban 등)


## ✨ 학습 포인트
- 실전 TCP/IP 소켓 프로그래밍 경험
- 스레드 동기화 및 데이터 공유 처리
- 사용자 중심의 콘솔 UI 설계
- 네트워크 오류/이벤트 처리

## 🔥 기타
본 프로젝트는 학습 및 연습을 목적으로 진행되었습니다.
코드 수정 및 확장 환영합니다!
