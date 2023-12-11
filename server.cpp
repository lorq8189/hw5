#include <pthread.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <set>
#include <vector>
#include <cctype>
#include <cassert>
#include "message.h"
#include "connection.h"
#include "user.h"
#include "room.h"
#include "guard.h"
#include "server.h"
#include "client_util.h"

////////////////////////////////////////////////////////////////////////
// Server implementation data types
////////////////////////////////////////////////////////////////////////

// add any additional data types that might be helpful
//       for implementing the Server member functions

struct ConnInfo {
    Server& server;
    int clientfd;
    Connection connection;
    bool sender;
    bool receiver;
    User *user;
    Room *room;

    // Constructor with parameters for ConnInfo
    ConnInfo(Server& server, int fd) : server(server), clientfd(fd), connection(fd), room(nullptr) {
        
    }

    ~ConnInfo() {
      delete user;
      connection.close();
    }
};

////////////////////////////////////////////////////////////////////////
// Client thread functions
////////////////////////////////////////////////////////////////////////

namespace {

void clear(ConnInfo *info) {

  // leave the room
  if (info->room != nullptr) {
    info->room->remove_member(info->user);
  }
  // clear all memory
  info->connection.close();
  return;
}

bool generate_response(ConnInfo *info, const std::string &tag, const std::string &msgTxt) {
  Message msg(tag, msgTxt);
  if (!info->connection.send(msg)) { // lose connection
    return false;
  }
  return true;
}

void chat_with_receiver(ConnInfo *info) {
  
  Message msg, responseMsg;

  while(true) {
    if(!info->connection.receive(msg)) {
      std::cerr<<"Error in receiving!"<<std::endl;
      continue;
    }

    if (msg.tag == TAG_JOIN) {

      Room *room = info->server.find_or_create_room(msg.data); // find the room
      info->room = room;
      // add user to the room
      room->add_member(info->user);
      generate_response(info, TAG_OK, "Joined room successfully!");
      break;

    } else {
      generate_response(info, TAG_ERR, "Invalid Command!");
    }
  }

  while(true) {
    Message *sendMsg = info->user->mqueue.dequeue();
    if (!info->connection.send(*sendMsg)) { // lose connection
      delete sendMsg;
      clear(info);
      return;
    }
    delete sendMsg;
  }
}

void chat_with_sender(ConnInfo *info) {

  Message responseMsg;
  Message receiveMsg;
  while(true) {
    if (!info->connection.receive(receiveMsg)) {
      // error
    }
    if (receiveMsg.tag == TAG_JOIN) {

      if (info->room != nullptr) {
        info->room->remove_member(info->user); // quit the current room first
      }
    
      Room *room = info->server.find_or_create_room(receiveMsg.data);
      info->room = room;
      // add user to the room
      room->add_member(info->user); 

      generate_response(info, TAG_OK, "Joined the room successfully");
    } else if (receiveMsg.tag == TAG_QUIT) {
      generate_response(info, TAG_OK, "Quit successfully!");
      clear(info);
      return;
    } else if (receiveMsg.tag == TAG_LEAVE) {

      // find the room
      if (info->room == nullptr) {
        generate_response(info, TAG_ERR, "No room found!");
        continue;
      }
      info->room->remove_member(info->user); // remove user from the room
      info->room = nullptr;
      generate_response(info, TAG_OK, "Leave the room successfully!");
    } else if (receiveMsg.tag == TAG_SENDALL) {
      info->room->broadcast_message(info->user->username, receiveMsg.data);
      generate_response(info, TAG_OK, "Message sent successfully");
    } else {
      generate_response(info, TAG_ERR, "Invalid Command!");
    }
  }
}

void *worker(void *arg) {
  pthread_detach(pthread_self());

  // use a static cast to convert arg from a void* to
  //       whatever pointer type describes the object(s) needed
  //       to communicate with a client (sender or receiver)
  ConnInfo *info = static_cast<ConnInfo*>(arg);

  // read login message (should be tagged either with
  //       TAG_SLOGIN or TAG_RLOGIN), send response
  Message loginMessage;

  if (!info->connection.receive(loginMessage)) {
    std::cerr << "Error reading login message\n";
  } 
  User *user = new User(loginMessage.data);
  info->user = user;

  Message sendMsg(TAG_OK, "login successfully!"); 
  if (!generate_response(info, TAG_OK, "login successfully!")) { // lose connection
    delete info;
    return nullptr;
  }

  if (loginMessage.tag == TAG_RLOGIN) {
    chat_with_receiver(info);
  }
  if (loginMessage.tag == TAG_SLOGIN) {
    chat_with_sender(info);
  }

  delete info;
  return nullptr;
}

}

////////////////////////////////////////////////////////////////////////
// Server member function implementation
////////////////////////////////////////////////////////////////////////

Server::Server(int port)
  : m_port(port)
  , m_ssock(-1) {
  // initialize mutex
  pthread_mutex_init(&m_lock, nullptr);
}

Server::~Server() {
  // destroy mutex
  pthread_mutex_destroy(&m_lock);

  // Close the server socket if it was open
  if (m_ssock != -1) {
      // Close the server socket
      close(m_ssock);
  }
  // remove rooms
  for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it) {
      delete it->second;  // Delete the Room pointer
  }
}

bool Server::listen() {
  // use open_listenfd to create the server socket, return true
  // if successful, false if not

  std::string portStr = std::to_string(m_port);
  m_ssock = open_listenfd(portStr.c_str());
  if (m_ssock < 0) {
    return false;
  }

  return true;
}

void Server::handle_client_requests() {
  // infinite loop calling accept or Accept, starting a new
  // pthread for each connected client

  while (1) {
    int clientfd = Accept(m_ssock, NULL, NULL);
    if (clientfd < 0) {
      std::cerr << "Error accepting client connection\n";
      continue;
    }

    ConnInfo *info = new ConnInfo(*this, clientfd);

    pthread_t thr_id;
    if (pthread_create(&thr_id, NULL, worker, static_cast<void*>(info)) != 0) {
      std::cerr << "pthread_create failed\n";
      delete info;
    }
    
  }
}

Room *Server::find_or_create_room(const std::string &room_name) {
  // return a pointer to the unique Room object representing
  //       the named chat room, creating a new one if necessary

  // Lock the mutex to ensure thread safety
  Guard myguard(m_lock);
  std::string new_room_name = room_name.substr(0, room_name.length() - 1); // remove newline character
  RoomMap::iterator it = m_rooms.find(new_room_name);

  // room is found
  if (it != m_rooms.end()) {
      Room *existingRoom = it->second;
      return existingRoom;
  }

  // create a new room if it's not found
  Room *newRoom = new Room(new_room_name);
  m_rooms[new_room_name] = newRoom;

  return newRoom;

}
