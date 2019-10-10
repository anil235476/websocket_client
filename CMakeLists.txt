cmake_minimum_required(VERSION 3.7)

set(WEBRTC_INC "D:/webrtcBuild/Sept/webrtc_checkout/src")
set(BOOST_LIB "D:/boost_1_69_0/boost_1_69_0")
set(BORING_SSL_PATH ${WEBRTC_INC}/third_party/boringssl/src/include)

if(SECURE_SIGNALING)
add_definitions(-DSECURE_SIGNALING)
endif(SECURE_SIGNALING)

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
	$<BUILD_INTERFACE:${BORING_SSL_PATH}>
	)


target_link_directories( signaller PUBLIC
	#	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/openssl-1.1.0f-vs2017/lib64>
	#$<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/webrtc_open_source/debug>
)
