cmake_minimum_required(VERSION 3.15.0)
cmake_policy(SET CMP0091 NEW)

set(PROJECT_NAME ghlfolder)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF CACHE BOOL "" FORCE)
project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/fetch_bar.cpp
        src/thread_pool.cpp
        src/term.cpp
        src/matcher.cpp
        src/repo.cpp
        src/bar_pool.cpp
        src/progress_bar.cpp
        src/zip_file.cpp
        src/utils.cpp
)

if (NOT WIN32)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
        include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/argparse/include
)

find_package(libzip REQUIRED CONFIG)
find_package(cpr REQUIRED CONFIG)
find_package(ctre REQUIRED CONFIG)
find_package(nlohmann_json REQUIRED CONFIG)

target_link_libraries(${PROJECT_NAME}
        libzip::zip
        cpr::cpr
        ctre::ctre
        nlohmann_json::nlohmann_json)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)