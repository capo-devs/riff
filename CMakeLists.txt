cmake_minimum_required(VERSION 3.23)

project(riff VERSION "0.1.2")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "-d")

option(RIFF_MA_DEBUG_OUTPUT "Enable miniaudio debug output" ${PROJECT_IS_TOP_LEVEL})
option(RIFF_BUILD_BIN2CPP "Build bin2cpp tool" ${PROJECT_IS_TOP_LEVEL})

add_subdirectory(ext)

message(STATUS "[riff]")

configure_file(src/build_version.hpp.in "${CMAKE_CURRENT_BINARY_DIR}/include/build_version.hpp")

add_executable(${PROJECT_NAME} WIN32)
klib_set_mainCRTStartup(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
  gvdi::gvdi
  capo::capo
  klib::klib
  icons::kenny
)

target_include_directories(${PROJECT_NAME} PRIVATE
  src
  "${CMAKE_CURRENT_BINARY_DIR}/include"
)

file(GLOB_RECURSE sources LIST_DIRECTORIES false "src/*.[hc]pp")
target_sources(${PROJECT_NAME} PRIVATE
  ${sources}
)
