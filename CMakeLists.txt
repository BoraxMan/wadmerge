project(wadmerge)

add_executable(wadmerge wadlumpptr.cpp wad.cpp main.cpp)

install(TARGETS wadmerge RUNTIME DESTINATION bin)
