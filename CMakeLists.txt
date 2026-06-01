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

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql Charts)

qt_standard_project_setup()

qt_add_executable(UpkunHmi WIN32
    src/app/main.cpp
    src/app/MainWindow.h
    src/app/MainWindow.cpp
    src/ui/pages/MonitorPage.h
    src/ui/pages/MonitorPage.cpp
    src/ui/pages/BatchPage.h
    src/ui/pages/BatchPage.cpp
    src/ui/pages/AlarmPage.h
    src/ui/pages/AlarmPage.cpp
    src/ui/pages/RecipePage.h
    src/ui/pages/RecipePage.cpp
    src/ui/pages/TrendPage.h
    src/ui/pages/TrendPage.cpp
    src/ui/pages/SimulatorPage.h
    src/ui/pages/SimulatorPage.cpp
    src/ui/dialogs/LoginDialog.h
    src/ui/dialogs/LoginDialog.cpp
    src/domain/DeviceTypes.h
    src/domain/AlarmTypes.h
    src/domain/Batch.h
    src/domain/Recipe.h
    src/domain/User.h
    src/services/LineControlService.h
    src/services/LineControlService.cpp
    src/services/AlarmService.h
    src/services/AlarmService.cpp
    src/services/UserSessionService.h
    src/services/UserSessionService.cpp
    src/device/IDeviceClient.h
    src/device/ModbusTcpClient.h
    src/device/ModbusTcpClient.cpp
    src/device/ModbusPointMap.h
    src/storage/DatabaseManager.h
    src/storage/DatabaseManager.cpp
    src/storage/AlarmRepository.h
    src/storage/AlarmRepository.cpp
    src/storage/BatchRepository.h
    src/storage/BatchRepository.cpp
    src/storage/OperationLogRepository.h
    src/storage/OperationLogRepository.cpp
    src/storage/UserRepository.h
    src/storage/UserRepository.cpp
    src/storage/RecipeRepository.h
    src/storage/RecipeRepository.cpp
    src/storage/TrendRepository.h
    src/storage/TrendRepository.cpp
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

qt_add_executable(UpkunSimulator WIN32
    src/simulator/main.cpp
    src/simulator/LineSimulator.h
    src/simulator/LineSimulator.cpp
    src/simulator/SimulatedModbusServer.h
    src/simulator/SimulatedModbusServer.cpp
    src/device/ModbusPointMap.h
    src/domain/DeviceTypes.h
)

target_include_directories(UpkunSimulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(UpkunSimulator PRIVATE
    Qt6::Core
    Qt6::Network
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    foreach(target_name UpkunHmi UpkunSimulator)
        target_compile_options(${target_name} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -finput-charset=UTF-8
            -fexec-charset=UTF-8
        )
    endforeach()
endif()

set_target_properties(UpkunHmi PROPERTIES
    OUTPUT_NAME "upkun-hmi"
)

set_target_properties(UpkunSimulator PROPERTIES
    OUTPUT_NAME "upkun-simulator"
)
