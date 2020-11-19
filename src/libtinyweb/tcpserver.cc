#include "tcpserver.h"
#include <arpa/inet.h>
#include <errno.h>
#include <glog/logging.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>

TcpServer::TcpServer(TcpEventManager* handler) : handler(handler) {
}

TcpServer::~TcpServer() {
  StopService();
}

void TcpServer::StartService(char ip[], int port) {
  CHECK(!hasStarted) << "Tcp server has been started.";
  hasStarted = true;
  socket_fd = OpenSocket(ip, port);
  epoll_fd = epoll_create1(0);
  CHECK(epoll_fd != -1) << "Error to create epoll. " << strerror(errno);

  struct epoll_event eve = {
      .events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET,
      .data = {.fd = socket_fd},
  };
  struct epoll_event events[MAX_EVENT_NUM];
  int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &eve);
  CHECK(ret != -1) << "Error to monitor socket. " << strerror(errno);

  while (hasStarted) {
    int ready = epoll_wait(epoll_fd, events, MAX_EVENT_NUM, 10);
    CHECK(ready >= 0 || (errno == EINTR)) << "Epoll events error. " << strerror(errno);
    for (int i = 0; i < ready; i++) {
      auto& event = events[i];
      if (event.data.fd == socket_fd)
        AcceptNewConnection(socket_fd, epoll_fd);
      else if (event.events & EPOLLRDHUP) {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event.data.fd, NULL);
        handler -> handle(TcpEvent(event.data.fd, EventType::CLOSE));
      } else if (event.events & EPOLLIN) {
        handler -> handle(TcpEvent(event.data.fd, EventType::READABLE));
      } else if (event.events & EPOLLOUT) {
        handler -> handle(TcpEvent(event.data.fd, EventType::WRITABLE));
      }
    }
  }
}

void TcpServer::AcceptNewConnection(int socket_fd, int epoll_fd) {
  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(struct sockaddr_in);
  while (true) {
    int client_fd = accept4(socket_fd, (sockaddr*)&client_addr, &addr_len, SOCK_NONBLOCK);
    if (client_fd < 0) {
      if (errno != EAGAIN && errno != EWOULDBLOCK)
        LOG(ERROR) << "Bad Connection. " << strerror(errno);
      break;
    } else {
      LOG(INFO) << "Connection established. IP:" << inet_ntoa(client_addr.sin_addr) << ":"
                << client_addr.sin_port;
      handler -> handle({client_fd, EventType::ACCEPTED});
      struct epoll_event epoll_client_event {
        .events = EPOLLIN | EPOLLET, .data = {.fd = client_fd},
      };
      int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &epoll_client_event);
      LOG_IF(ERROR, ret < 0) << "Error to add a new connection to epoll" << strerror(errno);
    }
  }
}

int TcpServer::OpenSocket(char ip[], int port) {
  int socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  CHECK(socket_fd != -1) << "Error to create socket. " << strerror(errno);
  struct sockaddr_in server_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
  };
  server_addr.sin_addr.s_addr = inet_addr(ip);
  int ret = bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in));
  CHECK(ret != -1) << "Error to bind server to " << ip << ":" << port << ". " << strerror(errno);
  ret = listen(socket_fd, 100);
  CHECK(ret != -1) << "Error to listen to socket. " << strerror(errno);
  return socket_fd;
}

void TcpServer::StopService() {
  hasStarted = false;
  close(epoll_fd);
  close(socket_fd);
}