cmake_minimum_required(VERSION 3.14.0)
set(PROJECT_NAME ghlfolder)
project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

## Find and include CURL headers
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

# Include p-ranav/argparse headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/argparse/include)

add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

target_compile_options(${PROJECT_NAME} PRIVATE -fstandalone-debug -fsanitize=address -fno-omit-frame-pointer)
target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)