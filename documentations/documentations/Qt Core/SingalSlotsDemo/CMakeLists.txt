cmake_minimum_required(VERSION 3.19)
project(ObjectDemo LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_executable(ObjectDemo
    main.cpp
)

target_link_libraries(ObjectDemo
    PRIVATE
        Qt::Core
)

include(GNUInstallDirs)

install(TARGETS ObjectDemo
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ObjectDemo
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
