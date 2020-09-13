#include "connection.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <glog/logging.h>
#include <iostream>
#include <map>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

std::map<int, Connection*> m;

int socket_fd, epoll_fd;

bool onRunning = true;
void signal_handler(int signal) {
  switch (signal) {
  case SIGINT:
  case SIGKILL:
    onRunning = false;
    LOG(INFO) << "close server";
    close(socket_fd);
    close(epoll_fd);
    for (auto iter : m) {
      delete iter.second;
    }
    break;
  }
}

int create_socket(char ip[], int port) {
  int socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  CHECK(socket_fd != -1) << "Error to create socket. " << strerror(errno);
  struct sockaddr_in server_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
  };
  server_addr.sin_addr.s_addr = inet_addr(ip);
  int ret = bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in));
  CHECK(ret != -1) << "Error to bind server to " << ip << ":" << port << ". " << strerror(errno);
  ret = listen(socket_fd, 10);
  CHECK(ret != -1) << "Error to listen to socket. " << strerror(errno);
  return socket_fd;
}

void accept_new_connection(int socket_fd, int epoll_fd) {
  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(struct sockaddr_in);
  while (1) {
    int client_fd = accept4(socket_fd, (sockaddr*)&client_addr, &addr_len, SOCK_NONBLOCK);
    if (client_fd < 0) {
      if (errno != EAGAIN && errno != EWOULDBLOCK)
        LOG(ERROR) << "Bad Connection. " << strerror(errno);
      break;
    } else {
      LOG(INFO) << "Receive connect from IP:" << inet_ntoa(client_addr.sin_addr) << ":"
                << client_addr.sin_port;
      m[client_fd] = new Connection(epoll_fd, client_fd);
      struct epoll_event epoll_client_event {
        .events = EPOLLIN | EPOLLET | EPOLLHUP, .data = {.fd = client_fd},
      };
      int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &epoll_client_event);
      LOG_IF(ERROR, ret < 0) << "Error to add a new connection to epoll" << strerror(errno);
    }
  }
}

int start_service(char ip[], int port) {
  socket_fd = create_socket(ip, port);
  epoll_fd = epoll_create1(0);
  CHECK(epoll_fd != -1) << "Error to create epoll. " << strerror(errno);
  struct epoll_event eve = {
      .events = EPOLLIN | EPOLLOUT | EPOLLET,
      .data = {.fd = socket_fd},
  };
  signal(SIGINT, signal_handler);
  signal(SIGSTOP, signal_handler);
  signal(SIGKILL, signal_handler);
  struct epoll_event events[256];
  int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &eve);
  CHECK(epoll_fd != -1) << "Error to add epoll fd. " << strerror(errno);
  while (onRunning) {
    int ready = epoll_wait(epoll_fd, events, 256, -1);
    CHECK(ready >= 0 || (errno == EINTR)) << "Epoll events error. " << strerror(errno);
    for (int i = 0; i < ready; i++) {
      auto event = events[i];
      if (event.data.fd == socket_fd)
        accept_new_connection(socket_fd, epoll_fd);
      else if (event.events & EPOLLRDHUP) {
        LOG(INFO) << "Connection closed";
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event.data.fd, NULL);
        delete m[event.data.fd];
      } else if (event.events & EPOLLIN) {
        m[event.data.fd]->TriggerRead();
      } else if (event.events & EPOLLOUT) {
        m[event.data.fd]->TriggerWrite();
      }
    }
  }
  return 0;
}

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  google::LogToStderr();
  google::SetStderrLogging(0);
  CHECK(argc == 3) << "Error to launch web.";
  start_service(argv[1], atoi(argv[2]));
}
