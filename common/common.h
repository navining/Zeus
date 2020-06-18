#ifndef _common_h_

#include "Timestamp.hpp"

// Size of the recieve buffer
#define RECV_BUFF_SIZE 10240

// Size of the message buffer (Secondary buffer)
#define MSG_BUFF_SIZE 51200

// Size of the send buffer
#define SEND_BUFF_SIZE 10240

#ifndef _WIN32
#include <signal.h>
struct sigaction sa;
sa.sa_handler = SIG_IGN;
sigaction(SIGPIPE, &sa, 0);
#endif // !_WIN32
 
#endif // !_common_h_
