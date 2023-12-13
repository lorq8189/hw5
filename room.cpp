#include "guard.h"
#include "message.h"
#include "message_queue.h"
#include "user.h"
#include "room.h"

Room::Room(const std::string &room_name)
  : room_name(room_name) {
  // TODO: initialize the mutex
  pthread_mutex_init(&lock, nullptr);
}


Room::~Room() {
  // TODO: destroy the mutex
  pthread_mutex_destroy(&lock);
}

void Room::add_member(User *user) {
  Guard myguard(lock);
  // TODO: add User to the room
  members.insert(user);
}

void Room::remove_member(User *user) {
  Guard myguard(lock);
  // TODO: remove User from the room
  auto it = members.find(user);
  if (it != members.end()) 
    members.erase(it);  // Remove the User pointer from the set
}

void Room::broadcast_message(const std::string &sender_username, const std::string &message_text) {
  // TODO: send a message to every (receiver) User in the room
  Guard g(lock);
  std::string msgTxt = get_room_name() + ":" + sender_username.substr(0, sender_username.length() - 1) 
  + ":" + message_text.substr(0, message_text.length() - 1);     // remove newline characters

  for (User *user : members) {
    Message *msg = new Message(TAG_DELIVERY, msgTxt);
    user->mqueue.enqueue(msg);
  }
}
 
