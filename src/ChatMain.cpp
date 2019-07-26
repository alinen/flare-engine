#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <algorithm>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <queue>
#include <map>
#include <cctype>
#include <boost/algorithm/string.hpp>
#include <iostream>

#include "ChatClient.h"
using namespace std;

bool inputAvailable()
{
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  return (FD_ISSET(0, &fds));
}

int main(){
  ChatClient client1(1,"Lemon");

  // int c;
  // do {
  //   //if (inputAvailable()){
  //   //  c = getchar();
  //   //  if (c == 'h'){
  //   //    client1.postMessage("Hello!");
  //   //  }
  //   //}
  //   usleep(100);
  // //  std::cout << c << std::endl;
  //   client1.logic();
  // } while (c != 'q');

  while(true){
    //client1.logic();
    //client1.getRemoteChat();
  }

  return 0;
}
