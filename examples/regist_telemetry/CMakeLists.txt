set(PROJECT_NAME rotate_vehicle)
cmake_minimum_required(VERSION 3.10.2)
project(regist_telemetry)


if(NOT MSVC)
    add_definitions("-Wall -Wextra")
else()
    add_definitions("-WX -W2")
endif()

find_package(MAVSDK REQUIRED)


add_executable(regist_telemetry
regist_telemetry.cpp
)
target_link_libraries(regist_telemetry
    MAVSDK::mavsdk_telemetry
    MAVSDK::mavsdk_action
    MAVSDK::mavsdk
    MAVSDK::mavsdk_offboard
)