#include "dispatcher.h"
#include "tcpserver.h"
#include <glog/logging.h>

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  google::LogToStderr();
  google::SetStderrLogging(0);
  CHECK(argc == 3) << "Error to launch web.";

  Dispatcher dispatcher;
  TcpServer tcp_server(&dispatcher);
  tcp_server.StartService(argv[1], atoi(argv[2]));
}
