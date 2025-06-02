cmake_minimum_required(VERSION 3.14.0)
set(PROJECT_NAME ghlfolder)
set(CMAKE_CXX_STANDARD 20)
project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/fetch_bar.cpp
    src/thread_pool.cpp
    src/term.cpp
    src/matcher.cpp
    src/repo.cpp
)
target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Find and include CURL headers
find_package(CURL REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARIES})

# Find and include CPR library
find_package(cpr REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)

# Find and include ctre
find_package(ctre CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ctre::ctre)

# Find and include nlohmann/json
find_package(nlohmann_json 3.11.3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# Include p-ranav/argparse headers
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/argparse/include)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)