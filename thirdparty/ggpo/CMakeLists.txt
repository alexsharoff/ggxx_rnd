include(CMakeSources.cmake)

project(GGPO VERSION 1.0.0)

get_directory_property(defs COMPILE_DEFINITIONS)
if(defs)
	# WIN32_LEAN_AND_MEAN is not supported by GGPO
	list(REMOVE_ITEM defs WIN32_LEAN_AND_MEAN)
	set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "${defs}")
endif()

add_library(GGPO
	${GGPO_LIB_SRC}
)

target_include_directories(GGPO PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/ggpo>
)

if(WIN32)
	target_compile_options(GGPO PRIVATE "/W4" "/WX")
	# Link to Multimedia API and Winsocks during a shared build.
	target_link_libraries(GGPO PUBLIC winmm.lib ws2_32.lib)
	if(BUILD_SHARED_LIBS)
		add_definitions(-DGGPO_SHARED_LIB)
		add_definitions(-DGGPO_SDK_EXPORT)
	endif()
endif()

set_target_properties(GGPO PROPERTIES VERSION ${PROJECT_VERSION})

add_subdirectory(test)
