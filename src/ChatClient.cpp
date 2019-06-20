


#include "ChatClient.h"
#include <string>
#include <iostream>
#include "ChatManager.h"


ChatClient::ChatClient(int ID, std::string name, ChatManager* ChatManager){
 this->player_id = ID;
 this->player_name = name;
 this->mgr = ChatManager;
 this->mgr->registerPlayer(this);
}

ChatClient::~ChatClient(){

}

std::string ChatClient::postMessage(std::string message){
  mgr->postMessage(this->player_id,message); //parse the channel
  return message;
}
std::string ChatClient::recieveMessage(std::string sender, std::string message){
  return message;
}
int ChatClient::ID(){
  return this->player_id;
}

std::string ChatClient::name(){
  return this->player_name;
}
