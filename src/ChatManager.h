
#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <string>
#include <vector>

class ChatClient;

class ChatManager{
public:
  ChatManager();
  ~ChatManager();
  void registerPlayer(ChatClient* client);
  std::string postMessage(int ID, std::string message); //passes message to other player
  std::vector<std::string> Parse(std::string message, char delimiter);
  void recieveMessage(std::string username, std::string message);
private:
  std::vector<ChatClient*> player_info; //stores player name and ID #
  int reciever_ID;
  int sender_ID;
  //player_info size variable
};

#endif
