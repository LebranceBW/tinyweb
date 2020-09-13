#include "connection.h"
#include <glog/logging.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

using std::string;
using std::stringstream;

bool endsWith(string s, string sub) {
  return s.rfind(sub) == (s.length() - sub.length());
}

void* SimpleHttp(Connection* conn) {
  int len = 0;
  char buffer[4096];
  // do {
  pthread_cond_wait(&conn->_read_cond, &conn->mutex);
  len = recv(conn->_conn_fd, buffer, 4096, MSG_DONTWAIT);
  conn->req_buf += string(buffer, len);
  // } while (!endsWith(conn->req_buf, "\r\n\r\n"));
  string rsp = conn->ProduceRsp(conn->req_buf);

  len = 0;
  const char* ptr = rsp.data();
  // do {
  // pthread_cond_wait(&conn->_write_cond, &conn->mutex);
  int i = send(conn->_conn_fd, ptr + len, rsp.size() - len, SOCK_NONBLOCK);
  ptr += i;
  // } while (ptr - rsp.data() < rsp.size());
  return NULL;
}

Connection::Connection(int epoll_fd, int conn_fd) : _epoll_fd(epoll_fd), _conn_fd(conn_fd) {
  int ret = pthread_create(&thread, NULL, (void* (*)(void*))SimpleHttp, this);
  CHECK(ret == 0) << "Error to create a thread. ";
}

void Connection::TriggerRead() {
  pthread_cond_signal(&_read_cond);
}

void Connection::TriggerWrite() {
  pthread_cond_signal(&_write_cond);
}

Connection::~Connection() {
  pthread_exit(&thread);
  close(_conn_fd);
}

std::string Connection::ProduceRsp(const std::string& req) {
  auto i = req.find('\n');
  LOG(INFO) << req.substr(0, i);
  string html = "<!DOCTYPE html><html>\
                <head><meta charset=\"UTF-8\"><title> Insert title\
                here</title></head>\
                <body>Helloworld</body></html>";
  stringstream ss;
  ss << "HTTP/1.0 200 OK\n"
     << "Server: TinyWeb\n"
     << "Content-Type: text/html; charset=utf-8\n"
     << "Content-Length: " << html.size() << "\n\n"
     << html;
  return ss.str();
}