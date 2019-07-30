
#ifndef NETCLIENT_H
#define NETCLIENT_H

#include <string>
#include "Utils.h"

class ValveNetClient;
class NetClient {
public:
  NetClient(int ID, std::string name);
  ~NetClient();

  void logic();
  void postMessage(std::string message); //passes chat message
  void postData(const FPoint& position); //passes data message

  int ID() const;
  const std::string& name() const;
  const std::vector<std::string>& getRemoteChat() const;
  FPoint getRemoteData() const;

private:
  int player_id;
  std::string player_name;
  ValveNetClient* client;
};

#endif
