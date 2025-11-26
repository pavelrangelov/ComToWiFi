cmake_minimum_required(VERSION 3.19)
project(ComToWifi LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets SerialPort Network)

qt_standard_project_setup()

qt_add_resources(IMAGE_RESOURCES main.qrc)

qt_add_executable(ComToWifi
    WIN32 MACOSX_BUNDLE
    main.cpp
    maindialog.cpp
    maindialog.h
    maindialog.ui
    settings.h
    mythread.h
    mythread.cpp
    ${IMAGE_RESOURCES}
)

target_link_libraries(ComToWifi
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::SerialPort
        Qt::Network
)

include(GNUInstallDirs)

install(TARGETS ComToWifi
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ComToWifi
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
