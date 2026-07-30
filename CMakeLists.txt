cmake_minimum_required(VERSION 3.27)

project(connect_four LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(connect_four
        src/main.cpp
        src/GameBoard.cpp
        src/RandomPlayer.cpp
        src/HumanPlayer.cpp
        src/BotPlayer.cpp
)

target_include_directories(connect_four PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)