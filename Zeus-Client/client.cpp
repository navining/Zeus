#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
    #include <WinSock2.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <string.h>
    #define SOCKET int
    #define INVALID_SOCKET    (SOCKET)(~0)
    #define SOCKET_ERROR        (-1)
#endif

#include <iostream>
#include <thread>


using namespace std;

enum CMD {
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_USER_JOIN,
    CMD_ERROR
};

struct Header {
    short cmd;
    short length;
};

struct Login : public Header {
    Login() {
        length = sizeof(Login);
        cmd = CMD_LOGIN;
    }
    char username[32];
    char password[32];
};

struct LoginResult : public Header {
    LoginResult() {
        length = sizeof(LoginResult);
        cmd = CMD_LOGIN_RESULT;
        result = 0;
    }
    int result;
};

struct Logout : public Header {
    Logout() {
        length = sizeof(Logout);
        cmd = CMD_LOGOUT;
    }
    char username[32];
};

struct LogoutResult : public Header {
    LogoutResult() {
        length = sizeof(LogoutResult);
        cmd = CMD_LOGOUT_RESULT;
        result = 0;
    }
    int result;
};

struct NewUserJoin : public Header {
    NewUserJoin() {
        length = sizeof(NewUserJoin);
        cmd = CMD_NEW_USER_JOIN;
        sock = 0;
    }
    int sock;
};

int processor(SOCKET _cli) {
    
    // Buffer
    char recvBuf[1024] = {};
    // Recv
    int recvlen = (int)recv(_cli, recvBuf, sizeof(Header), 0);
    Header *_header = (Header *)recvBuf;
    if (recvlen <= 0) {
        cout << "Disconnected" << endl;
        return -1;
    }
    
    switch (_header->cmd) {
        case CMD_LOGIN_RESULT:
        {
            recv(_cli, recvBuf + sizeof(Header), _header->length - sizeof(Header), 0);
            LoginResult* _loginResult = (LoginResult *)recvBuf;
            cout << "Recieve Message: " << _loginResult->cmd << " Data Length: " << _loginResult->length << " Result: " << _loginResult->result << endl;
            break;
        }
        case CMD_LOGOUT_RESULT:
        {
            recv(_cli, recvBuf + sizeof(Header), _header->length - sizeof(Header), 0);
            LogoutResult* _logoutResult = (LogoutResult *)recvBuf;
            cout << "Recieve Message: " << _logoutResult->cmd << " Data Length: " << _logoutResult->length << " Result: " << _logoutResult->result << endl;
            break;
        }
        case CMD_NEW_USER_JOIN:
        {
            recv(_cli, recvBuf + sizeof(Header), _header->length - sizeof(Header), 0);
            NewUserJoin* _userJoin = (NewUserJoin *)recvBuf;
            cout << "Recieve Message: " << _userJoin->cmd << " Data Length: " << _userJoin->length << " New User: " << _userJoin->sock << endl;
            break;
        }
    }
    
    return 0;
}

bool g_exit = false;

void cmdThread(SOCKET _sock) {
    char cmdBuf[256] = {};
    while (!g_exit) {
        scanf("%s", cmdBuf);
        if (0 == strcmp(cmdBuf, "quit")) {
            cout << "Client exits" << endl;
            g_exit = true;
        }
        else if (0 == strcmp(cmdBuf, "login")) {
            Login _login;
            strcpy(_login.username, "Navi");
            strcpy(_login.password, "123456");
            send(_sock, (const char *)&_login, sizeof(Login), 0);
        }
        else if (0 == strcmp(cmdBuf, "logout")) {
            Logout _logout;
            strcpy(_logout.username, "Navi");
            send(_sock, (const char *)&_logout, sizeof(Logout), 0);
        }
        else {
            cout << "Invalid input!" << endl;
        }
    }
}

int main() {
#ifdef _WIN32
    WORD version = MAKEWORD(2, 2);
    WSADATA data;
    WSAStartup(version, &data);
#endif
    // Create socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == _sock) {
        cout << "Create socket - Fail" << endl;
    }
    else {
        cout << "Create socket - Success" << endl;
    }
    
    // Connect
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
#ifdef _WIN32
    _sin.sin_addr.S_un.S_addr = inet_addr("192.168.238.128");
#else
    _sin.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif
    int ret = connect(_sock, (sockaddr *)&_sin, sizeof(sockaddr_in));
    if (SOCKET_ERROR == ret) {
        cout << "Connect - Fail" << endl;
    }
    else {
        cout << "Connect - Success" << endl;
    }
    
    // New thread
    thread _cmd(cmdThread, _sock);
    _cmd.detach();
    
    while (!g_exit) {
        // Select
        fd_set fdRead;
        FD_ZERO(&fdRead);
        FD_SET(_sock, &fdRead);
        timeval t = { 0, 0 };
        int ret = select(_sock + 1, &fdRead, NULL, NULL, &t);
        
        if (ret < 0) {
            cout << "Select quits" << endl;
            break;
        }
        
        // If socket is inside the set
        if (FD_ISSET(_sock, &fdRead)) {
            FD_CLR(_sock, &fdRead);
            // Handle request
            if (-1 == processor(_sock)) {
                break;
            }
        }
    }
    
    // Handle other services
    //cout << "Other services..." << endl;
    
    // Close
#ifdef _WIN32
    closesocket(_sock);
    
    WSACleanup();
#else
    close(_sock);
#endif
    cout << "Quit." << endl;
    getchar();
    return 0;
}

