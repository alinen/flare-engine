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

#define PLATFORM_CPP_INCLUDE

#ifdef _WIN32
#include "PlatformWin32.cpp"
#elif __ANDROID__
#include "PlatformAndroid.cpp"
#elif __IPHONEOS__
#include "PlatformIPhoneOS.cpp"
#elif __GCW0__
#include "PlatformGCW0.cpp"
#elif __EMSCRIPTEN__
#include "PlatformEmscripten.cpp"
bool init_finished = false;
#else
// Linux stuff should work on Mac OSX/BSD/etc, too
#include "PlatformLinux.cpp"
#endif

#include "NetClient.h"
using namespace std;

int main(){
  NetClient client1(1,"Lemon");

  int c;
  FPoint pos(0,0);
  do {
    c = getchar();
    if (c == 'h'){
      client1.postMessage("Hello!");
    }
    else if (c == 'p'){
      client1.postData(pos);
      pos.x += 10;
      pos.y += 10;
    }
   } while (c != 'q');

  return 0;
}
