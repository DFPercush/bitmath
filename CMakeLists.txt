cmake_minimum_required(VERSION 3.5)

#find_package(readline)

project(bitmath LANGUAGES C)

add_executable(bitmath main.c)


#if (${CMAKE_HOST_UNIX})
#	target_compile_definitions(bitmath PUBLIC USE_READLINE)
#endif()
target_link_libraries(bitmath PUBLIC readline)

install(TARGETS bitmath
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
