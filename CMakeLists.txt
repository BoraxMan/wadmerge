project(wadmerge)

add_executable(wadmerge wad.cpp main.cpp)

install(TARGETS wadmerge RUNTIME DESTINATION bin)
