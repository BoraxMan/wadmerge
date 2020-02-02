project(wadmerge)

add_executable(wadmerge wad.cpp main.cpp)
set (PACKAGE wadmerge)
set (VERSION 1.0.2)

install(TARGETS wadmerge RUNTIME DESTINATION bin)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/man1/ DESTINATION share/man/man1)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/doc/ DESTINATION share/doc/${PACKAGE})

cmake_minimum_required (VERSION 2.6)

# The version number.
configure_file(version.h.in ${CMAKE_SOURCE_DIR}/version.h)
configure_file(wadmerge.spec.in ${CMAKE_SOURCE_DIR}/wadmerge.spec)

set(MINGW_PREFIX "i686-w64-mingw32-")
 
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "wadmerge")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "Dennis Katsonis")
set(CPACK_PACKAGE_VENDOR "Dekay Software")
set(CPACK_STRIP_FILES "bin/wadmerge")
#set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/share/man")
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/share/man/man1" "/usr/share/man")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
#set(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_SOURCE_DIR}/pak.spec")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack) 
 
 
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
