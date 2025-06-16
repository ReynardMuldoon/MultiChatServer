# TCP Multi-Client Chat Program

## 개요
- Windows 환경(CMake + Visual Studio)에서 Winsock API를 사용하여
- 멀티 클라이언트 채팅 서버 및 클라이언트 프로그램을 구현했습니다.
- 콘솔(UI) 경험 개선을 위해 WinAPI를 활용해 입력/출력 영역을 분리하였습니다.
- 다양한 설계구조를 사용하여 구현해보고자 합니다.

### 멀티 쓰레딩 서버, SELECT 서버, WSAPOLL 서버, IOCP 서버를 구현하였습니다.
### 클라이언트의 경우 위 4가지 서버에 모두 사용 가능한 하나의 클라이언트 코드만 사용합니다.

## 프로젝트 계층 구조 
 TCP_MultiChat/
 ├── CMakeLists.txt
 ├── tcp_server/
     └── CMakeLists.txt
     └── main.cpp
     └── TCPServer.h
     └── TCPServer.cpp
     └── multithread/
         └── multithreadServer.h
         └── multithreadServer.cpp
     └── select/
         └── selectServer.h
         └── selectServer.cpp
     └── poll/
         └── pollServer.h
         └── pollServer.cpp
     └── multithread/
         └── iocpServer.h
         └── iocpServer.cpp
 ├── tcp_client/
     └── CMakeLists.txt
     └── ConsoleChatUI.h
     └── ConsoleChatUI.cpp
     └── main.cpp
     └── TCPClient.h
     └── TCPClient.cpp


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

## 1. Multithread 버전 
- thread: HandleClient 함수를 실행.
- multithread: 클라이언트마다 하나의 쓰레드를 할당하여 동기적으로 통신을 처리.
- 클라이언트의 수가 많아질수록 비효율적.

## 2. Select
- 목표: 하나의 프로세스(쓰레드)로 여러 클라이언트에 서비스 제공.
- fd_set: 감시할 소켓들을 담는 구조체(FD_SET, FD_ZERO, FD_ISSET 등으로 조작)
- select: 하나 이상의 소켓의 상태를 확인. 각 소켓에 대해 읽기, 쓰기 또는 오류 상태에 대한 정보를 요청 가능.
  호출 시점에 소켓들의 상태를 검사하고, I/O가 가능한 소켓을 감지.
- int select(
    int nfds,
    fd_set* readfds,
    fd_set* writefds,
    fd_set* exceptfds,
    const struct timeval* timeout
);
- 단점:
  - FD_SETSIZE 제한(기본 64).
  - 각 호출마다 fd_set을 새로 구성해야 하므로 오버헤드가 큼.
  - 내부적으로 선형 탐색을 하기 때문에 크기가 커질수록 성능이 급감.

## 3. Poll
- WSAPoll: Winodws 전용 함수. 다중 소켓 I/O 감시. 리눅스의 poll() 함수와 유사. 
  WSAPOLLFD 배열 사용하여 감시할 소켓과 이벤트를 지정.
  **시그니처**
  int WSAPoll(
    LPWSAPOLLFD fdArray,
    ULONG fds,
    INT timeout
  );
  - fdArray: 감시할 소켓과 이벤트 정보를 담은 배열
  - fds: 배열의 길이
  - timeout: 타임아웃 (ms 단위)
- **장점:**
  - FD_SETSIZE 제한 없음 -> 이론 상 수천 개의 소켓도 감시 가능
  - 이벤트 지정이 직관적 (POLLIN, POLLOUT 등)
  - 매번 배열 초기화가 필요 없고, 기존 배열을 재사용 가능
  - 리눅스 코드 (poll)와 호환성이 높아 이식성도 향상됨
 
### Select와 WSAPoll의 실제 코드 비교 
**select 예시 **
fd_set readSet;
FD_ZERO(&readSet);
FD_SET(sock1, &readSet);
FD_SET(sock2, &readSet);

select(0, &readSet, NULL, NULL, &timeout);

**WSAPoll 예시 **
WSAPOLLFD fds[2];
fds[0].fd = sock1;
fds[0].events = POLLRDNORM;
fds[1].fd = sock2;
fds[1].events = POLLRDNORM;

WSAPoll(fds, 2, 1000);  // 타임아웃 1000ms

- WSAPoll은 select의 단점을 일부 보완한 함수 및 알고리즘. 

