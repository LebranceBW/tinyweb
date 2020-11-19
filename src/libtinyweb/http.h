#pragma once

#include <string>
using std::string;

enum class ReqType { GET, POST, UNKNOWN };

inline ReqType valueOf(string req) {
  if (req == "GET")
    return ReqType::GET;
  else if (req == "POST")
    return ReqType::POST;
  else
    return ReqType::UNKNOWN;
}

inline string ReqTypeToString(ReqType rt) {
  switch (rt) {
  case ReqType::GET:
    return "GET";
  case ReqType::POST:
    return "POST";
  default:
    return "UNKNOWN";
  }
}

class HTTPReq {
public:
  ReqType req_type;
  std::string virtual_dir;
  std::string source_ip;

  HTTPReq() {
    req_type = ReqType::UNKNOWN;
    virtual_dir = "";
    source_ip = "";
  }

  HTTPReq(ReqType _req_type, std::string vd, std::string ip = "None") {
    req_type = _req_type;
    virtual_dir = vd;
    source_ip = ip;
  }

  string toString() {
    return ReqTypeToString(req_type) + " from " + source_ip + " " + virtual_dir;
  }
};

class HTTPRsp {

};

class HTTPHandler {
  static HTTPRsp handler(HTTPReq req);
};