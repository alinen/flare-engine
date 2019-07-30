
#ifndef NETCLIENT_H
#define NETCLIENT_H

#include <string>

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
