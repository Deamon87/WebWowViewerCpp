cmake_minimum_required(VERSION 3.20)
project(NotificationTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)
enable_testing()
set(UI_DIR "${CMAKE_CURRENT_LIST_DIR}/../../src/ui")
add_executable(notification_tests NotificationTests.cpp
    ${UI_DIR}/imguiLib/imgui.cpp
    ${UI_DIR}/imguiLib/imgui_draw.cpp
    ${UI_DIR}/imguiLib/imgui_tables.cpp
    ${UI_DIR}/imguiLib/imgui_widgets.cpp
    ${UI_DIR}/imguiLib/stateSaver/stateSaver.cpp)
target_include_directories(notification_tests PRIVATE ${UI_DIR}/imguiLib
    ${CMAKE_CURRENT_LIST_DIR}/../../3rdparty/imgui-notify/example/src)
target_link_libraries(notification_tests PRIVATE Threads::Threads)
add_test(NAME notification_tests COMMAND notification_tests)
