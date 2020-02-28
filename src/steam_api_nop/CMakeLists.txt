cmake_minimum_required(VERSION 3.12)

project(steam_api_nop)

add_library(steam_api SHARED
	SteamAPI.cpp
	SteamFriends005.cpp
	SteamGameServer009.cpp
	SteamMasterServerUpdater001.cpp
	SteamMatchmaking007.cpp
	SteamNetworking004.cpp
	SteamRemoteStorage002.cpp
	SteamUser012.cpp
	SteamUserStats007.cpp
	SteamUtils005.cpp
)
target_include_directories(steam_api PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/include
)
#target_compile_definitions(steam_api
#	STEAM_API_DBGPRINT
#)