## 4. IOCP (+ Overlapped I/O + Thread Pool)
- 목표: 완전한 비동기 I/O와 Thread Pool을 사용하여 효율적으로 다수의 클라이언트와 효율적으로 통신.
### 주요 함수
- CreateIoCompletionPort: IOCP를 생성하거나, 기존 IOCP에 소켓(또는 파인 핸들)을 등록.
  **시그니처**
  HANDLE CreateIoCompletionPort(
  HANDLE FileHandle,       // 등록할 소켓 또는 핸들
  HANDLE ExistingCompletionPort, // 기존 IOCP 핸들 or NULL
  ULONG_PTR CompletionKey, // 연결된 키 (식별자)
  DWORD NumberOfConcurrentThreads // 커널이 동시에 실행시킬 수 있는 스레드 수
  );

  **사용 예**
  HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0); // IOCP 생성
  CreateIoCompletionPort((HANDLE)clientSocket, iocp, (ULONG_PTR)session, 0); // 클라이언트 소켓 등록
  
- WSARecv, WSASend: 비동기 데이터 수신/송신 요청. 비동기 모드 사용시, OVERLAPPED 구조체 반드시 필요. 
  완료 시 IOCP에 완료 통지 전달.
  **사용 예**
  WSARecv(
  clientSocket,
  &wsaBuf,
  1,
  NULL,         // 동기라면 여기 결과가, 비동기라면 무시
  &flags,
  &overlapped,  // 필수!
  NULL          // 비동기이므로 콜백은 NULL
  );
  - 비동기 처리를 위해 소켓이 overlapped 모드로 생성되어야 함: WSASocket(..., WSA_FLAG_OVERLAPPED, ...);
  
- GetQueuedCompletionStatus: 완료된 I/O 이벤트를 블로킹 또는 논블로킹 방식으로 꺼내서 처리. 
  IOCP의 "큐"에서 이벤트를 꺼내는 핵심 함수.
  **시그니처**
  BOOL GetQueuedCompletionStatus(
  HANDLE CompletionPort,
  LPDWORD lpNumberOfBytesTransferred,
  PULONG_PTR lpCompletionKey,
  LPOVERLAPPED *lpOverlapped,
  DWORD dwMilliseconds
  );

  **사용 예**
  DWORD transferred;
  ULONG_PTR key;
  LPOVERLAPPED overlapped;
  BOOL result = GetQueuedCompletionStatus(iocp, &transferred, &key, &overlapped, INFINITE);

- PostQueuedCompletionStatus: I/O 없이 IOCP에 직접 이벤트 넣음. 보통 쓰레드를 깔끔하게 종료시키기 위해 사용.
  **사용 예**
  PostQueuedCompletionStatus(iocp, 0, 0, NULL); // 스레드에게 "종료 신호" 전송
  - overlapped가 NULL이면 의미 있는 I/O 완료가 아니라는 신호로 간주.
  - Worker Thread가 이 신호를 감지하면 루프에서 빠져나올 수 있음. 

- IOCP 동작 흐름 요약 (Work Flow)
  - CreateIoCompletionPort로 IOCP 생성
  - 클라이언트 소켓을 IOCP에 등록
  - WSARecv 등으로 비동기 I/O 요청
  - Worker Thread가 GetQueuedCompletionStatus로 완료 이벤트 대기
  - I/O 완료 시 IOCP에 통보되고, Worker Thread가 이벤트 수신
  - 필요하면 WSASend 등으로 응답
  - 종료 시 PostQueuedCompletionStatus로 Thread 종료 유도
  - 자원 해제 및 종료
    
- IOCP는 Winodws에서 가장 효율적인 비동기 I/O 모델이므로 현업에서 가장 자주 사용됨 (확실히 알 필요가 있음)
- CreateIoCompletionPort, WSARecv/WSASend, GetQueuedCompletionStatus, PostQueuedCompletionSTatus: 핵심 함수
- 비동기 통신 모델에 대한 철저한 이해 없이 구현할 경우, 데이터 무한 루프, 레이스 컨디션, 리소스 누수 등이 
  발생할 수 있으므로 구조와 큐 처리 방식에 대한 명확한 이해가 필요.
- 또한 Thread Pool을 사용하는 multithread 방식이므로 동기화도 매우 중요!!


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
- 실제 IP 주소를 이용하는 방식으로 변경


## ✨ 학습 포인트
- 실전 TCP/IP 소켓 프로그래밍 경험
- 스레드 동기화 및 데이터 공유 처리
- 사용자 중심의 콘솔 UI 설계
- 네트워크 오류/이벤트 처리

## 🔥 기타
본 프로젝트는 학습 및 연습을 목적으로 진행되었습니다.
코드 수정 및 확장 환영합니다!
