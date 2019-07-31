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

#include "NetClient.h"
#include <string>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <signal.h>

SteamNetworkingMicroseconds g_logTimeZero;
const uint16 DEFAULT_SERVER_PORT = 27020;

static void DebugOutput( ESteamNetworkingSocketsDebugOutputType eType, const char *pszMsg )
{
    SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
    printf( "%10.6f %s\n", time*1e-6, pszMsg );
    fflush(stdout);
    if ( eType == k_ESteamNetworkingSocketsDebugOutputType_Bug )
    {
        fflush(stdout);
        fflush(stderr);
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


class ValveNetClient : private ISteamNetworkingSocketsCallbacks
{
public:
  ValveNetClient() : m_bQuit(false), m_pInterface(NULL) {
    remoteData.x = 20;
    remoteData.y = 20;
  }

  virtual ~ValveNetClient() {
    m_pInterface->CloseConnection( m_hConnection, 0, "Goodbye", true );
    m_hConnection = k_HSteamNetConnection_Invalid;
  }

    void Start( const SteamNetworkingIPAddr &serverAddr )	{
        // Select instance to use.  For now we'll always use the default.
        m_pInterface = SteamNetworkingSockets();

        // Start connecting
        char szAddr[ SteamNetworkingIPAddr::k_cchMaxString ];
        serverAddr.ToString( szAddr, sizeof(szAddr), true );
        Printf( "Connecting to chat server at %s", szAddr );
        m_hConnection = m_pInterface->ConnectByIPAddress( serverAddr );
        if ( m_hConnection == k_HSteamNetConnection_Invalid ) {
            FatalError( "Failed to create connection" );
        }
    }

  void Tick() {
    PollIncomingMessages();
    PollConnectionStateChanges();
  }

  void SendMessage(const std::string& cmd)  {
    if (m_pInterface && m_hConnection != k_HSteamNetConnection_Invalid ) {
        m_pInterface->SendMessageToConnection( m_hConnection,
            cmd.c_str(), (uint32) cmd.length(), k_nSteamNetworkingSend_Reliable );
    }
  }

  void SendData(const FPoint& pos){
    if (m_pInterface && m_hConnection != k_HSteamNetConnection_Invalid ) {

        char cmd[1024];
        snprintf(cmd, 1024, "/data %f,%f", pos.x, pos.y); // TODO: Manage names
        m_pInterface->SendMessageToConnection( m_hConnection,
            cmd, (uint32) strlen(cmd), k_nSteamNetworkingSend_Reliable );
    }
  }

  const std::vector<std::string>& getRemoteChat() const{
    return remoteMessages;
  }

  const FPoint& getRemoteData() const{
    return remoteData;
  }

private:

  std::vector<std::string> remoteMessages;
  FPoint remoteData;
  HSteamNetConnection m_hConnection;
  bool m_bQuit;
  ISteamNetworkingSockets *m_pInterface;

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

            // '\0'-terminate it to make it easier to parse
            std::string sCmd;
            sCmd.assign( (const char *)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize );

            size_t idx = sCmd.find("/data");
            if (idx != std::string::npos)
            {
                float x, y;
                std::vector<std::string> tokens = split(sCmd.substr(idx+5, std::string::npos), ',');
                x = ::atof(tokens[0].c_str());
                y = ::atof(tokens[1].c_str());
                remoteData.x = x;
                remoteData.y = y;

            }
            else
            {
                // Just echo anything we get from the server
                //fwrite( sCmd.c_str(), 1, sCmd.length(), stdout);
                //fputc( '\n', stdout );
                remoteMessages.push_back(sCmd);
            }
            pIncomingMsg->Release();
        }
    }

    std::vector<std::string> split(const std::string &s, char delim) {
      std::stringstream ss(s);
      std::string item;
      std::vector<std::string> elems;
      while (std::getline(ss, item, delim)) {
        elems.push_back(item);
        // elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
      }
      return elems;
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


NetClient::NetClient(int ID, std::string name){
 this->player_id = ID;
 this->player_name = name;

 SteamNetworkingIPAddr addrServer;
 addrServer.ParseString("127.0.0.1");
 addrServer.m_port = DEFAULT_SERVER_PORT;
 InitSteamDatagramConnectionSockets();
 client = new ValveNetClient();
 client->Start(addrServer);
}

NetClient::~NetClient(){
  delete client;
  ShutdownSteamDatagramConnectionSockets();
}

void NetClient::logic(){
  client->Tick();
}

void NetClient::postMessage(std::string message){
  client->SendMessage(message);
}

void NetClient::postData(const FPoint& pos){
  client->SendData(pos);
}

const std::vector<std::string>& NetClient::getRemoteChat() const{
  return client->getRemoteChat();
}

const FPoint& NetClient::getRemoteData() const{
  return client->getRemoteData();
}

int NetClient::ID() const{
  return this->player_id;
}

const std::string& NetClient::name() const{
  return this->player_name;
}
