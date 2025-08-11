cmake_minimum_required(VERSION 3.15)
project(tmuxscript VERSION 0.1)
add_executable(tmuxscript main.cpp)

add_subdirectory(src)

target_link_libraries(tmuxscript OSTmuxScripting)

# Testing
add_executable(testexec test.cc)
target_link_libraries(testexec OSTmuxScripting)

enable_testing()

add_test(
  NAME BasicFuncTest
  COMMAND $<TARGET_FILE:testexec>
  )
