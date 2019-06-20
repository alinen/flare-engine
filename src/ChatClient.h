
#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <string>


class ChatManager;

class ChatClient {
public:
  ChatClient(int ID, std::string name, ChatManager* ChatManager);
  ~ChatClient();
  //void chatClient(int ID, string name, ChatManager* ChatManager);
  std::string postMessage(std::string message); //passes message client
  int ID();
  std::string name();
  std::string recieveMessage(std::string sender,std::string message);

private:
  int player_id;
  std::string player_name;
  ChatManager* mgr;

};

#endif
