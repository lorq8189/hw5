#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  
  std::string server_hostname;  //initializing variables
  int server_port;
  std::string username;

  server_hostname = argv[1];
  server_port = std::stoi(argv[2]);
  username = argv[3];

  // connect to server
  Connection serverConnection;
  serverConnection.connect(server_hostname, server_port);

  // send slogin message
  Message senderMsg(TAG_SLOGIN, username);
  serverConnection.send(senderMsg);

  Message serverResponse;
  if(!serverConnection.receive(serverResponse)) {
    std::cerr << "Error: " << serverConnection.get_last_result() << "\n";
    serverConnection.close();
    exit(1);
  }
  if(serverResponse.tag == TAG_ERR) {     //error handling block for the server's errors
    std::cerr << serverResponse.data;
    serverConnection.close();
    exit(1);
  }

  std::string userInput;
  // loop reading commands from user, sending messages to
  while(serverConnection.is_open()) {
    std::getline(std::cin, userInput);

    senderMsg = createSenderMsg(userInput);
    if (senderMsg.tag.empty()) { // invalid command/message
      continue;
    }
    serverConnection.send(senderMsg);     

    if(!serverConnection.receive(serverResponse)) {   //case for if server responds back with an error or not at all
      std::cerr << "Error: " << serverConnection.get_last_result() << "\n";
      serverConnection.close();
      exit(1);
    }
    if(serverResponse.tag == TAG_ERR) 
      std::cerr << serverResponse.data;
    if(senderMsg.tag == TAG_QUIT)      //quit block
      break;
  }

  serverConnection.close();
  return 0;     //succesful end case
}
