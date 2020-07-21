#include "Epoll.h"

Epoll::~Epoll() {
  close();
}

int Epoll::create(int n) {
  if (_epfd > 0) {
    LOG_WARNING("Epoll::create() - close old fd\n");
    close();
  }

  _nEvents = n;
  _epfd = epoll_create(n);
  if (EPOLL_ERROR == _epfd) {
    LOG_PERROR("Epoll::create() - Fail\n");
  }
  _pEvents = new epoll_event[n];
  return _epfd;
}

int Epoll::ctl(int op, SOCKET sockfd, uint32_t events) {
  epoll_event ev;
  ev.events = events;
  ev.data.fd = sockfd;
  int ret = epoll_ctl(_epfd, op, sockfd, &ev);
  if (EPOLL_ERROR == ret) {
    LOG_PERROR("Epoll::ctl() - Fail\n");
  }
  return ret;
}

int Epoll::wait(int timeout) {
  int ret = epoll_wait(_epfd, _pEvents, _nEvents, timeout);
  if (EPOLL_ERROR == ret) {
    LOG_PERROR("Epoll:wait() - Fail\n");
  }
  return ret;
}

void Epoll::close() {
  if (_epfd > 0) {
    _epfd = -1;
  }

  if (_pEvents != nullptr) {
    delete[] _pEvents;
    _pEvents = nullptr;
  }

}

const epoll_event *Epoll::events() {
  return _pEvents;
}