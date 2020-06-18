cmake_minimum_required(VERSION 3.12)

project(steam_api_stub)

# TODO: fix warnings and remove this
get_directory_property(opts COMPILE_OPTIONS)
if(opts)
	list(REMOVE_ITEM opts /W4)
	set_directory_properties(PROPERTIES COMPILE_OPTIONS "${opts}")
endif()

add_library(steam_api SHARED
	src/SteamAPI.cpp
	src/SteamFriends005.cpp
	src/SteamGameServer009.cpp
	src/SteamMasterServerUpdater001.cpp
	src/SteamMatchmaking007.cpp
	src/SteamNetworking004.cpp
	src/SteamRemoteStorage002.cpp
	src/SteamUser012.cpp
	src/SteamUserStats007.cpp
	src/SteamUtils005.cpp
)
target_include_directories(steam_api PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/include
)
#target_compile_definitions(steam_api
#	STEAM_API_DBGPRINT
#)
