#pragma once
#include <pthread.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

class Connection {
public:
  pthread_t thread;
  pthread_cond_t _read_cond = PTHREAD_COND_INITIALIZER;
  pthread_cond_t _write_cond = PTHREAD_COND_INITIALIZER;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  std::string req_buf;

  int _conn_fd;
  int _epoll_fd;
  Connection(int epoll_fd, int conn_fd);

  void TriggerWrite();
  void TriggerRead();
  std::string ProduceRsp(const std::string& req);

  ~Connection();
};