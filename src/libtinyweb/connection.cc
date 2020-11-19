// #include "connection.h"
// #include <boost/algorithm/string.hpp>
// #include <deque>
// #include <glog/logging.h>
// #include <sstream>
// #include <sys/socket.h>
// #include <vector>

// using std::bind;
// using std::string;
// using std::stringstream;
// using std::vector;
// using std::placeholders::_1;

// void readfromsocket(Connection* conn, coro_t::pull_type& source) {
//   int len;
//   while (true) {
//     while (true) {
//       char buffer[1024];
//       int len = recv(conn->_connection_fd, buffer, 1024, MSG_DONTWAIT);
//       if (len > 0) {
//         string s(buffer, len);
//         conn->HttpHandler(std::ref(s));
//       } else
//         break;
//     }
//     source();
//   }
// }

// void handlehttp(Connection* conn, coro_string_t::pull_type& source) {
//   constexpr int S0 = 0;
//   constexpr int S1 = 1;
//   int state = S0;
//   stringstream ss;
//   HTTPReq req;
//   vector<string> vecSegTag;
//   while (true) {
//     ss << source.get();
//     while (true) {
//       string line;
//       std::getline(ss, line);
//       if (ss.fail()) {
//         ss.clear();
//         ss << line;
//         source();
//         break;
//       } else {
//         switch (state) {
//         case S0:
//           vecSegTag = boost::split(vecSegTag, line, boost::is_space());
//           if (vecSegTag.size() < 3)
//             LOG(ERROR) << "HTTP REQ Error. " << line;
//           else
//             req = HTTPReq(valueOf(vecSegTag[0]), vecSegTag[1]);
//           state = S1;
//           break;
//         case S1:
//           if (line == "\r") {
//             LOG(INFO) << req.toString();
//             state = S0;
//           } else {
//             vecSegTag = boost::split(vecSegTag, line, boost::is_any_of(":"));
//             if (vecSegTag[0] == "Host") {
//               boost::trim_left(vecSegTag[1]);
//               req.source_ip = vecSegTag[1];
//             }
//             string rsp = conn->ProduceRsp(req);
//             if (rsp != "")
//               conn->send_buf.push_front(rsp);
//             conn->WriteToSocket();
//             state = S1;
//           }
//           break;
//         }
//       }
//     }
//   }
// }

// void writetosocket(Connection* conn, coro_t::pull_type& source) {
//   while (true) {
//     source();
//     while (conn->send_buf.size()) {
//       string s = conn->send_buf.back();
//       conn->send_buf.pop_back();
//       int len = send(conn->_connection_fd, s.data(), s.size(), MSG_DONTWAIT);
//       if (len < s.size()) {
//         conn->send_buf.emplace_back(s.data() + len, s.size() - len);
//         break;
//       }
//     }
//   }
// }

// Connection::Connection(int fd)
//     : _connection_fd(fd),                             //
//       ReadFromSocket(bind(readfromsocket, this, _1)), //
//       HttpHandler(bind(handlehttp, this, _1)),        //
//       WriteToSocket(bind(writetosocket, this, _1)) {
// }

// Connection::~Connection() {
//   LOG(INFO) << "Throw";
//   close(_connection_fd);
// }

// std::string Connection::ProduceRsp(const HTTPReq& req) {
//   if (req.req_type == ReqType::GET && req.virtual_dir == "/") {
//     string html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Insert title "
//                   "here</title></head> <body> Helloworld</body></html>";
//     stringstream ss;
//     ss << "HTTP/1.1 200 OK\r\n"
//        << "Server: TinyWeb\r\n"
//        << "Connection:close\r\n"
//        << "Content-Type: text/html; charset=UTF-8\r\n"
//        << "Content-Length: " << html.size() << "\r\n\r\n"
//        << html;
//     return ss.str();
//   }
//   return "";
// }