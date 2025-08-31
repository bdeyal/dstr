# CMake simple example

cmake_minimum_required(VERSION 3.15...4.0)

#set(CMAKE_C_COMPILER clang)
#set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -W -Wextra")

#add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
#add_link_options(-fsanitize=address)

project(
  Dstr
  VERSION 1.0
  LANGUAGES C)

add_library(dstr SHARED dstr.c dstr.h dstr_impl.h dstr_inlines.h)

add_executable(dstrtest dstr_test.c)

target_link_libraries(dstrtest PRIVATE dstr)

enable_testing()

add_test(Test dstrtest COMMAND ./dstrtest)
