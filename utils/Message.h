#ifndef _Message_h_
#define _Message_h_

#define TEST_DATA_SIZE 100

enum CMD {
	CMD_TEST = 6,
	CMD_ERROR
};

struct Message {
	Message() {
		length = sizeof(Message);
		cmd = CMD_ERROR;
	}
	short cmd;
	short length;
};

struct Test : public Message {
	Test() {
		length = sizeof(Test);
		cmd = CMD_TEST;
	}
	char data[TEST_DATA_SIZE - 4];
};
#endif // !_Message_h_