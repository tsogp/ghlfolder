cmake_minimum_required(VERSION 3.14.0)
set(PROJECT_NAME ghlfolder)
project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARIES})
