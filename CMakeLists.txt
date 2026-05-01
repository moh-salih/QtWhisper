cmake_minimum_required(VERSION 3.21)

project(QtWhisper VERSION 1.0.0 LANGUAGES CXX)

# Options
option(QtWhisper_BUILD_EXAMPLES "Build QtWhisper examples" OFF)


find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(whisper REQUIRED)

add_library(QtWhisper STATIC 
    include/QtWhisper/Engine.h
    include/QtWhisper/IEngine.h
    include/QtWhisper/Session.h
    include/QtWhisper/Types.h

    src/Session.cpp
    src/Engine.cpp
)

add_library(QtWhisper::QtWhisper ALIAS QtWhisper)

# Export include directory for consumers
target_include_directories(QtWhisper PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(QtWhisper PUBLIC Qt6::Core whisper)

set_target_properties(QtWhisper PROPERTIES AUTOMOC ON CXX_STANDARD 23)











if(QtWhisper_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()


# ==============================================================================
# INSTALLATION & EXPORT LOGIC
# ==============================================================================
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# A. Install the compiled library and export the target
install(TARGETS QtWhisper
    EXPORT QtWhisperTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# B. Install the header files so consumers can #include <QtWhisper/...>
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# C. Generate and install the QtWhisperTargets.cmake file
install(EXPORT QtWhisperTargets
    FILE QtWhisperTargets.cmake
    NAMESPACE QtWhisper::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtWhisper
)

# D. Generate Config and Version files using your .cmake.in template
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/QtWhisperConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/QtWhisperConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtWhisper
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/QtWhisperConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/QtWhisperConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/QtWhisperConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtWhisper
)
