#include "dispatcher.h"
#include "connection.h"
#include <glog/logging.h>

void Dispatcher::removeIfFinished(Socket_t socket_id, Corot_t corot_id) {
  if (!scheduler.isActive(corot_id)) {
    close(socket_id);
    info_table.erase(socket_id);
  }
}

void Dispatcher::handle(TcpEvent event) {
  Socket_t socket_id = event.socket_id;
  switch (event.type) {
  case EventType::ACCEPTED: {
    auto& info = info_table[socket_id];
    Corot_t corot_id = scheduler.new_coroutine(ConnectionHandler, &info);
    info.corot_id = corot_id;
    info.socket_id = socket_id;
    info.scheduler = &scheduler;
    scheduler.resume(corot_id);
    break;
  }
  case EventType::READABLE: {
    Corot_t corot_id = info_table[socket_id].corot_id;
    scheduler.resume(corot_id);
    removeIfFinished(socket_id, corot_id);
    break;
  }
  case EventType::WRITABLE: {
    Corot_t corot_id = info_table[socket_id].corot_id;
    scheduler.resume(corot_id);
    removeIfFinished(socket_id, corot_id);
    break;
  }
  case EventType::CLOSE:
    close(socket_id);
    break;
  }
}
