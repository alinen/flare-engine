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

#include "NetClient.h"
using namespace std;

int main(){
  NetClient client1(1,"Lemon");

  while(true){
    client1.logic();
    const vector<string>& msgs = client1.getRemoteChat();
    for (unsigned int i = 0; i < msgs.size(); i++)
    {
      cout << msgs[i] << endl;
    }
  }

  return 0;
}
