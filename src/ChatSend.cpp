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

  int c;
  do {
    c = getchar();
    if (c == 'h'){
      client1.postMessage("Hello!");
    }
   } while (c != 'q');

  return 0;
}
