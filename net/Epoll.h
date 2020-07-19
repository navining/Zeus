#ifndef _Epoll_h_
#define _Epoll_h_

#ifdef __linux__

#include "common.h"

class Epoll {
public:
  ~Epoll();

  // Create an epoll fd
  int create(int n = EPOLL_SIZE);

  // Register sockfd and events
  // @param op operation 
  // @param sockfd socket fd
  // @param events events to be mornitored
  int ctl(int op, SOCKET sockfd, uint32_t events);

  // epoll_wait()
  int wait(int timeout);

  // Close epoll
  void close();

  const epoll_event *events();
private:
  epoll_event *_pEvents = nullptr;  // Events array for responce
  int _epfd = -1;  // Epoll fd
  int _nEvents = 0;
};

#endif  // __linux__
#endif  // _Epoll_h_