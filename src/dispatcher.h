#pragma once

#include "connection.h"
#include "tcpserver.h"
#include <map>
#include <memory>

class Dispatcher : public TcpEventHandler {
  std::map<int, std::shared_ptr<Connection>> m;

  void onEstablished(int connect_fd) {
    m[connect_fd] = std::shared_ptr<Connection>(new Connection(connect_fd));
  }

  void onReadable(int connect_fd) {
    m[connect_fd]->ReadFromSocket();
  }

  void onWriteable(int connect_fd) {
    m[connect_fd]->WriteToSocket();
  }

  void onClosed(int connect_fd) {
    m.erase(connect_fd);
  }
};