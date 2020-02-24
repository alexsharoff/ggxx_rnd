// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: DLL library entry point, SteamAPI exports
//
// Initial author: NTAuthority
// Started: 2010-08-29
// ==========================================================

#define STEAM_API_EXPORTS

#include "SteamFriends005.h"
#include "SteamGameServer009.h"
#include "SteamMasterServerUpdater001.h"
#include "SteamMatchmaking007.h"
#include "SteamNetworking004.h"
#include "SteamRemoteStorage002.h"
#include "SteamUser012.h"
#include "SteamUserStats007.h"
#include "SteamUtils005.h"

#include <osw/Steamclient.h>

#include <debug.h>


HSteamPipe GetHSteamPipe()
{
	PRINT_FUNC();
	return NULL;
}
HSteamUser GetHSteamUser()
{
	PRINT_FUNC();
	return NULL;
}
HSteamPipe SteamAPI_GetHSteamPipe()
{
	PRINT_FUNC();
	return NULL;
}
HSteamUser SteamAPI_GetHSteamUser()
{
	PRINT_FUNC();
	return NULL;
}
const char *SteamAPI_GetSteamInstallPath()
{
	PRINT_FUNC();
	return NULL;
}

bool SteamAPI_Init()
{
	PRINT_FUNC();
	return true;
}

bool SteamAPI_InitSafe()
{
	PRINT_FUNC();
	return true;
}

char SteamAPI_RestartApp()
{
	PRINT_FUNC();
	return 1;
}

bool SteamAPI_RestartAppIfNecessary( uint32 unOwnAppID )
{
	PRINT_FUNC();
	return 0;
}

void SteamAPI_RegisterCallResult( CCallbackBase* pResult, SteamAPICall_t APICall )
{
	PRINT_FUNC();
}

void SteamAPI_RegisterCallback( CCallbackBase *pCallback, int iCallback )
{
	PRINT_FUNC();
}
void SteamAPI_RunCallbacks()
{
	PRINT_FUNC();
}
void SteamAPI_SetMiniDumpComment( const char *pchMsg )
{
	PRINT_FUNC();
}

void SteamAPI_SetTryCatchCallbacks( bool bUnknown )
{
	PRINT_FUNC();
}

void SteamAPI_Shutdown()
{
	PRINT_FUNC();
}
void SteamAPI_UnregisterCallResult( CCallbackBase* pResult, SteamAPICall_t APICall )
{
	PRINT_FUNC();
}
void SteamAPI_UnregisterCallback( CCallbackBase *pCallback )
{
	PRINT_FUNC();
}

void SteamAPI_WriteMiniDump( uint32 uStructuredExceptionCode, void* pvExceptionInfo, uint32 uBuildID )
{
	PRINT_FUNC();
}

ISteamApps003* SteamApps()
{
	PRINT_FUNC();
	return NULL;
}
ISteamClient009* SteamClient()
{
	return NULL;
}
ISteamContentServer002* SteamContentServer()
{
	PRINT_FUNC();
	return NULL;
}

ISteamUtils005* SteamContentServerUtils()
{
	PRINT_FUNC();
	return NULL;
}

bool SteamContentServer_Init( unsigned int uLocalIP, unsigned short usPort )
{
	PRINT_FUNC();
	return NULL;
}

void SteamContentServer_RunCallbacks()
{
	PRINT_FUNC();
}

void SteamContentServer_Shutdown()
{
	PRINT_FUNC();
}

ISteamFriends005* SteamFriends()
{
	PRINT_FUNC();
	return (ISteamFriends005*) new CSteamFriends005;
	return NULL;
}

ISteamGameServer010* SteamGameServer()
{
	PRINT_FUNC();
	return (ISteamGameServer010*) new CSteamGameServer009;
	return NULL;
}

ISteamUtils005* SteamGameServerUtils()
{
	PRINT_FUNC();
	return NULL;
}

bool SteamGameServer_BSecure()
{
	PRINT_FUNC();
	return true;
}

HSteamPipe SteamGameServer_GetHSteamPipe()
{
	PRINT_FUNC();
	return NULL;
}

HSteamUser SteamGameServer_GetHSteamUser()
{
	PRINT_FUNC();
	return NULL;
}

int32 SteamGameServer_GetIPCCallCount()
{
	PRINT_FUNC();
	return NULL;
}

uint64 SteamGameServer_GetSteamID()
{
	PRINT_FUNC();
	return NULL;
}

bool SteamGameServer_Init( uint32 unIP, uint16 usPort, uint16 usGamePort, EServerMode eServerMode, int nGameAppId, const char *pchGameDir, const char *pchVersionString )
{
	PRINT_FUNC();
	return true;
}

bool SteamGameServer_InitSafe( uint32 unIP, uint16 usPort, uint16 usGamePort, EServerMode eServerMode, int nGameAppId, const char *pchGameDir, const char *pchVersionString, unsigned long dongs )
{
	PRINT_FUNC();
	return true;
}

void SteamGameServer_RunCallbacks()
{
	PRINT_FUNC();
}

void SteamGameServer_Shutdown()
{
	PRINT_FUNC();
}

ISteamMasterServerUpdater001* SteamMasterServerUpdater()
{
	PRINT_FUNC();
	return (ISteamMasterServerUpdater001*) new CSteamMasterServerUpdater001;
}

ISteamMatchmaking008* SteamMatchmaking()
{
	PRINT_FUNC();
	return (ISteamMatchmaking008*) new CSteamMatchmaking007;
}

ISteamMatchmakingServers002* SteamMatchmakingServers()
{
	PRINT_FUNC();
	return NULL;
}

ISteamNetworking003* SteamNetworking()
{
	PRINT_FUNC();
	return (ISteamNetworking003*) new CSteamNetworking004;
}

ISteamRemoteStorage002* SteamRemoteStorage()
{
	PRINT_FUNC();
	return new CSteamRemoteStorage002;
}

ISteamUser013* SteamUser()
{
	PRINT_FUNC();
	return (ISteamUser013*) new CSteamUser012;
}

ISteamUserStats007* SteamUserStats()
{
	PRINT_FUNC();
	return new CSteamUserStats007();
}

HSteamUser Steam_GetHSteamUserCurrent()
{
	PRINT_FUNC();
	return NULL;
}

void Steam_RegisterInterfaceFuncs( void *hModule )
{
	PRINT_FUNC();
}

void Steam_RunCallbacks( HSteamPipe hSteamPipe, bool bGameServerCallbacks )
{
	PRINT_FUNC();
}

ISteamUtils005* SteamUtils()
{
	PRINT_FUNC();
	return new CSteamUtils005;
}