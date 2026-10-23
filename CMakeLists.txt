cmake_minimum_required(VERSION 3.16)
project(DoomLauncher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(doom-launcher
    src/main.cpp
    src/launcher.cpp
    src/mod_scanner.cpp
    src/main_window.cpp
)

target_link_libraries(doom-launcher PRIVATE Qt6::Widgets)

if(WIN32)
    add_custom_command(TARGET doom-launcher POST_BUILD
        COMMAND Qt6::windeployqt
            --no-translations
            --compiler-runtime
            "$<TARGET_FILE:doom-launcher>"
        COMMENT "Deploying Qt runtime dependencies"
        VERBATIM
    )
endif()
