add_library(steamless_lib STATIC
    src/PEFile.cpp
    src/SteamStub32_Variant3.cpp
    src/Utils.cpp
    src/aes/aes_modes.c
    src/aes/aes_ni.c
    src/aes/aescrypt.c
    src/aes/aeskey.c
    src/aes/aestab.c
)
target_include_directories(steamless_lib PUBLIC include)

add_executable(steamless src/main.cpp)
target_link_libraries(steamless steamless_lib)
