cmake_minimum_required(VERSION 3.7)

#set(WEBRTC_INC "D:/webrtcBuild/Sept/webrtc_checkout/src")
#set(BOOST_LIB "F:/project/boost_1_69_0/")
set(BORING_SSL_PATH ${LIBWEBRTC_INCLUDE_PATH}/third_party/boringssl/src/include)

if(SECURE_SIGNALING)
add_definitions(-DSECURE_SIGNALING)
endif(SECURE_SIGNALING)

add_definitions(/ZI)

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

target_include_directories(signaller
  PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
	$<BUILD_INTERFACE:${BOOST_LIB}>
	$<BUILD_INTERFACE:${BORING_SSL_PATH}>
	)

target_link_directories( signaller PUBLIC

	$<BUILD_INTERFACE:${BOOST_LIB}/stage/lib>

	)
target_link_libraries(signaller PUBLIC
	${LIBWEBRTC_BINARY_PATH}/webrtc${CMAKE_STATIC_LIBRARY_SUFFIX}
)

