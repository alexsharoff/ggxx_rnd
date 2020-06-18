// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: ISteamUtils005 implementation
//
// Initial author: NTAuthority
// Started: 2010-09-10
// ==========================================================

#include "SteamUtils005.h"

#include <debug.h>

uint32 CSteamUtils005::GetSecondsSinceAppActive() {
	PRINT_FUNC();
	return 0;
}

uint32 CSteamUtils005::GetSecondsSinceComputerActive() {
	PRINT_FUNC();
	return 0;
}

EUniverse CSteamUtils005::GetConnectedUniverse() {
	PRINT_FUNC();
	return k_EUniversePublic;
}

uint32 CSteamUtils005::GetServerRealTime() {
	PRINT_FUNC();
	return 0;
}

const char* CSteamUtils005::GetIPCountry() {
	PRINT_FUNC();
	return "US";
}

bool CSteamUtils005::GetImageSize( int iImage, uint32 *pnWidth, uint32 *pnHeight ) {
	PRINT_FUNC();
	return false;
}

bool CSteamUtils005::GetImageRGBA( int iImage, uint8 *pubDest, int nDestBufferSize ) {
	PRINT_FUNC();
	return false;
}

bool CSteamUtils005::GetCSERIPPort( uint32 *unIP, uint16 *usPort ) {
	PRINT_FUNC();
	return false;
}

uint8 CSteamUtils005::GetCurrentBatteryPower() {
	PRINT_FUNC();
	return 255;
}

uint32 CSteamUtils005::GetAppID() {
	PRINT_FUNC();
	return 440;
}

void CSteamUtils005::SetOverlayNotificationPosition( ENotificationPosition eNotificationPosition ) {
	PRINT_FUNC();
}

bool CSteamUtils005::IsAPICallCompleted( SteamAPICall_t hSteamAPICall, bool *pbFailed ) {
	PRINT_FUNC();
	//return (Callbacks::_calls->ContainsKey(hSteamAPICall)) ? Callbacks::_calls[hSteamAPICall] : false;
	return false;
}

ESteamAPICallFailure CSteamUtils005::GetAPICallFailureReason( SteamAPICall_t hSteamAPICall ) {
	PRINT_FUNC();
	return k_ESteamAPICallFailureNone;
}

bool CSteamUtils005::GetAPICallResult( SteamAPICall_t hSteamAPICall, void *pCallback, int cubCallback, int iCallbackExpected, bool *pbFailed ) {
	PRINT_FUNC();
	return false;
}

void CSteamUtils005::RunFrame() {
	PRINT_FUNC();

}

uint32 CSteamUtils005::GetIPCCallCount() {
	PRINT_FUNC();
	return 0;
}

void CSteamUtils005::SetWarningMessageHook( SteamAPIWarningMessageHook_t pFunction ) {
	PRINT_FUNC();
}

bool CSteamUtils005::IsOverlayEnabled() { 
	PRINT_FUNC();return false; }

bool CSteamUtils005::BOverlayNeedsPresent() { 
	PRINT_FUNC();return false; }

SteamAPICall_t CSteamUtils005::CheckFileSignature(const char *)
{
	PRINT_FUNC();
	return SteamAPICall_t();
}

bool CSteamUtils005::ShowGamepadTextInput(EGamepadTextInputMode,EGamepadTextInputLineMode,const char *,uint32)
{
	PRINT_FUNC();
	return true;
}

uint32 CSteamUtils005::GetEnteredGamepadTextLength()
{
	PRINT_FUNC();
	return 0;
}

bool CSteamUtils005::GetEnteredGamepadTextInput(char *,uint32)
{
	PRINT_FUNC();
	return 0;
}
