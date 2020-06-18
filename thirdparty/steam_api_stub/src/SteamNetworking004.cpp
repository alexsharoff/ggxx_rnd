// ==========================================================
// IW4M project
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: ISteamNetworking003 implementation
//
// Initial author: NTAuthority
// Started: 2010-09-19
// ==========================================================

#include "SteamNetworking004.h"

#include <debug.h>

bool CSteamNetworking004::SendP2PPacket( CSteamID steamIDRemote, const void *pubData, uint32 cubData, EP2PSend eP2PSendType, int iVirtualPort ) {
	PRINT_FUNC();
	return false;
}

bool CSteamNetworking004::IsP2PPacketAvailable( uint32 *pcubMsgSize, int iVirtualPort ) {
	PRINT_FUNC();
	return false;
}

bool CSteamNetworking004::ReadP2PPacket( void *pubDest, uint32 cubDest, uint32 *pcubMsgSize, CSteamID *psteamIDRemote, int iVirtualPort ) {
	PRINT_FUNC();
	return false;
}

bool CSteamNetworking004::AcceptP2PSessionWithUser( CSteamID steamIDRemote ) {
	PRINT_FUNC();
	return false;
}

bool CSteamNetworking004::CloseP2PSessionWithUser( CSteamID steamIDRemote ) {
	PRINT_FUNC();
	return false;
}

bool CSteamNetworking004::GetP2PSessionState( CSteamID steamIDRemote, P2PSessionState_t *pConnectionState ) {
	PRINT_FUNC();
	return false;
}

SNetListenSocket_t CSteamNetworking004::CreateListenSocket( int nVirtualP2PPort, uint32 nIP, uint16 nPort, bool bAllowUseOfPacketRelay ) {
	PRINT_FUNC();
	return NULL;
}

SNetSocket_t CSteamNetworking004::CreateP2PConnectionSocket( CSteamID steamIDTarget, int nVirtualPort, int nTimeoutSec, bool bAllowUseOfPacketRelay ) {
	PRINT_FUNC();
	return NULL;
}

SNetSocket_t CSteamNetworking004::CreateConnectionSocket( uint32 nIP, uint16 nPort, int nTimeoutSec ) {
	PRINT_FUNC();
	return NULL;
}

bool CSteamNetworking004::DestroySocket( SNetSocket_t hSocket, bool bNotifyRemoteEnd ) {
	PRINT_FUNC();
	return false;
}

bool CSteamNetworking004::DestroyListenSocket( SNetListenSocket_t hSocket, bool bNotifyRemoteEnd ) {
	PRINT_FUNC();
	return false;
}

bool CSteamNetworking004::SendDataOnSocket( SNetSocket_t hSocket, void *pubData, uint32 cubData, bool bReliable ) {
	PRINT_FUNC();
	return false;
}

bool CSteamNetworking004::IsDataAvailableOnSocket( SNetSocket_t hSocket, uint32 *pcubMsgSize ) {
	PRINT_FUNC();
	return false;
}

bool CSteamNetworking004::RetrieveDataFromSocket( SNetSocket_t hSocket, void *pubDest, uint32 cubDest, uint32 *pcubMsgSize ) {
	PRINT_FUNC();
	return false;
}

bool CSteamNetworking004::IsDataAvailable( SNetListenSocket_t hListenSocket, uint32 *pcubMsgSize, SNetSocket_t *phSocket ) {
	PRINT_FUNC();
	return false;
}

bool CSteamNetworking004::RetrieveData( SNetListenSocket_t hListenSocket, void *pubDest, uint32 cubDest, uint32 *pcubMsgSize, SNetSocket_t *phSocket ) {
	PRINT_FUNC();
	return false;
}

bool CSteamNetworking004::GetSocketInfo( SNetSocket_t hSocket, CSteamID *pSteamIDRemote, int *peSocketStatus, uint32 *punIPRemote, uint16 *punPortRemote ) {
	PRINT_FUNC();
	return false;
}

bool CSteamNetworking004::GetListenSocketInfo( SNetListenSocket_t hListenSocket, uint32 *pnIP, uint16 *pnPort ) {
	PRINT_FUNC();
	return false;
}

ESNetSocketConnectionType CSteamNetworking004::GetSocketConnectionType( SNetSocket_t hSocket ) {
	PRINT_FUNC();
	return k_ESNetSocketConnectionTypeNotConnected;
}

int CSteamNetworking004::GetMaxPacketSize( SNetSocket_t hSocket ) {
	PRINT_FUNC();
	return 0;
}