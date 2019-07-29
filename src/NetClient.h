
#ifndef NETCLIENT_H
#define NETCLIENT_H

#include <string>

struct ChatMessage
{
  char type; // '0' or '1'
  char padding[7];
  char message[1024];
};

struct PlayerMessage
{
  char type; // '0' or '1'
  char padding[7];
  float x;
  float y;
  float z;
};

class ValveNetClient;
class NetClient {
public:
  NetClient(int ID, std::string name);
  ~NetClient();

  void logic();
  void postMessage(std::string message); //passes chat message


  int ID() const;
  const std::string& name() const;
  const std::vector<std::string>& getRemoteChat() const;

private:
  int player_id;
  std::string player_name;
  ValveNetClient* client;
};

#endif
