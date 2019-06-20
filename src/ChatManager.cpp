

#include <string>
#include "ChatManager.h"
#include <iostream>
#include "ChatClient.h"
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <vector>

ChatManager::ChatManager(){
}

ChatManager::~ChatManager(){
}

void ChatManager::registerPlayer(ChatClient* client){
  this->player_info.push_back(client);
//  std:: cout << "ID:" << client->ID() << " Player: "<< client->name() << std::endl;
}
std::string ChatManager::postMessage(int ID, std::string message){
  this->sender_ID = ID;
  std::string fullmessage;
  std::vector<std::string> parsed_message = Parse(message,' ');
  int arraySize = parsed_message.size(); //change to method variable

  if (parsed_message.at(0) == "/whisper"){
    for(int i=2;i<arraySize;i++){
      fullmessage += parsed_message.at(i) + " ";
    }
    recieveMessage(parsed_message.at(1),fullmessage);
  }
  else{
    std::string senderName;
    int sizeOf = this->player_info.size();
     for(int i =0; i<sizeOf;i++){
        if(this->player_info.at(i)->ID() == ID){
          senderName = this->player_info.at(i)->name();
        }
      }
  return message;
  }
}

std::vector<std::string> ChatManager::Parse(std::string message, char delimiter){
    std::vector<std::string> internal;
    std::stringstream ss(message); // Turn the string into a stream.
    std::string tok;

    while(getline(ss, tok, delimiter)) {
      internal.push_back(tok);
    }

    return internal;
  }


 void ChatManager::recieveMessage(std::string username, std::string message){
  std::string senderName;
  int arraySize = this->player_info.size();
  for(int i=0; i<arraySize;i++){
      if (this->player_info.at(i)->name() == username){
          this->reciever_ID = this->player_info.at(i)->ID();
      }
      if(this->player_info.at(i)->ID() == this->sender_ID){
        senderName = this->player_info.at(i)->name();
    }
  }
    //DEBUG find a better way perhaps
      ChatClient* reciever = this->player_info.at(this->reciever_ID-1);
      reciever->recieveMessage(senderName,message);

}
