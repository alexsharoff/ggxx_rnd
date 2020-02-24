cmake_minimum_required(VERSION 3.12)

project(steam_api_nop)

add_compile_options($<IF:$<CONFIG:Debug>,/MTd,/MT>)

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