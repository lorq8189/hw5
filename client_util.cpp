#include <iostream>
#include <string>
#include "connection.h"
#include "message.h"
#include "client_util.h"

// string trim functions shamelessly stolen from
// https://www.techiedelight.com/trim-string-cpp-remove-leading-trailing-spaces/

const std::string WHITESPACE = " \n\r\t\f\v";

std::string ltrim(const std::string &s) {
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}
 
std::string rtrim(const std::string &s) {
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
 
std::string trim(const std::string &s) {
  return rtrim(ltrim(s));
}

Message createSenderMsg(std::string &msg) {
  // trim the message
  msg = trim(msg);

  Message senderMsg;

  if(msg[0] == '/') { // handle command
    std::string substring = msg.substr(1);
    if (substring == "leave") {
      senderMsg.tag = TAG_LEAVE;
    } else if (substring == "quit") {
      senderMsg.tag = TAG_QUIT;
    } else if (substring.substr(0,4) == "join") {
      size_t spacePos = substring.find(' ');
      senderMsg.tag = TAG_JOIN;
      senderMsg.data = substring.substr(spacePos+1);
    } else {
      std::cerr << "Error: Invalid Commands!\n";
    }
    return senderMsg;
  } 

  // message is reaching maximum
  if(msg.length() > Message::MAX_LEN) {
    std::cerr << "Error: Invalid Commands!\n";
    return senderMsg;
  }

  senderMsg.tag = TAG_SENDALL;
  senderMsg.data = msg;

  return senderMsg;
}

