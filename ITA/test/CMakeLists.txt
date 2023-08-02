cmake_minimum_required(VERSION 3.10)

project(test)

add_executable(test demo.c)

target_link_libraries(test sort)


