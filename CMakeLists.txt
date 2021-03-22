cmake_minimum_required(VERSION 3.9)
project(Green)
include(FetchContent)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.7.3)
  
FetchContent_Declare(libgit2
  GIT_REPOSITORY https://github.com/libgit2/libgit2.git
  GIT_TAG v1.1.0)
  

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  set(BUILD_SHARED_LIBS OFF)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
  set(BUILD_SHARED_LIBS ON)
endif()

FetchContent_GetProperties(libgit2)
if (NOT libgit2_POPULATED)
  FetchContent_Populate(libgit2)
  set(BUILD_SHARED_LIBS OFF)
  add_subdirectory(${libgit2_SOURCE_DIR} ${libgit2_BINARY_DIR} EXCLUDE_FROM_ALL)
  set(BUILD_SHARED_LIBS ON)
endif()

include_directories(${libgit2_SOURCE_DIR}/include)

add_executable(Green main.cpp FSW.cpp UnixSocketClient.cpp UnixSocketClient.h git2pp.h)


target_link_libraries(
        Green PRIVATE nlohmann_json::nlohmann_json git2
)
