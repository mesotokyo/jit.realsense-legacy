set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(CMAKE_BUILD_TYPE Release)

cmake_minimum_required(VERSION 3.5)
project(jit.realsense)
set(LibUSB_FIND_STATIC ON)
set(BUILD_EXAMPLES OFF)
set(BUILD_UNIT_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)
if(APPLE)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pipe -Ofast -march=corei7-avx")
else()
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O4 /MT")
endif()

if(CMAKE_GENERATOR MATCHES ".*Win64.*")
    set(WIN64 1)
endif()


add_subdirectory(librealsense)
set(CMAKE_POSITION_INDEPENDENT_CODE True)
set(BUILD_SHARED_LIBS ON)

set(C74SUPPORT ${CMAKE_CURRENT_SOURCE_DIR}/max-sdk/source/max-sdk-base/c74support)
set(MAX_ROOT ${C74SUPPORT}/max-includes)
set(MSP_ROOT ${C74SUPPORT}/msp-includes)
set(JIT_ROOT ${C74SUPPORT}/jit-includes)

add_library(MaxAPI INTERFACE IMPORTED)
add_library(MaxAudio INTERFACE IMPORTED)
add_library(Jitter INTERFACE IMPORTED)

# Includes
set_target_properties(MaxAPI PROPERTIES	INTERFACE_INCLUDE_DIRECTORIES ${MAX_ROOT})
set_target_properties(MaxAudio PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${MSP_ROOT})
set_target_properties(Jitter PROPERTIES	INTERFACE_INCLUDE_DIRECTORIES ${JIT_ROOT})
if(WIN32)
	if(WIN64)
		set_target_properties(MaxAPI PROPERTIES INTERFACE_LINK_LIBRARIES ${MAX_ROOT}/x64/MaxAPI.lib)
		set_target_properties(MaxAudio PROPERTIES INTERFACE_LINK_LIBRARIES ${MSP_ROOT}/x64/MaxAudio.lib)
		set_target_properties(Jitter PROPERTIES INTERFACE_LINK_LIBRARIES ${JIT_ROOT}/x64/jitlib.lib)
	else()
		set_target_properties(MaxAPI PROPERTIES INTERFACE_LINK_LIBRARIES ${MAX_ROOT}/MaxAPI.lib)
		set_target_properties(MaxAudio PROPERTIES INTERFACE_LINK_LIBRARIES ${MSP_ROOT}/MaxAudio.lib)
		set_target_properties(Jitter PROPERTIES INTERFACE_LINK_LIBRARIES ${JIT_ROOT}/jitlib.lib)
	endif()

	set_target_properties(MaxAPI PROPERTIES 
		INTERFACE_COMPILE_DEFINITIONS 
			"WIN_VERSION;_CRT_SECURE_NO_WARNINGS;VER_TARGETNAME=\"jit.realsense\"")

elseif(APPLE)
	set_target_properties(MaxAPI PROPERTIES INTERFACE_LINK_LIBRARIES ${MAX_ROOT}/MaxAPI.framework)
	set_target_properties(MaxAudio PROPERTIES INTERFACE_LINK_LIBRARIES ${MSP_ROOT}/MaxAudioAPI.framework)
	set_target_properties(Jitter PROPERTIES INTERFACE_LINK_LIBRARIES ${JIT_ROOT}/JitterAPI.framework)

	# #include<Files.h> is incompatbile with CMake framework handling
	FIND_PATH(FILE_H_DIR Files.h
            HINTS
            /System/Library/Frameworks/
            /System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers/
            /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers
            /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers
            /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.13.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers
            /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers
            /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers
            /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.10.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers
            /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers)

	# Rpath handling to be able to copy the built external in max folder
	set(CMAKE_INSTALL_NAME_DIR @rpath)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
	set(CMAKE_INSTALL_RPATH "@loader_path/")

        # libusb settings
        find_package(PkgConfig REQUIRED)
        find_package (Threads REQUIRED)
        pkg_search_module(LIBUSB1 REQUIRED libusb-1.0)
        if(LIBUSB1_FOUND)
            include_directories(SYSTEM ${LIBUSB1_INCLUDE_DIRS})
            link_directories(${LIBUSB1_LIBRARY_DIRS})
        else()
            message( FATAL_ERROR "Failed to find libusb-1.0" )
        endif(LIBUSB1_FOUND)
endif()

add_library(jit.realsense MODULE jit.realsense.cpp max.jit.realsense.cpp rs_options.cpp)
target_link_libraries(jit.realsense PRIVATE MaxAPI Jitter realsense)
set_target_properties(jit.realsense PROPERTIES CXX_STANDARD 14)

if(APPLE)
	find_library(coreFoundation CoreFoundation)
	find_library(iokit IOKit)
	target_link_libraries(jit.realsense PUBLIC ${LIBUSB1_LIBRARIES} ${coreFoundation} ${iokit} objc)
	target_compile_options(jit.realsense PUBLIC
		-pipe
		-Weverything
		-Wno-old-style-cast
		-Wno-c++98-compat
		-Wno-c++98-compat-pedantic
		-Wno-sign-conversion
		-Wno-vla-extension
		-Wno-exit-time-destructors
		-Wno-cast-align
		-Wno-weak-vtables
		-Wno-vla
		-Wno-four-char-constants
		-Wno-missing-prototypes
		-Wno-gnu-zero-variadic-macro-arguments
		-Wno-float-equal)
endif()

if(WIN32)
	target_compile_definitions(jit.realsense PRIVATE MAXAPI_USE_MSCRT)
	if(WIN64)
		set_target_properties(jit.realsense PROPERTIES 
			PREFIX "" 
			SUFFIX .mxe64)
	else()
		set_target_properties(jit.realsense PROPERTIES 
			PREFIX "" 
			SUFFIX .mxe)
	endif()
else()
	set_target_properties(jit.realsense PROPERTIES 
		PREFIX "" 
		SUFFIX ""
		BUNDLE TRUE
		BUNDLE_EXTENSION "mxo")

	target_include_directories(jit.realsense PRIVATE ${FILE_H_DIR})
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/jit.realsense.mxo/Contents/PkgInfo "iLaX????")
endif()
