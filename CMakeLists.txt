project(wadmerge)

add_executable(wadmerge wad.cpp main.cpp)

install(TARGETS wadmerge RUNTIME DESTINATION bin)
INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/man1 DESTINATION share/man)

cmake_minimum_required (VERSION 2.6)

# The version number.
set (wadmerge_VERSION 1.0.0)
configure_file(version.h.in ${CMAKE_SOURCE_DIR}/version.h)
configure_file(wadmerge.spec.in ${CMAKE_SOURCE_DIR}/wadmerge.spec)

set(MINGW_PREFIX "i686-w64-mingw32-")
 
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()