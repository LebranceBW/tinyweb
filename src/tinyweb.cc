#include "connection.h"
#include <arpa/inet.h>
#include <errno.h>
#include <glog/logging.h>
#include <map>
#include <memory>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>

using std::shared_ptr;
constexpr int MAX_EVENT_NUM = 65535;
std::map<int, shared_ptr<Connection>> m;

int socket_fd, epoll_fd;

bool onRunning = true;
void signal_handler(int signal) {
  onRunning = false;
  LOG(INFO) << "close server";
  close(epoll_fd);
  close(socket_fd);
  for (const auto& iter : m)
    close(iter.first);
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
  while (true) {
    int client_fd = accept4(socket_fd, (sockaddr*)&client_addr, &addr_len, SOCK_NONBLOCK);
    if (client_fd < 0) {
      if (errno != EAGAIN && errno != EWOULDBLOCK)
        LOG(ERROR) << "Bad Connection. " << strerror(errno);
      break;
    } else {
      LOG(INFO) << "Connection established. IP:" << inet_ntoa(client_addr.sin_addr) << ":"
                << client_addr.sin_port;
      m[client_fd] = shared_ptr<Connection>(new Connection(client_fd));
      struct epoll_event epoll_client_event {
        .events = EPOLLIN | EPOLLET, .data = {.fd = client_fd},
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
      .events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET,
      .data = {.fd = socket_fd},
  };
  struct epoll_event events[MAX_EVENT_NUM];
  int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &eve);
  CHECK(ret != -1) << "Error to monitor socket fd. " << strerror(errno);
  while (onRunning) {
    int ready = epoll_wait(epoll_fd, events, MAX_EVENT_NUM, 10);
    CHECK(ready >= 0 || (errno == EINTR)) << "Epoll events error. " << strerror(errno);
    for (int i = 0; i < ready; i++) {
      auto& event = events[i];
      if (event.data.fd == socket_fd)
        accept_new_connection(socket_fd, epoll_fd);
      else if (event.events & EPOLLIN) {
        m[event.data.fd]->ReadFromSocket();
      } else if (event.events & EPOLLOUT) {
        m[event.data.fd]->WriteToSocket();
      } else if (event.events & EPOLLRDHUP) {
        LOG(INFO) << "Connection closed";
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event.data.fd, NULL);
        m.erase(event.data.fd);
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
  signal(SIGINT, signal_handler);
  signal(SIGSTOP, signal_handler);
  signal(SIGKILL, signal_handler);
  start_service(argv[1], atoi(argv[2]));
}
