project(wadmerge)

add_executable(wadmerge wadlumpptr.cpp wad.cpp main.cpp)

install(TARGETS wadmerge RUNTIME DESTINATION bin)
INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/man DESTINATION share/man/man1)

cmake_minimum_required (VERSION 2.6)

# The version number.
set (wadmerge_VERSION_MAJOR 0)
set (wadmerge_VERSION_MINOR 8)
 
