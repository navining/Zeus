#ifndef _Message_h_
#define _Message_h_

#define TEST_DATA_SIZE 100

enum MESSAGE_TYPE {
	STREAM,
	CMD_TEST,
	CMD_ERROR
};

struct Message {
	Message() {
		length = sizeof(Message);
		type = CMD_ERROR;
	}
	short type;
	short length;
};

struct Test : public Message {
	Test() {
		length = sizeof(Test);
		type = CMD_TEST;
	}
	char data[TEST_DATA_SIZE - 4];
};
#endif // !_Message_h_