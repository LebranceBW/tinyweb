#pragma once

#include "http.h"
#include <boost/coroutine2/all.hpp>
#include <deque>
#include <string>

using coro_t = boost::coroutines2::coroutine<void>;
using coro_string_t = boost::coroutines2::coroutine<std::string>;
class Connection {
public:
  coro_t::push_type ReadFromSocket;
  coro_t::push_type WriteToSocket;
  coro_string_t::push_type HttpHandler;
  int _connection_fd;

  std::deque<std::string> send_buf;
  Connection(int fd);
  Connection() = delete;
  ~Connection();
  std::string ProduceRsp(const HTTPReq& req);
};
