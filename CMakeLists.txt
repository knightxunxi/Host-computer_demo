cmake_minimum_required(VERSION 3.24)

project(UpkunHmi
    VERSION 0.1.0
    DESCRIPTION "C++ Qt industrial HMI learning project"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Sql Charts)

qt_standard_project_setup()

qt_add_executable(UpkunHmi WIN32
    src/app/main.cpp
    src/app/MainWindow.h
    src/app/MainWindow.cpp
    src/ui/pages/MonitorPage.h
    src/ui/pages/MonitorPage.cpp
    src/ui/pages/SimulatorPage.h
    src/ui/pages/SimulatorPage.cpp
    src/domain/DeviceTypes.h
    src/domain/AlarmTypes.h
    src/domain/Recipe.h
    src/domain/User.h
    src/services/LineControlService.h
    src/services/LineControlService.cpp
    src/services/UserSessionService.h
    src/services/UserSessionService.cpp
    src/device/IDeviceClient.h
    src/device/ModbusPointMap.h
    src/storage/DatabaseManager.h
    src/storage/DatabaseManager.cpp
    src/infrastructure/AppConfig.h
    src/infrastructure/AppConfig.cpp
    src/infrastructure/Logger.h
    src/infrastructure/Result.h
)

target_include_directories(UpkunHmi PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(UpkunHmi PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::Charts
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(UpkunHmi PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -finput-charset=UTF-8
        -fexec-charset=UTF-8
    )
endif()

set_target_properties(UpkunHmi PROPERTIES
    OUTPUT_NAME "upkun-hmi"
)
