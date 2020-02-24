#include "SteamUserStats007.h"

#include <debug.h>

bool CSteamUserStats007::RequestCurrentStats()
{
	PRINT_FUNC();
	return true;
}


bool CSteamUserStats007::GetStat( const char *pchName, float *pData )
{
	PRINT_FUNC();
	return false;
}

bool CSteamUserStats007::GetStat( const char *pchName, int32 *pData )
{
	PRINT_FUNC();
	return false;
}


bool CSteamUserStats007::SetStat( const char *pchName, float fData )
{
	PRINT_FUNC();
	return false;
}
bool CSteamUserStats007::SetStat( const char *pchName, int32 nData )
{
	PRINT_FUNC();
	return false;
}

bool CSteamUserStats007::UpdateAvgRateStat( const char *pchName, float flCountThisSession, double dSessionLength )
{
	PRINT_FUNC();
	return false;
}


bool CSteamUserStats007::GetAchievement( const char *pchName, bool *pbAchieved )
{
	PRINT_FUNC();
	return false;
}
bool CSteamUserStats007::SetAchievement( const char *pchName )
{
	PRINT_FUNC();
	return false;
}
bool CSteamUserStats007::ClearAchievement( const char *pchName )
{
	PRINT_FUNC();
	return false;
}


bool CSteamUserStats007::GetAchievementAndUnlockTime( const char *pchName, bool *pbAchieved, RTime32 *prtTime )
{
	PRINT_FUNC();
	return false;
}

bool CSteamUserStats007::StoreStats()
{
	PRINT_FUNC();
	return true;
}


int CSteamUserStats007::GetAchievementIcon( const char *pchName )
{
	PRINT_FUNC();
	return 0;
}

const char * CSteamUserStats007::GetAchievementDisplayAttribute( const char *pchName, const char *pchKey )
{
	PRINT_FUNC();
	return 0;
}

bool CSteamUserStats007::IndicateAchievementProgress( const char *pchName, uint32 nCurProgress, uint32 nMaxProgress )
{
	PRINT_FUNC();
	return false;
}

SteamAPICall_t CSteamUserStats007::RequestUserStats( CSteamID steamIDUser )
{
	PRINT_FUNC();
	return 0;
}


bool CSteamUserStats007::GetUserStat( CSteamID steamIDUser, const char *pchName, float *pData )
{
	PRINT_FUNC();
	return 0;
}

bool CSteamUserStats007::GetUserStat( CSteamID steamIDUser, const char *pchName, int32 *pData )
{
	PRINT_FUNC();
	return 0;
}


bool CSteamUserStats007::GetUserAchievement( CSteamID steamIDUser, const char *pchName, bool *pbAchieved )
{
	PRINT_FUNC();
	return 0;
}

bool CSteamUserStats007::GetUserAchievementAndUnlockTime( CSteamID steamIDUser, const char *pchName, bool *pbAchieved, RTime32 *prtTime )
{
	PRINT_FUNC();
	return 0;
}


bool CSteamUserStats007::ResetAllStats( bool bAchievementsToo )
{
	PRINT_FUNC();
	return 0;
}


SteamAPICall_t CSteamUserStats007::FindOrCreateLeaderboard( const char *pchLeaderboardName, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType )
{
	PRINT_FUNC();
	return 0;
}


SteamAPICall_t CSteamUserStats007::FindLeaderboard( const char *pchLeaderboardName )
{
	PRINT_FUNC();
	return 0;
}


const char * CSteamUserStats007::GetLeaderboardName( SteamLeaderboard_t hSteamLeaderboard )
{
	PRINT_FUNC();
	return 0;
}


int CSteamUserStats007::GetLeaderboardEntryCount( SteamLeaderboard_t hSteamLeaderboard )
{
	PRINT_FUNC();
	return 0;
}


ELeaderboardSortMethod CSteamUserStats007::GetLeaderboardSortMethod( SteamLeaderboard_t hSteamLeaderboard )
{
	PRINT_FUNC();
	return k_ELeaderboardSortMethodNone;
}


ELeaderboardDisplayType CSteamUserStats007::GetLeaderboardDisplayType( SteamLeaderboard_t hSteamLeaderboard )
{
	PRINT_FUNC();
	return k_ELeaderboardDisplayTypeNone;
}


SteamAPICall_t CSteamUserStats007::DownloadLeaderboardEntries( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardDataRequest eLeaderboardDataRequest, int nRangeStart, int nRangeEnd )
{
	PRINT_FUNC();
	return 0;
}


bool CSteamUserStats007::GetDownloadedLeaderboardEntry( SteamLeaderboardEntries_t hSteamLeaderboardEntries, int index, LeaderboardEntry001_t *pLeaderboardEntry, int32 *pDetails, int cDetailsMax )
{
	PRINT_FUNC();
	return 0;
}



SteamAPICall_t CSteamUserStats007::UploadLeaderboardScore( SteamLeaderboard_t hSteamLeaderboard,ELeaderboardUploadScoreMethod eLeaderboardUploadScoreMethod, int32 nScore, int32 *pScoreDetails, int cScoreDetailsCount )
{
	PRINT_FUNC();
	return 0;
}

SteamAPICall_t CSteamUserStats007::GetNumberOfCurrentPlayers()
{
	PRINT_FUNC();
	return 0;
}

