// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: ISteamUser012 implementation
//
// Initial author: NTAuthority
// Started: 2010-09-10
// ==========================================================

#include "SteamUser012.h"

#include <debug.h>

unsigned int steamID = 0;

HSteamUser CSteamUser012::GetHSteamUser()
{
	PRINT_FUNC();
	return NULL;
}

bool CSteamUser012::LoggedOn()
{
	PRINT_FUNC();
	return false;
}

bool CSteamUser012::BLoggedOn()
{
	PRINT_FUNC();
	return false;
}

CSteamID CSteamUser012::GetSteamID()
{
	PRINT_FUNC();
	return CSteamID(32, 1, k_EUniversePublic, k_EAccountTypeIndividual);
}

int CSteamUser012::InitiateGameConnection( void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure )
{
	PRINT_FUNC();
	return 0;
}

void CSteamUser012::TerminateGameConnection( uint32 unIPServer, uint16 usPortServer )
{
	PRINT_FUNC();
}

void CSteamUser012::TrackAppUsageEvent( CGameID gameID, EAppUsageEvent eAppUsageEvent, const char *pchExtraInfo )
{
	PRINT_FUNC();
}

bool CSteamUser012::GetUserDataFolder( char *pchBuffer, int cubBuffer )
{
	PRINT_FUNC();
	return true;
}

void CSteamUser012::StartVoiceRecording( )
{
	PRINT_FUNC();
}

void CSteamUser012::StopVoiceRecording( )
{
	PRINT_FUNC();
}

EVoiceResult CSteamUser012::GetCompressedVoice( void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten )
{
	PRINT_FUNC();
	return k_EVoiceResultOK;
}

EVoiceResult CSteamUser012::DecompressVoice( void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten )
{
	PRINT_FUNC();
	return k_EVoiceResultOK;
}

HAuthTicket CSteamUser012::GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket )
{
	PRINT_FUNC();
	return 0;
}

EBeginAuthSessionResult CSteamUser012::BeginAuthSession( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID )
{
	PRINT_FUNC();
	return k_EBeginAuthSessionResultOK;
}

void CSteamUser012::EndAuthSession( CSteamID steamID )
{
	PRINT_FUNC();

}

void CSteamUser012::CancelAuthTicket( HAuthTicket hAuthTicket )
{

	PRINT_FUNC();
}

EUserHasLicenseForAppResult CSteamUser012::UserHasLicenseForApp( CSteamID steamID, AppId_t appID )
{
	PRINT_FUNC();
	return k_EUserHasLicenseResultHasLicense;
}
