#include <iostream>
#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"

Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  // call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, m_fd);
}

void Connection::connect(const std::string &hostname, int port) {

  std::string portStr = std::to_string(port);

  // call open_clientfd to connect to the server
  m_fd = open_clientfd(hostname.c_str(), portStr.c_str());
  if (m_fd < 0) {
    std::cerr << "Error: Couldn't connect to server\n";
    exit(1);
  }
  m_last_result = SUCCESS;

  // call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, m_fd);
}

Connection::~Connection() {
  // close the socket if it is open
  if (is_open()) {
    close();
  }
}

bool Connection::is_open() const {
  // return true if the connection is open
  return m_fd >= 0;
}

void Connection::close() {
  // close the connection if it is open
  ::close(m_fd);
  m_fd = -1;
}

bool Connection::send(const Message &msg) {
  // send a message
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  ssize_t bytes_written = rio_writen(m_fd, msg.tag.c_str(), msg.tag.length());
  if (bytes_written < 0) {
      m_last_result = EOF_OR_ERROR;
      return false;
  }

  bytes_written = rio_writen(m_fd, ":", 1);
  if (bytes_written < 0) {
      m_last_result = EOF_OR_ERROR;
      return false;
  }

  bytes_written = rio_writen(m_fd, msg.data.c_str(), msg.data.length());
  if (bytes_written < 0) {
      m_last_result = EOF_OR_ERROR;
      return false;
  }

  bytes_written = rio_writen(m_fd, "\n", 1);
  if (bytes_written < 0) {
      m_last_result = EOF_OR_ERROR;
      return false;
  }

  return true;

  
}

bool Connection::receive(Message &msg) {
  // receive a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  char buffer[Message::MAX_LEN];
  ssize_t bytes_read = rio_readlineb(&m_fdbuf, buffer, Message::MAX_LEN);

  if (bytes_read <= 0) {
    if (bytes_read == 0) {
        m_last_result = EOF_OR_ERROR;
    } else {
        m_last_result = INVALID_MSG;
    }
    return false;
  }

  // Parse the received line into a Message
  msg.parseLineIntoMessage(buffer);
  return true;
}
