add_compile_definitions(WIN32_LEAN_AND_MEAN)
add_executable(test_ggpo ggpo.cpp)
target_link_libraries(test_ggpo GGPO Winmm Ws2_32)

add_test(NAME test_ggpo COMMAND test_ggpo)
