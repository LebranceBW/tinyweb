#pragma once

#include "coroutine_wrapper.h"
#include "http.h"
#include "pystring.h"
#include <fstream>
#include <glog/logging.h>
#include <istream>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
using std::ifstream;
using std::istream;
using std::streambuf;
using std::stringstream;
using std::vector;

class TcpStreamInputBuf : public streambuf {
  char buffer[1024];
  const Scheduler* scheduler;
  const int socket_id;
  bool hasMore = true;

public:
  TcpStreamInputBuf(const Scheduler* sche, const int socket_id)
      : scheduler(sche), socket_id(socket_id) {
  }
  virtual streambuf::int_type underflow() {
    if (hasMore) {
      scheduler->yield();
      int len = recv(socket_id, buffer, 1024, MSG_DONTWAIT);
      if (len <= 0)
        return traits_type::eof();
      hasMore = len == 1024;
      setg(buffer, buffer, buffer + len);
      return traits_type::to_int_type(*gptr());
    } else
      return traits_type::eof();
  }
};

void ConnectionHandler(CoSheduler* _, CoroutineInfo* info) {
  auto sche = info->scheduler;
  auto streambuf = TcpStreamInputBuf(sche, info->socket_id);
  istream is(&streambuf);
  char buf[128];
  is.getline(buf, 128);
  vector<string> pieces;
  string s(buf);
  pystring::split(s, pieces);
  if (pieces.size() != 3)
    LOG(ERROR) << "error header: " << buf;

  HTTPReq req(valueOf(pieces[0]), pieces[1]);
  LOG(INFO) << s;
  while (is)
    is.getline(buf, 128);
  if (req.req_type == ReqType::GET && req.virtual_dir == "/") {
    stringstream ss, tempss;
    ifstream fs("./html/index.html");
    tempss << fs.rdbuf();
    string html = tempss.str();
    ss << "HTTP/1.1 200 OK\r\n"
       << "Server: TinyWeb\r\n"
       << "Connection:close\r\n"
       << "Content-Type: text/html; charset=UTF-8\r\n"
       << "Content-Length: " << html.size() << "\r\n\r\n"
       << html;
    string s = ss.str();
    char* buf = s.data();
    int len = s.size();
    int pos = 0;
    while (pos < len) {
      int l = send(info->socket_id, buf + pos, len - pos, MSG_DONTWAIT);
      if (l < len - pos)
        sche->yield();
      pos += l;
    }
  }
}
