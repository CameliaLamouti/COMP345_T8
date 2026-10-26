# CMakeLists.txt - lets CLion build and run the five Assignment 1 drivers.
# This is an IDE/build file: do NOT put it in the submission zip.

cmake_minimum_required(VERSION 3.16)
project(Warzone CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# One program per driver, each built from only the .cpp files it needs.
add_executable(MapDriver        Map.cpp MapDriver.cpp Player.cpp Cards.cpp Orders.cpp)
add_executable(PlayerDriver     Player.cpp PlayerDriver.cpp Cards.cpp Orders.cpp Map.cpp)
add_executable(OrdersDriver     Orders.cpp OrdersDriver.cpp Map.cpp Player.cpp Cards.cpp)
add_executable(CardsDriver      Cards.cpp CardsDriver.cpp Player.cpp Orders.cpp Map.cpp)
add_executable(GameEngineDriver GameEngine.cpp GameEngineDriver.cpp)

# MapDriver opens files like maps/europe.map relative to the folder it runs in.
# CLion runs programs from its build folder, so copy the maps folder there
# every time MapDriver is built.
if(EXISTS ${CMAKE_SOURCE_DIR}/maps)
    add_custom_command(TARGET MapDriver POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/maps $<TARGET_FILE_DIR:MapDriver>/maps)
endif()
