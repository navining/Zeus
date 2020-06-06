#ifndef _Message_hpp_
#define _Message_hpp_

enum CMD {
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};

struct Header {
	Header() {
		length = sizeof(Header);
		cmd = CMD_ERROR;
	}
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

#endif // !_Message_hpp_