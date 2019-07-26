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

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#include "ChatClient.h"
#include <string>
#include <iostream>
#include <unistd.h>
#include <signal.h>

SteamNetworkingMicroseconds g_logTimeZero;
const uint16 DEFAULT_SERVER_PORT = 27020;

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

static void InitSteamDatagramConnectionSockets(){
	#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
		SteamDatagramErrMsg errMsg;
		if ( !GameNetworkingSockets_Init( nullptr, errMsg ) )
			FatalError( "GameNetworkingSockets_Init failed.  %s", errMsg );
	#else
		SteamDatagramClient_SetAppIDAndUniverse( 570, k_EUniverseDev ); // Just set something, doesn't matter what

		SteamDatagramErrMsg errMsg;
		if ( !SteamDatagramClient_Init( true, errMsg ) )
			FatalError( "SteamDatagramClient_Init failed.  %s", errMsg );

		// Disable authentication when running with Steam, for this
		// example, since we're not a real app.
		//
		// Authentication is disabled automatically in the open-source
		// version since we don't have a trusted third party to issue
		// certs.
		SteamNetworkingUtils()->SetGlobalConfigValueInt32( k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1 );
	#endif

	g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();

	SteamNetworkingUtils()->SetDebugOutputFunction( k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput );
}

static void ShutdownSteamDatagramConnectionSockets(){
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


class ValveChatClient : private ISteamNetworkingSocketsCallbacks
{
public:
  ValveChatClient() : m_bQuit(false) {
  }

  virtual ~ValveChatClient() {
    m_pInterface->CloseConnection( m_hConnection, 0, "Goodbye", true );
  }

	void Start( const SteamNetworkingIPAddr &serverAddr )	{
		// Select instance to use.  For now we'll always use the default.
		m_pInterface = SteamNetworkingSockets();

		// Start connecting
		char szAddr[ SteamNetworkingIPAddr::k_cchMaxString ];
		serverAddr.ToString( szAddr, sizeof(szAddr), true );
		Printf( "Connecting to chat server at %s", szAddr );
		m_hConnection = m_pInterface->ConnectByIPAddress( serverAddr );
		if ( m_hConnection == k_HSteamNetConnection_Invalid )
			FatalError( "Failed to create connection" );
	}

  void Tick() {
    PollIncomingMessages();
    PollConnectionStateChanges();
  }

  void SendMessage(const std::string& cmd)  {
    m_pInterface->SendMessageToConnection( m_hConnection,
      cmd.c_str(), (uint32)cmd.length(), k_nSteamNetworkingSend_Reliable );
  }

  std::vector<std::string> getRemoteChat(){
    return remoteMessages;
  }

private:

  std::vector<std::string> remoteMessages;
	HSteamNetConnection m_hConnection;
	ISteamNetworkingSockets *m_pInterface;
  bool m_bQuit;


	void PollIncomingMessages()
	{
		remoteMessages.clear();
		while ( !m_bQuit )
		{
			ISteamNetworkingMessage *pIncomingMsg = nullptr;

			int numMsgs = m_pInterface->ReceiveMessagesOnConnection( m_hConnection, &pIncomingMsg, 1 );
			if ( numMsgs == 0 )
				break;
			if ( numMsgs < 0 )
				FatalError( "Error checking for messages" );

			// Just echo anything we get from the server
      char buffer[1024];
			fwrite( pIncomingMsg->m_pData, 1, pIncomingMsg->m_cbSize, stdout );
			fputc( '\n', stdout );
      strncpy(buffer,(const char *) pIncomingMsg->m_pData,pIncomingMsg->m_cbSize);
			buffer[pIncomingMsg->m_cbSize] = '\0';

			// We don't need this anymore.
      remoteMessages.push_back(buffer);
			pIncomingMsg->Release();
		}
	}

	void PollConnectionStateChanges()	{
		m_pInterface->RunCallbacks( this );
	}

	virtual void OnSteamNetConnectionStatusChanged( SteamNetConnectionStatusChangedCallback_t *pInfo ) override	{
		assert( pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid );

		// What's the state of the connection?
		switch ( pInfo->m_info.m_eState )
		{
			case k_ESteamNetworkingConnectionState_None:
				// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
				break;

			case k_ESteamNetworkingConnectionState_ClosedByPeer:
			case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
			{
				m_bQuit = true;

				// Print an appropriate message
				if ( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting )
				{
					// Note: we could distinguish between a timeout, a rejected connection,
					// or some other transport problem.
					Printf( "We sought the remote host, yet our efforts were met with defeat.  (%s)", pInfo->m_info.m_szEndDebug );
				}
				else if ( pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally )
				{
					Printf( "Alas, troubles beset us; we have lost contact with the host.  (%s)", pInfo->m_info.m_szEndDebug );
				}
				else
				{
					// NOTE: We could check the reason code for a normal disconnection
					Printf( "The host hath bidden us farewell.  (%s)", pInfo->m_info.m_szEndDebug );
				}

				// Clean up the connection.  This is important!
				// The connection is "closed" in the network sense, but
				// it has not been destroyed.  We must close it on our end, too
				// to finish up.  The reason information do not matter in this case,
				// and we cannot linger because it's already closed on the other end,
				// so we just pass 0's.
				m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
				m_hConnection = k_HSteamNetConnection_Invalid;
				break;
			}

			case k_ESteamNetworkingConnectionState_Connecting:
				// We will get this callback when we start connecting.
				// We can ignore this.
				break;

			case k_ESteamNetworkingConnectionState_Connected:
				Printf( "Connected to server OK" );
				break;

			default:
				// Silences -Wswitch
				break;
		}
	}
};


ChatClient::ChatClient(int ID, std::string name){
 this->player_id = ID;
 this->player_name = name;

 SteamNetworkingIPAddr addrServer;
 addrServer.ParseString("127.0.0.1");
 addrServer.m_port = DEFAULT_SERVER_PORT;
 InitSteamDatagramConnectionSockets();
 client = new ValveChatClient();
 client->Start(addrServer);
}

ChatClient::~ChatClient(){
  delete client;
  ShutdownSteamDatagramConnectionSockets();
}

void ChatClient::logic(){
  client->Tick();
}

std::string ChatClient::postMessage(std::string message){
  client->SendMessage(message);
  return message;
}

std::vector<std::string> ChatClient::getRemoteChat(){
  return client->getRemoteChat();
}
std::string ChatClient::recieveMessage(std::string sender, std::string message){
  return message;
}
int ChatClient::ID(){
  return this->player_id;
}

std::string ChatClient::name(){
  return this->player_name;
}
