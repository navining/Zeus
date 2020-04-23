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
#include <vector>

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

vector<SOCKET> g_clients;

int processor(SOCKET _cli) {
    
    // Buffer
    char recvBuf[1024] = {};
    // Recv
    int recvlen = (int)recv(_cli, recvBuf, sizeof(Header), 0);
    Header *_header = (Header *)recvBuf;
    if (recvlen <= 0) {
        cout << "<socket " << _cli << "> " << "exits" << endl;
        return -1;
    }
    
    switch (_header->cmd) {
        case CMD_LOGIN:
        {
            recv(_cli, recvBuf + sizeof(Header), _header->length - sizeof(Header), 0);
            Login* _login = (Login *)recvBuf;
            cout << "<socket " << _cli << "> " << "Command: " << _login->cmd << " Data length: " << _login->length << " Username: " << _login->username << " Password: " << _login->password << endl;
            // Judge username and password
            // Send
            LoginResult _result;
            send(_cli, (char *)&_result, sizeof(LoginResult), 0);
            break;
        }
        case CMD_LOGOUT:
        {
            recv(_cli, recvBuf + sizeof(Header), _header->length - sizeof(Header), 0);
            Logout* _logout = (Logout *)recvBuf;
            cout << "<socket " << _cli << "> " << "Command: " << _logout->cmd << " Data length: " << _logout->length << " Username: " << _logout->username << endl;
            // Send
            LogoutResult _result;
            send(_cli, (char *)&_result, sizeof(LogoutResult), 0);
            break;
        }
        default:
        {
            Header _header = { 0, CMD_ERROR };
            send(_cli, (char *)&_header, sizeof(Header), 0);
        }
    }
    
    return 0;
}

int main() {
#ifdef _WIN32
    WORD version = MAKEWORD(2,2);
    WSADATA data;
    WSAStartup(version, &data);
#endif
    // Create socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == _sock) {
        cout << "Create socket - Fail" << endl;
    }
    else {
        cout << "Create socket - Success" << endl;
    }
    
    // Bind
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
#ifdef _WIN32
    _sin.sin_addr.S_un.S_addr = INADDR_ANY;
#else
    _sin.sin_addr.s_addr = INADDR_ANY;
#endif
    if (SOCKET_ERROR == ::bind(_sock, (sockaddr *)&_sin, sizeof(sockaddr_in))) {
        cout << "Bind - Fail" << endl;
    }
    else {
        cout << "Bind - Success" << endl;
    }
    
    // Listen
    if (SOCKET_ERROR == listen(_sock, 5)) {
        cout << "Listen - Fail" << endl;
    }
    else {
        cout << "Listen - Success" << endl;
    }
    
    while (true) {
        // Select
        fd_set fdRead;
        fd_set fdWrite;
        fd_set fdExcept;
        
        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdExcept);
        
        // Put server sockets inside fd_set
        FD_SET(_sock, &fdRead);
        FD_SET(_sock, &fdWrite);
        FD_SET(_sock, &fdExcept);
        
        // Record curret max socket
        SOCKET maxSock = _sock;
        
        // Put client sockets inside fd_set
        for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
            FD_SET(g_clients[n], &fdRead);
            if (maxSock < g_clients[n]) {
                maxSock = g_clients[n];
            }
        }
        
        timeval t = {0, 0};
        int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExcept, &t);
        
        if (ret < 0) {
            cout << "Select quits" << endl;
            break;
        }
        
        // If socket is inside the set
        if (FD_ISSET(_sock, &fdRead)) {
            FD_CLR(_sock, &fdRead);
            // Accept
            sockaddr_in clientAddr = {};
            int addrlen = sizeof(sockaddr_in);
#ifdef _WIN32
            SOCKET _cli = accept(_sock, (sockaddr *)&clientAddr, &addrlen);
#else
            SOCKET _cli = accept(_sock, (sockaddr *)&clientAddr, (socklen_t *)&addrlen);
#endif
            if (INVALID_SOCKET == _cli) {
                cout << "Invaild client socket" << endl;
                continue;
            }
            
            // Broadcast
            for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
                NewUserJoin userJoin;
                userJoin.sock = _cli;
                send(g_clients[n], (const char *)&userJoin, sizeof(NewUserJoin), 0);
            }
            
            g_clients.push_back(_cli);
            cout << "New client " << _cli << " : " << inet_ntoa(clientAddr.sin_addr) << "-" << clientAddr.sin_port << endl;
        }
        
        // Handle request
        for (int n = (int)g_clients.size() - 1; n >=0; n--) {
            if (FD_ISSET(g_clients[n], &fdRead)) {
                if (-1 == processor(g_clients[n])) {
                    auto it = g_clients.begin() + n;
                    if (it != g_clients.end()) {
                        g_clients.erase(it);
                    }
                }
            }
        }
        
        // Handle other services
        //cout << "Other services..." << endl;
    }
    
    // Close
#ifdef _WIN32
    for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
        closesocket(g_clients[n]);
    }
    closesocket(_sock);
    WSACleanup();
#else
    for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
        close(g_clients[n]);
    }
    close(_sock);
#endif
    getchar();
    return 0;
}


