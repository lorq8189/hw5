#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"
using std::string;


int main(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  std::string server_hostname = argv[1]; //initializing variables
  int port = std::stoi(argv[2]);
  std::string username = argv[3];
  std::string room = argv[4];
  Message response;

  // TODO: connect to server
  Connection serverConnection;
  serverConnection.connect(server_hostname, port);

  // TODO: send rlogin and join messages (expect a response from
  //       the server for each one)
  Message receiverLog(TAG_RLOGIN, username);  //rlogin
  serverConnection.send(receiverLog);

  if(!serverConnection.receive(response)) { //error handling for rlogin
    std::cerr << "Error: " << serverConnection.get_last_result() << "\n";
    serverConnection.close();
    exit(1);
  }
  if(response.tag == TAG_ERR) {
    std::cerr << response.data;
    serverConnection.close();
    exit(1);
  }


  Message receiverJoin(TAG_JOIN, room); //joining
  serverConnection.send(receiverJoin);

  if(!serverConnection.receive(response)) { //error handling for join
    std::cerr << "Error: " << serverConnection.get_last_result() << "\n";
    serverConnection.close();
    exit(1);
  }
  if(response.tag == TAG_ERR) {
    std::cerr << response.data;
    serverConnection.close();
    exit(1);
  }

  // TODO: loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)



  while(serverConnection.receive(response)) {
    if(response.tag == TAG_ERR) {                     //error handling from server error
      std::cerr << response.data;
      serverConnection.close();
      exit(1);
    }
    
    if(response.tag == TAG_DELIVERY) {                  //prints any delivery message 
      Message TempMessage;
      TempMessage.parseRLineIntoMessage(response.data);
      std::cout << TempMessage.tag << ": " << TempMessage.data;
    }
    else if (response.tag != TAG_OK)                    //if its not ok or delivery, some weird mixup happened with the server
      std::cerr << "irregular error: " << response.data;
  }

  std::cerr << "Error: " << serverConnection.get_last_result() << "\n"; //case for if server stops communicating
  serverConnection.close();
  exit(1);
}

