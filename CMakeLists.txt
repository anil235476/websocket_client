cmake_minimum_required(VERSION 3.7)

set(BOOST_LIB "D:/boost_1_69_0/boost_1_69_0")

add_library(signaller
	signaller.h
	signaller.cpp
	websocket_signaller.h
	websocket_signaller.cpp
	websocket_client_async.hpp
	)

	# Add some compile flags to our source files.
if(MSVC)
	#set_source_files_properties(${SOURCE_FILES}
	#	PROPERTIES COMPILE_FLAGS "/W3 /WX")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

endif()

target_include_directories(signaller PUBLIC 
	$<BUILD_INTERFACE:${BOOST_LIB}>
	#$<BUILD_INTERFACE:${ABSL_PATH}>
	)
