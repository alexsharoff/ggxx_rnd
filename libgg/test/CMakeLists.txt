add_executable(test_mini_reflection src/mini_reflection.cpp)
target_link_libraries(test_mini_reflection libgg_utils)

add_executable(test_fiber_mgmt src/fiber_mgmt.cpp)
target_link_libraries(test_fiber_mgmt libgg_utils)

add_executable(test_memory_dump src/memory_dump.cpp)
target_link_libraries(test_memory_dump libgg_utils)

add_test(NAME mini_reflection COMMAND test_mini_reflection)
add_test(NAME fiber_mgmt COMMAND test_fiber_mgmt)
add_test(NAME memory_dump COMMAND test_memory_dump)
