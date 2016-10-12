#
#	AddonInstaller library
#

include_directories(
	${CMAKE_SOURCE_DIR}/src/common
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/external/HL_SDK/common
	${CMAKE_SOURCE_DIR}/external/HL_SDK/engine
	${CMAKE_SOURCE_DIR}/external/HL_SDK/public
	${CMAKE_SOURCE_DIR}/external/SDL2/include
	${CMAKE_SOURCE_DIR}/external/SOURCE_SDK/src/public/steam
)

add_sources(
	AddonInstaller.h
	AddonInstaller.cpp
	CAddonInstaller.h
	CAddonInstaller.cpp
	InstallerInterface.h
	InstallerInterface.cpp
)

add_subdirectory( ${CMAKE_SOURCE_DIR}/external/HL_SDK/public HL_SDK/public )
add_subdirectory( ${CMAKE_SOURCE_DIR}/external/SOURCE_SDK/src/public/steam public/steam )
add_subdirectory( ${CMAKE_SOURCE_DIR}/src/steam steam )
add_subdirectory( ${CMAKE_SOURCE_DIR}/src/common common )

preprocess_sources()

link_directories( "${STEAMCOMMON}/Half-Life" )

add_library( AddonInstaller SHARED ${PREP_SRCS} )

target_compile_definitions( AddonInstaller PRIVATE
	${SHARED_DEFS}
	STEAM_API_NODLL
	VERSION_SAFE_STEAM_API_INTERFACES
)

if( UNIX )
	if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
		set( SDL2_NAME "libSDL2-2.0.0.dylib" )
	else()
		set( SDL2_NAME "libSDL2-2.0.so.0" )
	endif()
else()
	set( SDL2_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}SDL2${CMAKE_STATIC_LIBRARY_SUFFIX}" )
endif()

find_library( SDL2 ${SDL2_NAME} PATHS ${CMAKE_SOURCE_DIR}/external/SDL2/lib/ "${STEAMCOMMON}/Half-Life/" )

#Link with addoninstaller dependencies
target_link_libraries( AddonInstaller 
	${SDL2}
	Tier1
	${UNIX_FS_LIB}
)

#CMake places libraries in /Debug or /Release on Windows, so explicitly set the paths for both.
#On Linux, it uses LIBRARY_OUTPUT_DIRECTORY
set_target_properties( AddonInstaller PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${GAME_TOOL_PATH}/AddonInstaller/bin"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${GAME_TOOL_PATH}/AddonInstaller/bin"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${GAME_TOOL_PATH}/AddonInstaller/bin"
)

if( WIN32 AND MSVC )
	#Set Windows subsystem
	set( ADDONINSTALLER_LINK_FLAGS "/SUBSYSTEM:WINDOWS /DELAYLOAD:Tier1.dll" )
else()
	set( ADDONINSTALLER_LINK_FLAGS "" )
endif()

set_target_properties( AddonInstaller 
	PROPERTIES COMPILE_FLAGS "${LINUX_32BIT_FLAG}" 
	LINK_FLAGS "${ADDONINSTALLER_LINK_FLAGS} ${LINUX_32BIT_FLAG}" )

#No lib prefix
set_target_properties( AddonInstaller PROPERTIES PREFIX "" )

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}" )

clear_sources()
