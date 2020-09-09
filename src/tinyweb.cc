#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <glog/logging.h>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h> /* See NOTES */

void signal_handler(int signal) {
  switch (signal) {
  case SIGINT:
  case SIGKILL:
    // close(socket_fd);
    break;
  }
}

// enum EVENT_TYPE {
//   NEW_CONNECTION,
//   NEW_INPUT_MESSAGE,
// };

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

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  google::LogToStderr();
  google::SetStderrLogging(0);
  CHECK(argc == 3) << "Error to launch web.";

  int socket_fd = create_socket(argv[1], atoi(argv[2]));
  int epoll_fd = epoll_create1(0);
  CHECK(epoll_fd != -1) << "Error to create epoll. " << strerror(errno);
  struct epoll_event eve = {
      .events = EPOLLIN,
      .data = {.fd = socket_fd},
  };
  struct epoll_event events[256];
  int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &eve);
  CHECK(epoll_fd != -1) << "Error to add epoll fd. " << strerror(errno);
  while (1) {
    int ready = epoll_wait(epoll_fd, events, 256, 1000);
    CHECK(ready >= 0) << "Epoll events error. " << strerror(errno);
    for (int i = 0; i < ready; i++) {
      auto event = events[i];
      if (event.data.fd == socket_fd) {
        struct sockaddr_in* client_addr = new struct sockaddr_in;
        socklen_t addr_len;
        int client_fd = accept4(socket_fd, (sockaddr*)client_addr, &addr_len, SOCK_NONBLOCK);
        if (client_fd < 0)
          LOG(ERROR) << "Bad Connection. " << strerror(errno);
        else {
          LOG(INFO) << "Receive connect from IP:" << inet_ntoa(client_addr->sin_addr);
          struct epoll_event epoll_client_event {
            .events = EPOLLIN | EPOLLOUT, .data = {.fd = client_fd},
          };
          ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &epoll_client_event);
          LOG_IF(ERROR, ret < 0) << "Error to add a new connection to epoll";
        }
      } else if (event.events & EPOLLOUT) {
        send(event.data.fd, "Hello world\n", 12, 0);
        struct epoll_event e = {
            .events = EPOLLIN,
            .data = {.fd = event.data.fd},
        };
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event.data.fd, &e);
      } else if (event.events & EPOLLIN) {
        char buffer[1024];
        int n = read(event.data.fd, buffer, 1024);
        send(event.data.fd, buffer, n, 0);
      }
    }
  }
}
// int connect_fd;
// char recbuf[2048];
// while (1) {
//   if ((connect_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL)) == -1) {
//     printf("accept socket error: %s(error: %d)\n", strerror(errno), errno);
//     continue;
//   }

//   //可以一直保持连接
//   while (1) {
//     //读取客户端发来的信息
//     ssize_t len = read(connect_fd, recbuf, sizeof(recbuf));
//     if (len < 0) {
//       if (errno == EINTR) {
//         continue;
//       }
//       exit(0);
//     }

//     printf("接收客户端的请求：%s\n", recbuf);

//     //向客户端发送信息
//     printf("回复客户端信息：");
//     write(connect_fd, recbuf, strlen(recbuf));
//   }

//   //关闭连接套接字
//   close(connect_fd);
// }

// //关闭监听套接字
// close(socket_fd);
// }