#pragma once
#include "common.h"

enum EventType { NONE = 0, ACCEPTED, READABLE, WRITABLE, CLOSE };

using Socket_t = int;
struct TcpEvent {
  Socket_t socket_id;
  EventType type;
  TcpEvent(int _id, EventType _type) : socket_id(_id), type(_type) {
  }
};

interface TcpEventManager {
public:
  virtual ~TcpEventManager() = default;

  virtual void handle(TcpEvent event) = 0;
};

class TcpServer {
  int socket_fd;
  int epoll_fd;
  bool hasStarted = false;
  constexpr static int MAX_EVENT_NUM = 65535;
  TcpEventManager* handler;

  void AcceptNewConnection(int socket_fd, int epoll_fd);
  int OpenSocket(char ip[], int port);

public:
  TcpServer(TcpEventManager* handler = nullptr);

  ~TcpServer();

  void StartService(char ip[], int port);

  void StopService();
};