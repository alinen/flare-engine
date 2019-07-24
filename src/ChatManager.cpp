
#include <string>
#include "ChatManager.h"
#include <iostream>
#include "ChatClient.h"
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <vector>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <queue>
#include <map>
#include <cctype>
#include <boost/algorithm/string.hpp>

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#ifdef WIN32
	#include <windows.h> // Ug, for NukeProcess -- see below
#else
	#include <unistd.h>
	#include <signal.h>
#endif

#include "ChatManager.h"
#include "ChatClient.h"

bool g_bQuit = false;

SteamNetworkingMicroseconds g_logTimeZero;

static void NukeProcess( int rc )
{
	#ifdef WIN32
		ExitProcess( rc );
	#else
		kill( getpid(), SIGKILL );
	#endif
}

static void DebugOutput( ESteamNetworkingSocketsDebugOutputType eType, const char *pszMsg )
{
	SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
	printf( "%10.6f %s\n", time*1e-6, pszMsg );
	fflush(stdout);
	if ( eType == k_ESteamNetworkingSocketsDebugOutputType_Bug )
	{
		fflush(stdout);
		fflush(stderr);
		NukeProcess(1);
	}
}

static void FatalError( const char *fmt, ... )
{
	char text[ 2048 ];
	va_list ap;
	va_start( ap, fmt );
	vsprintf( text, fmt, ap );
	va_end(ap);
	char *nl = strchr( text, '\0' ) - 1;
	if ( nl >= text && *nl == '\n' )
		*nl = '\0';
	DebugOutput( k_ESteamNetworkingSocketsDebugOutputType_Bug, text );
}

static void Printf( const char *fmt, ... )
{
	char text[ 2048 ];
	va_list ap;
	va_start( ap, fmt );
	vsprintf( text, fmt, ap );
	va_end(ap);
	char *nl = strchr( text, '\0' ) - 1;
	if ( nl >= text && *nl == '\n' )
		*nl = '\0';
	DebugOutput( k_ESteamNetworkingSocketsDebugOutputType_Msg, text );
}

static void InitSteamDatagramConnectionSockets()
{
	#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
		SteamDatagramErrMsg errMsg;
		if ( !GameNetworkingSockets_Init( nullptr, errMsg ) )
			FatalError( "GameNetworkingSockets_Init failed.  %s", errMsg );
	#else
		SteamDatagramClient_SetAppIDAndUniverse( 570, k_EUniverseDev ); // Just set something, doesn't matter what

		SteamDatagramErrMsg errMsg;
		if ( !SteamDatagramClient_Init( true, errMsg ) )
			FatalError( "SteamDatagramClient_Init failed.  %s", errMsg );

		SteamNetworkingUtils()->SetGlobalConfigValueInt32( k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1 );
	#endif

  g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();

  SteamNetworkingUtils()->SetDebugOutputFunction( k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput );
  }


  static void ShutdownSteamDatagramConnectionSockets()
  {
  	// Give connections time to finish up.  This is an application layer protocol
  	// here, it's not TCP.  Note that if you have an application and you need to be
  	// more sure about cleanup, you won't be able to do this.  You will need to send
  	// a message and then either wait for the peer to close the connection, or
  	// you can pool the connection to see if any reliable data is pending.
  	std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

  	#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
  		GameNetworkingSockets_Kill();
  	#else
  		SteamDatagramClient_Kill();
  	#endif
  }


  std::mutex mutexUserInputQueue;
  std::queue< std::string > queueUserInput;

  std::thread *s_pThreadUserInput = nullptr;
//#include "GameNetworkingSockets/examples/example_chat.cpp"

ChatManager::ChatManager(){
  SteamDatagramErrMsg errMsg;
  if ( !GameNetworkingSockets_Init( nullptr, errMsg ) )
    FatalError( "GameNetworkingSockets_Init failed.  %s", errMsg );
}

ChatManager::~ChatManager(){
}

void ChatManager::registerPlayer(ChatClient* client){
  this->player_info.push_back(client);
//  std:: cout << "ID:" << client->ID() << " Player: "<< client->name() << std::endl;
}
std::string ChatManager::postMessage(int ID, std::string message){
  this->sender_ID = ID;
  std::string fullmessage;
  std::vector<std::string> parsed_message = Parse(message,' ');
  int arraySize = parsed_message.size(); //change to method variable

  if (parsed_message.at(0) == "/whisper"){
    for(int i=2;i<arraySize;i++){
      fullmessage += parsed_message.at(i) + " ";
    }
    recieveMessage(parsed_message.at(1),fullmessage);
  }
  else{
    std::string senderName;
    int sizeOf = this->player_info.size();
     for(int i =0; i<sizeOf;i++){
        if(this->player_info.at(i)->ID() == ID){
          senderName = this->player_info.at(i)->name();
        }
      }
  return message;
  }
}

std::vector<std::string> ChatManager::Parse(std::string message, char delimiter){
    std::vector<std::string> internal;
    std::stringstream ss(message); // Turn the string into a stream.
    std::string tok;

    while(getline(ss, tok, delimiter)) {
      internal.push_back(tok);
    }

    return internal;
  }


 void ChatManager::recieveMessage(std::string username, std::string message){
  std::string senderName;
  int arraySize = this->player_info.size();
  for(int i=0; i<arraySize;i++){
      if (this->player_info.at(i)->name() == username){
          this->reciever_ID = this->player_info.at(i)->ID();
      }
      if(this->player_info.at(i)->ID() == this->sender_ID){
        senderName = this->player_info.at(i)->name();
    }
  }
    //DEBUG find a better way perhaps
      ChatClient* reciever = this->player_info.at(this->reciever_ID-1);
      reciever->recieveMessage(senderName,message);

}
