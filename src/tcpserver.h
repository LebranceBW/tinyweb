#pragma once
#include "common.h"

interface TcpEventHandler {
public:
  virtual ~TcpEventHandler(){};

  virtual void onEstablished(int connect_fd) = 0;

  virtual void onReadable(int connect_fd) = 0;

  virtual void onWriteable(int connect_fd) = 0;

  virtual void onClosed(int connect_fd) = 0;
};

class TcpServer {
  int socket_fd;
  int epoll_fd;
  bool hasStarted = false;
  constexpr static int MAX_EVENT_NUM = 65535;
  TcpEventHandler* handler;

  void AcceptNewConnection(int socket_fd, int epoll_fd);
  int OpenSocket(char ip[], int port);

public:
  TcpServer(TcpEventHandler* handler = nullptr);

  ~TcpServer();

  void StartService(char ip[], int port);

  void StopService();
};