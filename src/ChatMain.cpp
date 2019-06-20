
#include <string>
#include "ChatManager.h"
#include "ChatClient.h"

int main(){

  ChatManager mgr; //instance of the ChatManager class

  ChatClient client1(1,"Lemon",&mgr);
  ChatClient client2(2,"Apple",&mgr);
  ChatClient client3(3,"Cherry",&mgr);
  ChatClient client4(4,"Goose",&mgr);
  ChatClient client5(5, "Egg",&mgr);
  client3.postMessage("HI");
  client4.postMessage("Hello everyone");

  client5.postMessage("/whisper Apple Popcorn with marshmallows?");
  client1.postMessage("/whisper Goose Hello Benty! how are you?");
  return 0;
}
