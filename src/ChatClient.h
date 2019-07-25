
#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <string>

class ValveChatClient;

class ChatClient {
public:
  ChatClient(int ID, std::string name);
  ~ChatClient();

  void logic();
  std::string postMessage(std::string message); //passes message client
  int ID();
  std::string name();
  std::string recieveMessage(std::string sender,std::string message);
  std::vector<std::string> getRemoteChat();

private:
  int player_id;
  std::string player_name;
  ValveChatClient* client;
};

#endif
