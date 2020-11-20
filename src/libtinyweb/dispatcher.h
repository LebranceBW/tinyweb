#pragma once
#include "coroutine_wrapper.h"
#include "tcpserver.h"
#include <map>

struct CoroutineInfo {
  Corot_t corot_id;
  Socket_t socket_id;
  Scheduler* scheduler;
};

class Dispatcher : public TcpEventManager {
  std::map<Socket_t, CoroutineInfo> info_table;
  Scheduler scheduler;

public:
  virtual void handle(TcpEvent event) override;
  void removeIfFinished(Socket_t socket_id, Corot_t corot_id);
};