// 测试Socket层的连接稳定性
#include "dispatcher.h"
#include "tcpserver.h"
#include <arpa/inet.h>
#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

using std::string;
using std::thread;
using namespace ::testing;

class SimpleClient {
public:
  int sockfd = -1;
  SimpleClient(char ip[], int port) {
    int len, result;
    struct sockaddr_in address;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(port);
    len = sizeof(address);
    result = connect(sockfd, (struct sockaddr*)&address, len);
    if (result == -1)
      sockfd = -1;
  }

  ~SimpleClient() {
    if (isOk())
      close(sockfd);
  }

  void client_send(string message) {
    write(sockfd, message.data(), message.size());
  }

  string client_read() {
    string buf(1024, '\0');
    int len = read(sockfd, buf.data(), 1024);
    buf.resize(len);
    return buf;
  }

  bool isOk() {
    return sockfd != -1;
  }
};

TEST(socket, base) {
  //垃圾STL线程库
  // thread t([]() {
  //   Dispatcher dispatcher;
  //   TcpServer tcp_server(&dispatcher);
  //   tcp_server.StartService("127.0.0.1", 8080);
  // });
  // std::this_thread::sleep_for(std::chrono::seconds(2));
  // SimpleClient client("127.0.0.1", 8080);
  // ASSERT_TRUE(client.isOk()) << "errors connect to server";
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
/*
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;
    char ch = 'A';

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(9734);
    len = sizeof(address);
    result = connect(sockfd, (struct sockaddr *)&address, len);

    if (result == -1)
    {
        perror("oops: client1");
        exit(1);
    }
    write(sockfd, &ch, 1);
    read(sockfd, &ch, 1);
    printf("char from server = %c\n", ch);
    close(sockfd);
    exit(0);
}

*/